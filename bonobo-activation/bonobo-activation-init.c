#include "config.h"

#include "liboaf/liboaf.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

#include <glib.h>
#include <popt.h>

/****************** ORBit-specific stuff ****************/

#ifdef HAVE_ORB_ORBIT_H

#include <orb/orbit.h>

static int oaf_corba_prio = G_PRIORITY_LOW;

#ifndef ORBIT_USES_GLIB_MAIN_LOOP

static gboolean
orb_handle_connection(GIOChannel *source, GIOCondition cond,
		      GIOPConnection *cnx)
{
	/* The best way to know about an fd exception is if select()/poll()
	 * tells you about it, so we just relay that information on to ORBit
	 * if possible
	 */
	
	if(cond & (G_IO_HUP|G_IO_NVAL|G_IO_ERR))
		giop_main_handle_connection_exception(cnx);
	else
		giop_main_handle_connection(cnx);
	
	return TRUE;
}

static void
orb_add_connection(GIOPConnection *cnx)
{
	int tag;
	GIOChannel *channel;
	
	channel = g_io_channel_unix_new(GIOP_CONNECTION_GET_FD(cnx));
	tag = g_io_add_watch_full   (channel, oaf_corba_prio,
				     G_IO_IN|G_IO_ERR|G_IO_HUP|G_IO_NVAL, 
				     (GIOFunc)orb_handle_connection,
				     cnx, NULL);
	g_io_channel_unref (channel);
	
	cnx->user_data = GUINT_TO_POINTER (tag);
}

static void
orb_remove_connection(GIOPConnection *cnx)
{
	g_source_remove(GPOINTER_TO_UINT (cnx->user_data));
	cnx->user_data = GINT_TO_POINTER (-1);
}

#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */


static CORBA_ORB oaf_orb;
static CORBA_Context oaf_context;

CORBA_ORB
oaf_orb_get(void)
{
  return oaf_orb;
}

char *
liboaf_hostname_get(void)
{
  char *hostname;
  char hn_tmp[65];
  struct hostent *hent, *hent2;

  gethostname(hn_tmp, sizeof(hn_tmp) - 1);

  hent = gethostbyname(hn_tmp);
  if(hent) {
    hent2 = gethostbyaddr(hent->h_addr, 4, AF_INET);
    if(hent2)
      hostname = hent2->h_name;
    else
      hostname = inet_ntoa(*((struct in_addr *)hent->h_addr));
  } else
    hostname = hn_tmp;

  return g_strdup(hostname);
}

CORBA_Context
oaf_context_get(void)
{
  return oaf_context;
}

/* Whacked directly from gnome-libs/libgnorba/orbit.ns */
static CORBA_Object
name_server_by_forking (CORBA_Environment *ev)
{
  CORBA_Object name_service = CORBA_OBJECT_NIL;
  int iopipes[2];
  pid_t pid;
	
  /*
   * Since we're pretty sure no name server is running,
   * we start it ourself and tell the (GNOME session)
   * world about it
   */
	
  /* fork & get the ior from orbit-name-service stdout */
  pipe(iopipes);

  pid = fork ();
  if (pid == -1)
    return name_service;
	
  if (pid)
    {
      FILE *iorfh;
      int status;
      char iorbuf[2048];

      /* Parent */
      waitpid(pid, &status, 0); /* de-zombify */

      close(iopipes[1]);

      iorfh = fdopen(iopipes[0], "r");

      iorbuf[0] = '\0';
      while(fgets(iorbuf, sizeof(iorbuf), iorfh))
	{
#ifdef OAF_DEBUG
	  if(getenv("OAF_DEBUG_EXERUN"))
	    g_print("Got line \"%s\"\n", iorbuf);
#endif
	  if(!strncmp(iorbuf, "IOR:", 4))
	    break;
	}

      iorbuf[strlen(iorbuf)-1] = '\0';
      g_strchug(iorbuf);

      if (strncmp(iorbuf, "IOR:", 4))
	goto out;

      name_service = CORBA_ORB_string_to_object((CORBA_ORB)oaf_orb_get(), iorbuf, ev);
      if(ev->_major != CORBA_NO_EXCEPTION)
	name_service = CORBA_OBJECT_NIL;

    out:
      fclose(iorfh);

    }
  else if (fork ())
    {
      /* de-zombifier process, just exit */
      _exit(0);
    }
  else
    {
      /* Child of a child. We run the naming service */
      struct sigaction sa;
      int i, open_max;

      setsid();
		
      open_max = sysconf(_SC_OPEN_MAX);
      for (i = 3; i < open_max; i++)
	fcntl(i, F_SETFD, FD_CLOEXEC);

      sa.sa_handler = SIG_IGN;
      sigaction(SIGPIPE, &sa, 0);
      close(0);
      close(iopipes[0]);
      dup2(iopipes[1], 1);
      dup2(iopipes[1], 2);
      close(iopipes[1]);
		
      execlp("oafd", "oafd", NULL);
      _exit(1);
    }

  return name_service;
}

CORBA_Object
oaf_activation_context_get(void)
{
  static CORBA_Object ac = CORBA_OBJECT_NIL;
  CORBA_Environment ev;

  if(CORBA_Object_is_nil(ac, &ev))
    {
      CORBA_exception_init(&ev);
      ac = name_server_by_forking(&ev);
      CORBA_exception_free(&ev);
    }

  return ac;
}

static char *oaf_od_ior = NULL;

struct poptOption oaf_popt_options[] = {
  {"oaf-od-ior", '\0', POPT_ARG_STRING, &oaf_od_ior, 0, "Object directory to use when registering servers", "IOR"},
  {NULL}
};

void
oaf_preinit(void)
{
}

void
oaf_postinit(gpointer app, gpointer mod_info)
{
  
}

static void
do_barrier(int signum)
{
  volatile int barrier = 1;

  while(barrier);
}

CORBA_ORB
oaf_orb_init(int *argc, char **argv)
{
  CORBA_Environment ev;
  char *hostname;

#ifndef ORBIT_USES_GLIB_MAIN_LOOP
  IIOPAddConnectionHandler = orb_add_connection;
  IIOPRemoveConnectionHandler = orb_remove_connection;
#endif /* !ORBIT_USES_GLIB_MAIN_LOOP */

  CORBA_exception_init(&ev);

  oaf_orb = CORBA_ORB_init(argc, argv, "orbit-local-orb", &ev);
  g_assert(ev._major == CORBA_NO_EXCEPTION);

  /* Set values in default context */
  CORBA_ORB_get_default_context(oaf_orb, &oaf_context, &ev);
  g_assert(ev._major == CORBA_NO_EXCEPTION);

  hostname = liboaf_hostname_get();
  CORBA_Context_set_one_value(oaf_context, "hostname", hostname, &ev);
  CORBA_Context_set_one_value(oaf_context, "domain", "user", &ev);
  CORBA_Context_set_one_value(oaf_context, "username", g_get_user_name(), &ev);
  g_free(hostname);

  CORBA_exception_free(&ev);

#ifdef OAF_DEBUG
  if(getenv("OAF_TRAP_SEGV"))
    {
      struct sigaction sa;
      sa.sa_handler = do_barrier;
      sigaction(SIGSEGV, &sa, NULL);
      sigaction(SIGPIPE, &sa, NULL);
    }
  if(getenv("OAF_BARRIER_INIT"))
    {
      volatile int barrier=1;
      while(barrier);
    }
#endif

  return oaf_orb;
}

#else

#error "You need to use a supported ORB for liboaf"

#endif


