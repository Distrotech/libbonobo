#include "liboaf/liboaf-private.h"

OAF_RegistrationResult
oaf_active_server_register(const char *iid, CORBA_Object obj)
{
  OAF_ObjectDirectory od;
  OAFRegistrationCategory regcat = {"IDL:OAF/ObjectDirectory:1.0"};
  CORBA_Environment ev;
  OAF_RegistrationResult retval;

  regcat.session_name = oaf_session_name_get();
  regcat.username = oaf_username_get();
  regcat.hostname = oaf_hostname_get();

  od = oaf_service_get(&regcat);

  CORBA_exception_init(&ev);
  if(CORBA_Object_is_nil(od, &ev))
    return OAF_REG_ERROR;

  retval = OAF_ObjectDirectory_register_new(od, iid, obj, &ev);
  CORBA_exception_free(&ev);

  return retval;
}

void
oaf_active_server_unregister(const char *iid, CORBA_Object obj)
{
  OAF_ObjectDirectory od;
  OAFRegistrationCategory regcat = {"IDL:OAF/ObjectDirectory:1.0"};
  CORBA_Environment ev;

  regcat.session_name = oaf_session_name_get();
  regcat.username = oaf_username_get();
  regcat.hostname = oaf_hostname_get();

  od = oaf_service_get(&regcat);

  CORBA_exception_init(&ev);
  if(CORBA_Object_is_nil(od, &ev))
    return;

  OAF_ObjectDirectory_unregister(od, (char *)iid, obj, &ev);
  CORBA_exception_free(&ev);
}
