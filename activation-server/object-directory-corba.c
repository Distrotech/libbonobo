#include "oaf.h"

/*** App-specific servant structures ***/

typedef struct {
  POA_OAF_ObjectDirectory servant;
  PortableServer_POA poa;
  OAF_ServerInfoList attr_servers;

  CORBA_char *attr_domain;
  CORBA_char *attr_hostID;

  guchar is_locked;
} impl_POA_OAF_ObjectDirectory;

/*** Implementation stub prototypes ***/

static OAF_ServerInfoList *
impl_OAF_ObjectDirectory__get_servers(impl_POA_OAF_ObjectDirectory * servant,
				      CORBA_Environment * ev);

static CORBA_char *
impl_OAF_ObjectDirectory__get_domain(impl_POA_OAF_ObjectDirectory * servant,
				     CORBA_Environment * ev);
static CORBA_char *
impl_OAF_ObjectDirectory__get_hostID(impl_POA_OAF_ObjectDirectory * servant,
				     CORBA_Environment * ev);

static CORBA_Object
impl_OAF_ObjectDirectory_activate(impl_POA_OAF_ObjectDirectory * servant,
				  OAF_ImplementationID iid,
				  CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_lock(impl_POA_OAF_ObjectDirectory * servant,
			      CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_unlock(impl_POA_OAF_ObjectDirectory * servant,
				CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ObjectDirectory_base_epv =
{
   NULL,			/* _private data */
   NULL,			/* finalize routine */
   NULL				/* default_POA routine */
};
static POA_OAF_ObjectDirectory__epv impl_OAF_ObjectDirectory_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_OAF_ObjectDirectory__get_servers,
   (gpointer) & impl_OAF_ObjectDirectory__get_domain,
   (gpointer) & impl_OAF_ObjectDirectory__get_hostID,
   (gpointer) & impl_OAF_ObjectDirectory_activate,
   (gpointer) & impl_OAF_ObjectDirectory_lock,
   (gpointer) & impl_OAF_ObjectDirectory_unlock
};

/*** vepv structures ***/

static POA_OAF_ObjectDirectory__vepv impl_OAF_ObjectDirectory_vepv =
{
   &impl_OAF_ObjectDirectory_base_epv,
   &impl_OAF_ObjectDirectory_epv
};

/*** Stub implementations ***/

OAF_ObjectDirectory
OAF_ObjectDirectory_create(PortableServer_POA poa,
			   const char *domain,
			   const char *hostname,
			   const char **source_directories,
			   CORBA_Environment *ev)
{
   OAF_ObjectDirectory retval;
   impl_POA_OAF_ObjectDirectory *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_OAF_ObjectDirectory, 1);
   newservant->servant.vepv = &impl_OAF_ObjectDirectory_vepv;
   newservant->poa = poa;
   POA_OAF_ObjectDirectory__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

static void
impl_OAF_ObjectDirectory__destroy(impl_POA_OAF_ObjectDirectory * servant, CORBA_Environment * ev)
{
   PortableServer_ObjectId *objid;

   objid = PortableServer_POA_servant_to_id(servant->poa, servant, ev);
   PortableServer_POA_deactivate_object(servant->poa, objid, ev);
   CORBA_free(objid);

   POA_OAF_ObjectDirectory__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static OAF_ServerInfoList *
impl_OAF_ObjectDirectory__get_servers(impl_POA_OAF_ObjectDirectory * servant,
				      CORBA_Environment * ev)
{
   OAF_ServerInfoList *retval;

   return retval;
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_domain(impl_POA_OAF_ObjectDirectory * servant,
				     CORBA_Environment * ev)
{
   CORBA_char *retval;

   return retval;
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_hostID(impl_POA_OAF_ObjectDirectory * servant,
				     CORBA_Environment * ev)
{
   CORBA_char *retval;

   return retval;
}

static CORBA_Object
impl_OAF_ObjectDirectory_activate(impl_POA_OAF_ObjectDirectory * servant,
				  OAF_ImplementationID iid,
				  CORBA_Environment * ev)
{
   CORBA_Object retval;

   return retval;
}

static void
impl_OAF_ObjectDirectory_lock(impl_POA_OAF_ObjectDirectory * servant,
			      CORBA_Environment * ev)
{
}

static void
impl_OAF_ObjectDirectory_unlock(impl_POA_OAF_ObjectDirectory * servant,
				CORBA_Environment * ev)
{
}
