/* This is part of the per-app CORBA bootstrapping - we use this to get hold of a running metaserver and such */
#include "liboaf-private.h"
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

static GSList *reglocs = NULL;

typedef struct {
  int priority;
  const OAFRegistrationLocation *regloc;
  gpointer user_data;
} RegInfo;


static gint
ri_compare (gconstpointer a, gconstpointer b)
{
  const RegInfo *ra, *rb;

  ra = a;
  rb = b;

  return (rb->priority - ra->priority);
}

void
oaf_registration_location_add(const OAFRegistrationLocation *regloc, int priority, gpointer user_data)
{
  RegInfo *new_ri;

  g_return_if_fail(regloc);

  new_ri = g_new(RegInfo, 1);
  new_ri->priority = priority;
  new_ri->regloc = regloc;
  new_ri->user_data = user_data;

  reglocs = g_slist_insert_sorted(reglocs, new_ri, ri_compare);
}

CORBA_Object
oaf_registration_check(const OAFRegistrationCategory *regcat, CORBA_Environment *ev)
{
  GSList *cur;
  CORBA_Object retval = CORBA_OBJECT_NIL;
  int dist = INT_MAX;
  char *ior = NULL;

  for (cur = reglocs; cur; cur = cur->next)
    {
      RegInfo *ri;
      char *new_ior;
      int new_dist = dist;

      ri = cur->data;

      if(!ri->regloc->check)
	continue;

      new_ior = ri->regloc->check(ri->regloc, regcat, &dist, ri->user_data);
      if(new_dist < dist)
	{
	  g_free(ior);
	  ior = new_ior;
	}
    }

  if(ior)
    {
      retval = CORBA_ORB_string_to_object(oaf_orb_get(), ior, ev);
      if(ev->_major != CORBA_NO_EXCEPTION)
	retval = CORBA_OBJECT_NIL;
    }

  return retval;
}

/* dumb marshalling hack */
static void
oaf_registration_iterate(const OAFRegistrationCategory *regcat, CORBA_Object obj, CORBA_Environment *ev, gulong offset,
			 int nargs)
{
  GSList *cur;
  char *ior = NULL;

  if(nargs == 4)
    ior = CORBA_ORB_object_to_string(oaf_orb_get(), obj, ev);

  for (cur = reglocs; cur; cur = cur->next)
    {
      RegInfo *ri;
      void (*func_ptr)();

      ri = cur->data;

      func_ptr = G_STRUCT_MEMBER_P(ri->regloc, offset);

      if(!func_ptr)
	continue;

      switch(nargs)
	{
	case 4:
	  func_ptr(ri->regloc, ior, regcat, ri->user_data);
	  break;
	case 2:
	  func_ptr(ri->regloc, ri->user_data);
	  break;
	}
    }

  if(nargs == 4)
    CORBA_free(ior);
}


#define oaf_reglocs_lock() oaf_registration_iterate(NULL, CORBA_OBJECT_NIL, ev, G_STRUCT_OFFSET(OAFRegistrationLocation, lock), 2)
#define oaf_reglocs_unlock() oaf_registration_iterate(NULL, CORBA_OBJECT_NIL, ev, G_STRUCT_OFFSET(OAFRegistrationLocation, unlock), 2)

void
oaf_registration_unset(const OAFRegistrationCategory *regcat, CORBA_Object obj, CORBA_Environment *ev)
{
  oaf_reglocs_lock();
  oaf_registration_iterate(regcat, obj, ev, G_STRUCT_OFFSET(OAFRegistrationLocation, register_new), 4);
  oaf_reglocs_unlock();
}

void
oaf_registration_set(const OAFRegistrationCategory *regcat, CORBA_Object obj, CORBA_Environment *ev)
{
  oaf_registration_iterate(regcat, obj, ev, G_STRUCT_OFFSET(OAFRegistrationLocation, unregister), 4);
}

/* Whacked from gnome-libs/libgnorba/orbitns.c */

typedef struct {
  GMainLoop *mloop;
  char iorbuf[2048];
#ifdef OAF_DEBUG
  char *do_srv_output;
#endif
  FILE *fh;
} EXEActivateInfo;

static gboolean
handle_exepipe(GIOChannel      *source,
	       GIOCondition     condition,
	       EXEActivateInfo *data)
{
  gboolean retval = TRUE;

  *data->iorbuf = '\0';
  if(!(condition & G_IO_IN)
     || !fgets(data->iorbuf, sizeof(data->iorbuf), data->fh))
    retval = FALSE;

  if(retval && !strncmp(data->iorbuf, "IOR:", 4))
    retval = FALSE;

#ifdef OAF_DEBUG
  if(data->do_srv_output)
    g_message("srv output[%d]: '%s'", retval, data->iorbuf);
#endif

  if(!retval)
    g_main_quit(data->mloop);

  return retval;
}

