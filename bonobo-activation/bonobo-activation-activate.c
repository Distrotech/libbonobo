#include "liboaf/liboaf-private.h"

extern CORBA_Object oaf_server_activate_shlib(OAF_ActivationResult *sh, CORBA_Environment *ev);

CORBA_Object
oaf_activate(const char *requirements, const char **selection_order, OAF_ActivationFlags flags, CORBA_Environment *ev)
{
  GNOME_stringlist selorder;
  CORBA_Object retval = CORBA_OBJECT_NIL;
  OAF_ActivationResult *res;
  int i;
  CORBA_Environment myev;
  OAF_ActivationContext ac;

  g_return_val_if_fail(requirements, CORBA_OBJECT_NIL);
  ac = oaf_activation_context_get();
  g_return_val_if_fail(ac, CORBA_OBJECT_NIL);

  if(!ev) 
    {
      ev = &myev;
      CORBA_exception_init(&myev);
    }

  if(selection_order)
    {
      for (i = 0; selection_order[i]; i++) /**/;

      selorder._length = i;
      selorder._buffer = (char **)selection_order;
      CORBA_sequence_set_release(&selorder, CORBA_FALSE);
    }
  else
    memset(&selorder, 0, sizeof(selorder));

  res = OAF_ActivationContext_activate(ac, (char *)requirements,
				       &selorder, flags, oaf_context_get(), ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto out;

  switch(res->_d)
    {
    case OAF_RESULT_SHLIB:
      retval = oaf_server_activate_shlib(res, ev);
      break;
    case OAF_RESULT_OBJECT:
      retval = CORBA_Object_duplicate(res->_u.res_object, ev);
      break;
    case OAF_RESULT_NONE:
    default:
      break;
    }

  CORBA_free(res);

 out:
  if(ev == &myev) 
    CORBA_exception_free(&myev);

  return retval;
}

CORBA_Object
oaf_activate_from_id(const OAF_ActivationID aid, OAF_ActivationFlags flags, CORBA_Environment *ev)
{
  CORBA_Object retval = CORBA_OBJECT_NIL;
  OAF_ActivationResult *res;
  int i;
  CORBA_Environment myev;
  OAF_ActivationContext ac;

  g_return_val_if_fail(aid, CORBA_OBJECT_NIL);
  ac = oaf_activation_context_get();
  g_return_val_if_fail(ac, CORBA_OBJECT_NIL);

  if(!ev) 
    {
      ev = &myev;
      CORBA_exception_init(&myev);
    }

  res = OAF_ActivationContext_activate_from_id(ac, aid, flags, oaf_context_get(), ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto out;

  switch(res->_d)
    {
    case OAF_RESULT_SHLIB:
      retval = oaf_server_activate_shlib(res, ev);
      break;
    case OAF_RESULT_OBJECT:
      retval = CORBA_Object_duplicate(res->_u.res_object, ev);
      break;
    case OAF_RESULT_NONE:
    default:
      break;
    }

  CORBA_free(res);

 out:
  if(ev == &myev) 
    CORBA_exception_free(&myev);

  return retval;
}
