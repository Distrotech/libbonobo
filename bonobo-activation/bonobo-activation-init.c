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

const char *
oaf_session_name_get(void)
{
  return "local";
}

CORBA_Object
oaf_activation_context_get(void)
{
  OAFRegistrationCategory regcat;

  regcat.name = "IDL:OAF/ActivationContext:1.0";
  regcat.session_name = oaf_session_name_get();

  return oaf_service_get(&regcat);
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