#ifdef OAF_DEBUG
static void print_exit_status(int status)
{
  if(WIFEXITED(status))
    g_message("Exit status was %d", WEXITSTATUS(status));

  if(WIFSIGNALED(status))
    g_message("signal was %d", WTERMSIG(status));
}
#endif

CORBA_Object
oaf_server_by_forking(const char **cmd, int ior_fd, CORBA_Environment *ev)
{
  gint iopipes[2];
  CORBA_Object retval = CORBA_OBJECT_NIL;
  int childpid;

  pipe(iopipes);

  /* fork & get the IOR from the magic pipe */
  childpid = fork();
  if(childpid)
    {
      int status;
      FILE *iorfh;
      EXEActivateInfo ai;
      guint watchid;
      GIOChannel *gioc;

      waitpid(childpid, &status, 0); /* de-zombify */

#ifdef OAF_DEBUG
      ai.do_srv_output = getenv("OAF_DEBUG_EXERUN");

      if(ai.do_srv_output)
	print_exit_status(status);
#endif

      close(iopipes[1]);
      ai.fh = iorfh = fdopen(iopipes[0], "r");

      ai.iorbuf[0] = '\0';
      ai.mloop = g_main_new(FALSE);
      gioc = g_io_channel_unix_new(iopipes[0]);
      watchid = g_io_add_watch(gioc, G_IO_IN|G_IO_HUP|G_IO_NVAL|G_IO_ERR,
			       (GIOFunc)&handle_exepipe, &ai);
      g_io_channel_unref(gioc);
      g_main_run(ai.mloop);
      g_main_destroy(ai.mloop);

      g_strstrip(ai.iorbuf);
      if (!strncmp(ai.iorbuf, "IOR:", 4))
	{
	  retval = CORBA_ORB_string_to_object(oaf_orb_get(), ai.iorbuf, ev);
	  if(ev->_major != CORBA_NO_EXCEPTION)
	    retval = CORBA_OBJECT_NIL;
#ifdef OAF_DEBUG
	  if(ai.do_srv_output)
	    g_message("Did string_to_object on %s", ai.iorbuf);
#endif
	}
      else
	{
#ifdef OAF_DEBUG
	  if (ai.do_srv_output) g_message("string doesn't match IOR:");
#endif
	  retval = CORBA_OBJECT_NIL;
	}
    }
  else if(fork())
    {
      _exit(0); /* de-zombifier process, just exit */
    }
  else
    {
      struct sigaction sa;

      close(iopipes[0]);
      if(iopipes[1] != ior_fd)
	{
	  dup2(iopipes[1], ior_fd);
	  close(iopipes[1]);
	}

      setsid();
      memset(&sa, 0, sizeof(sa));
      sa.sa_handler = SIG_IGN;
      sigaction(SIGPIPE, &sa, 0);

      execvp(cmd[0], (char **)cmd);
      _exit(1);
    }

  return retval;
}

char * oaf_ac_cmd[] = {"oafd", "--ac-activate", "--ior-output-fd=123", NULL};

struct SysServer {
  const char *name;
  const char **cmd;
  int ior_fd;
  CORBA_Object already_running;
} activatable_servers[] = {
  {"IDL:OAF/ActivationContext:1.0", (const char **)oaf_ac_cmd, 123, CORBA_OBJECT_NIL},
  {NULL}
};

CORBA_Object
oaf_service_get(const OAFRegistrationCategory *regcat)
{
  CORBA_Object retval = CORBA_OBJECT_NIL;
  int i;
  CORBA_Environment *ev, myev;
  gboolean ne;

  g_return_val_if_fail(regcat, CORBA_OBJECT_NIL);

  for(i = 0; activatable_servers[i].name; i++)
    {
      if(!strcmp(regcat->name, activatable_servers[i].name))
	break;
    }

  if(!activatable_servers[i].name)
    return retval;

  CORBA_exception_init(&myev); ev = &myev;
  if(!CORBA_Object_non_existent(activatable_servers[i].already_running, &myev))
    {
      retval = CORBA_Object_duplicate(activatable_servers[i].already_running, &myev);
    }

  oaf_reglocs_lock();

  retval = oaf_registration_check(regcat, &myev);
  ne = CORBA_Object_non_existent(retval, &myev);
  if(ne)
    {
      CORBA_Object_release(retval, &myev);
      retval = oaf_server_by_forking(activatable_servers[i].cmd, activatable_servers[i].ior_fd, &myev);
      if(!CORBA_Object_is_nil(retval, &myev))
	oaf_registration_set(regcat, retval, &myev);
    }

  oaf_reglocs_unlock();

  CORBA_exception_free(&myev);

  return retval;
}

