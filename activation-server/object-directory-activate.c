#include "config.h"

#include "oafd.h"
#include "liboaf/liboaf.h"

#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define OAF_MAGIC_FD 123

CORBA_Object od_server_activate_factory(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev);
CORBA_Object od_server_activate_exe(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev);

CORBA_Object
od_server_activate(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev)
{
  if(!strcmp(si->server_type, "exe"))
    return od_server_activate_exe(si, actinfo, ev);

  else if(!strcmp(si->server_type, "factory"))
    return od_server_activate_factory(si, actinfo, ev);

  else if(!strcmp(si->server_type, "shlib"))
    g_warning("We don't handle activating shlib objects in a remote process yet");

  return CORBA_OBJECT_NIL;
}

CORBA_Object
od_server_activate_factory(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev)
{
  CORBA_Object retval = CORBA_OBJECT_NIL, factory;
  OAF_ActivationResult *res;
  GNOME_stringlist dummy = {0};

  res = OAF_ActivationContext_activate_from_id(actinfo->ac, si->location_info,
					       ( (actinfo->flags | OAF_FLAG_NO_LOCAL) & (~OAF_FLAG_IGNORE_EXISTING) ),
					       oaf_context_get(), ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto out;

  switch(res->_d) {
  case OAF_RESULT_NONE:
    CORBA_free(res);
    goto out;
    break;
  case OAF_RESULT_OBJECT:
    factory = res->_u.res_object;
    break;
  default:
    g_assert_not_reached();
    break;
  }

  retval = GNOME_GenericFactory_create_object(factory, si->iid, &dummy, ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    retval = CORBA_OBJECT_NIL;

  CORBA_free(res);

 out:
  return retval;
}

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

static void print_exit_status(int status)
{
  if(WIFEXITED(status))
    g_message("Exit status was %d", WEXITSTATUS(status));

  if(WIFSIGNALED(status))
    g_message("signal was %d", WTERMSIG(status));
}

/* Copied largely from goad.c, goad_server_activate_exe() */
CORBA_Object
od_server_activate_exe(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev)
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
      char **args;
      int i;
      struct sigaction sa;
      char *extra_arg, *ctmp, *ctmp2;

      close(iopipes[0]);
      if(iopipes[1] != OAF_MAGIC_FD)
	{
	  dup2(iopipes[1], OAF_MAGIC_FD);
	  close(iopipes[1]);
	}

      setsid();
      memset(&sa, 0, sizeof(sa));
      sa.sa_handler = SIG_IGN;
      sigaction(SIGPIPE, &sa, 0);

      /* Munge the args */
      args = oaf_alloca(34 * sizeof(char *));
      for(i = 0, ctmp = ctmp2 = si->location_info; i < 32; i++) {
	while(*ctmp2 && !isspace(*ctmp2)) ctmp2++;
	if(!*ctmp2) break;

	args[i] = oaf_alloca(ctmp2 - ctmp + 1);
	strncpy(args[i], ctmp, ctmp2 - ctmp);
	args[i][ctmp2 - ctmp] = '\0';

	ctmp = ctmp2;
	while(*ctmp2 && isspace(*ctmp2)) ctmp2++;
	if(!*ctmp2)
	  break;
	ctmp = ctmp2;
      }
      if(!isspace(*ctmp) && i < 32)
	args[i++] = ctmp;

      extra_arg = oaf_alloca(strlen(si->iid) + sizeof("--activate-oaf-server="));
      args[i++] = extra_arg;
      sprintf(extra_arg, "--activate-oaf-server=%s", si->iid);

      args[i] = NULL;

      execvp(args[0], args);
      _exit(1);
    }

  return retval;
}
