#include "config.h"

#include "oafd.h"
#include "liboaf/liboaf-private.h"

#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

#define OAF_MAGIC_FD 123

CORBA_Object od_server_activate_factory(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev);
CORBA_Object od_server_activate_exe(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Object od_obj,
				    CORBA_Environment *ev);

CORBA_Object
od_server_activate(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Object od_obj, CORBA_Environment *ev)
{
  if(!strcmp(si->server_type, "exe"))
    return od_server_activate_exe(si, actinfo, od_obj, ev);

  else if(!strcmp(si->server_type, "factory"))
    return od_server_activate_factory(si, actinfo, ev);

  else if(!strcmp(si->server_type, "shlib"))
    g_warning("We don't handle activating shlib objects in a remote process yet");

  return CORBA_OBJECT_NIL;
}

CORBA_Object
od_server_activate_factory(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Environment *ev)
{
  CORBA_Object retval = CORBA_OBJECT_NIL, factory = CORBA_OBJECT_NIL;
  OAF_ActivationResult *res;
  GNOME_stringlist dummy = {0};

  res = OAF_ActivationContext_activate_from_id(actinfo->ac, si->location_info,
					       ( (actinfo->flags | OAF_FLAG_NO_LOCAL) & (~OAF_FLAG_IGNORE_EXISTING) ),
					       oaf_context_get(), ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto out;

  switch(res->res._d) {
  case OAF_RESULT_NONE:
    CORBA_free(res);
    goto out;
    break;
  case OAF_RESULT_OBJECT:
    factory = res->res._u.res_object;
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


/* Copied largely from goad.c, goad_server_activate_exe() */
CORBA_Object
od_server_activate_exe(OAF_ServerInfo *si, ODActivationInfo *actinfo, CORBA_Object od_obj, CORBA_Environment *ev)
{
  char **args;
  char *extra_arg, *ctmp, *ctmp2;
  int i;

  /* Munge the args */
  args = oaf_alloca(36 * sizeof(char *));
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

  extra_arg = oaf_alloca(strlen(si->iid) + sizeof("--oaf-activate-iid="));
  args[i++] = extra_arg;
  sprintf(extra_arg, "--oaf-activate-iid=%s", si->iid);
  
  extra_arg = oaf_alloca(sizeof("--oaf-ior-fd=") + 10);
  args[i++] = extra_arg;
  sprintf(extra_arg, "--oaf-ior-fd=%d", OAF_MAGIC_FD);

  {
    char *iorstr;

    iorstr = CORBA_ORB_object_to_string(oaf_orb_get(), od_obj, ev);

    if(ev->_major == CORBA_NO_EXCEPTION)
      {
	extra_arg = oaf_alloca(sizeof("--oaf-od-ior=") + strlen(iorstr));
	args[i++] = extra_arg;
	sprintf(extra_arg, "--oaf-od-ior=%s", iorstr);

	CORBA_free(iorstr);
      }
  }

  args[i] = NULL;

  return oaf_server_by_forking((const char **)args, OAF_MAGIC_FD, ev);
}