/***** Implementation of the IOR registration system via plain files ******/
static void
rloc_file_lock(const OAFRegistrationLocation *regloc, gpointer user_data)
{
  char *fn;
  static int fd = -1;

  fn = oaf_alloca(sizeof("/tmp/orbit-%s/oaf-register.lock" + 32));
  sprintf(fn, "/tmp/orbit-%s/oaf-register.lock", g_get_user_name());

  while((fd = open(fn, O_CREAT|O_EXCL|O_RDWR)) < 0)
    {
      if(errno == EEXIST)
	{
#ifdef HAVE_USLEEP
	  usleep(10000);
#elif defined(HAVE_NANOSLEEP)
	  {
	    struct timespec timewait;
	    timewait.tv_sec = 0;
	    timewait.tv_nsec = 1000000;
	    nanosleep(&timewait, NULL);
	  }
#else
	  sleep(1);
#endif
	}
      else
	break;
    }

  close(fd);
}

static void
rloc_file_unlock(const OAFRegistrationLocation *regloc, gpointer user_data)
{
  char *fn;

  fn = oaf_alloca(sizeof("/tmp/orbit-%s/oaf-register.lock" + 32));
  sprintf(fn, "/tmp/orbit-%s/oaf-register.lock", g_get_user_name());

  unlink(fn);
}

static char *
rloc_file_check(const OAFRegistrationLocation *regloc, const OAFRegistrationCategory *regcat,
		int *ret_distance, gpointer user_data)
{
  FILE *fh;
  char fn[PATH_MAX], *uname;

  uname = g_get_user_name();

  sprintf(fn, "/tmp/orbit-%s/reg.%s-%s",
	  uname,
	  regcat->name,
	  regcat->session_name?regcat->session_name:"local");
  fh = fopen(fn, "r");
  if(fh)
    goto useme;

  sprintf(fn, "/tmp/orbit-%s/reg.%s",
	  uname,
	  regcat->name);
  fh = fopen(fn, "r");
  if(fh)
    goto useme;

 useme:
  if(fh)
    {
      char iorbuf[8192];

      iorbuf[0] = '\0';
      while(fgets(iorbuf, sizeof(iorbuf), fh)
	    && strncmp(iorbuf, "IOR:", 4)) /**/;
      g_strstrip(iorbuf);

      fclose(fh);

      if(!strncmp(iorbuf, "IOR:", 4))
	return g_strdup(iorbuf);
    }

  return NULL;
}

static void
rloc_file_register(const OAFRegistrationLocation *regloc, const char *ior,
		   const OAFRegistrationCategory *regcat, gpointer user_data)
{
  char fn[PATH_MAX], fn2[PATH_MAX], *uname;
  FILE *fh;

  uname = g_get_user_name();

  sprintf(fn, "/tmp/orbit-%s/reg.%s-%s",
	  uname,
	  regcat->name,
	  regcat->session_name?regcat->session_name:"local");

  sprintf(fn2, "/tmp/orbit-%s/reg.%s",
	  uname,
	  regcat->name);

  fh = fopen(fn, "w");
  fprintf(fh, "%s\n", ior);
  fclose(fh);

  symlink(fn, fn2);
}

static void
rloc_file_unregister(const OAFRegistrationLocation *regloc, const char *ior,
		     const OAFRegistrationCategory *regcat, gpointer user_data)
{
  char fn2[PATH_MAX], fn3[PATH_MAX];
  char fn[PATH_MAX];
  char *uname;

  uname = g_get_user_name();

  sprintf(fn, "/tmp/orbit-%s/reg.%s-%s",
	  uname,
	  regcat->name,
	  regcat->session_name?regcat->session_name:"local");
  unlink(fn);

  sprintf(fn2, "/tmp/orbit-%s/reg.%s",
	  uname,
	  regcat->name);

  if(readlink(fn2, fn3, sizeof(fn3) < 0))
    return;

  if(!strcmp(fn3, fn))
    unlink(fn2);
}

static const OAFRegistrationLocation rloc_file = {
  rloc_file_lock,
  rloc_file_unlock,
  rloc_file_check,
  rloc_file_register,
  rloc_file_unregister
};
