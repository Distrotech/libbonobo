#include "oaf.h"

/*** App-specific servant structures ***/

typedef struct {
   POA_OAF_ActivationContext servant;
   PortableServer_POA poa;
   OAF_ObjectDirectoryList attr_directories;

   OAF_ServerInfoList attr_servers;

} impl_POA_OAF_ActivationContext;

/*** Implementation stub prototypes ***/

static void impl_OAF_ActivationContext__destroy(impl_POA_OAF_ActivationContext * servant,
						CORBA_Environment * ev);
static OAF_ObjectDirectoryList *
 impl_OAF_ActivationContext__get_directories(impl_POA_OAF_ActivationContext * servant,
					     CORBA_Environment * ev);

static void
 impl_OAF_ActivationContext_add_directory(impl_POA_OAF_ActivationContext * servant,
					  OAF_ObjectDirectory dir,
					  CORBA_Environment * ev);

static void
 impl_OAF_ActivationContext_remove_directory(impl_POA_OAF_ActivationContext * servant,
					     OAF_ObjectDirectory dir,
					     CORBA_Environment * ev);

static OAF_ActivationResult *
 impl_OAF_ActivationContext_activate(impl_POA_OAF_ActivationContext * servant,
				     CORBA_char * requirements,
				     CORBA_char * selection_order,
				     CORBA_Context ctx,
				     CORBA_Environment * ev);

static OAF_ServerInfoList *
 impl_OAF_ActivationContext__get_servers(impl_POA_OAF_ActivationContext * servant,
					 CORBA_Environment * ev);

static OAF_ServerInfoList *
impl_OAF_ActivationContext_query(impl_POA_OAF_ActivationContext * servant,
				 CORBA_char * requirements,
				 CORBA_Environment * ev);

static OAF_ActivationResult *
impl_OAF_ActivationContext_activate_from_id(impl_POA_OAF_ActivationContext * servant,
					    OAF_ActivationID aid,
					    CORBA_Context ctx,
					    CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ActivationContext_base_epv =
{
   NULL,			/* _private data */
   NULL,			/* finalize routine */
   NULL			        /* default_POA routine */
};
static POA_OAF_ActivationContext__epv impl_OAF_ActivationContext_epv =
{
   NULL,			/* _private */
   (gpointer) & impl_OAF_ActivationContext__get_directories,
   (gpointer) & impl_OAF_ActivationContext_add_directory,
   (gpointer) & impl_OAF_ActivationContext_remove_directory,
   (gpointer) & impl_OAF_ActivationContext_activate,
   (gpointer) & impl_OAF_ActivationContext__get_servers,
   (gpointer) & impl_OAF_ActivationContext_query,
   (gpointer) & impl_OAF_ActivationContext_activate_from_id
};

/*** vepv structures ***/

static POA_OAF_ActivationContext__vepv impl_OAF_ActivationContext_vepv =
{
   &impl_OAF_ActivationContext_base_epv,
   &impl_OAF_ActivationContext_epv
};

/*** Stub implementations ***/

OAF_ActivationContext
OAF_ActivationContext_create(PortableServer_POA poa,
			     CORBA_Environment *ev);
{
   OAF_ActivationContext retval;
   impl_POA_OAF_ActivationContext *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_OAF_ActivationContext, 1);
   newservant->servant.vepv = &impl_OAF_ActivationContext_vepv;
   newservant->poa = poa;
   POA_OAF_ActivationContext__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

static void
impl_OAF_ActivationContext__destroy(impl_POA_OAF_ActivationContext * servant, CORBA_Environment * ev)
{
   PortableServer_ObjectId *objid;

   objid = PortableServer_POA_servant_to_id(servant->poa, servant, ev);
   PortableServer_POA_deactivate_object(servant->poa, objid, ev);
   CORBA_free(objid);

   POA_OAF_ActivationContext__fini((PortableServer_Servant) servant, ev);
   g_free(servant);
}

static OAF_ObjectDirectoryList *
impl_OAF_ActivationContext__get_directories(impl_POA_OAF_ActivationContext * servant,
					    CORBA_Environment * ev)
{
   OAF_ObjectDirectoryList *retval;

   return retval;
}

static void
impl_OAF_ActivationContext_add_directory(impl_POA_OAF_ActivationContext * servant,
					 OAF_ObjectDirectory dir,
					 CORBA_Environment * ev)
{
}

static void
impl_OAF_ActivationContext_remove_directory(impl_POA_OAF_ActivationContext * servant,
					    OAF_ObjectDirectory dir,
					    CORBA_Environment * ev)
{
}

static OAF_ActivationResult *
impl_OAF_ActivationContext_activate(impl_POA_OAF_ActivationContext * servant,
				    CORBA_char * requirements,
				    CORBA_char * selection_order,
				    CORBA_Context ctx,
				    CORBA_Environment * ev)
{
   OAF_ActivationResult *retval;

   return retval;
}

static OAF_ServerInfoList *
impl_OAF_ActivationContext__get_servers(impl_POA_OAF_ActivationContext * servant,
					CORBA_Environment * ev)
{
   OAF_ServerInfoList *retval;

   return retval;
}

static OAF_ServerInfoList *
impl_OAF_ActivationContext_query(impl_POA_OAF_ActivationContext * servant,
				 CORBA_char * requirements,
				 CORBA_Environment * ev)
{
   OAF_ServerInfoList *retval;

   return retval;
}

static OAF_ActivationResult *
impl_OAF_ActivationContext_activate_from_id(impl_POA_OAF_ActivationContext * servant,
					    OAF_ActivationID aid,
					    CORBA_Context ctx,
					    CORBA_Environment * ev)
{
   OAF_ActivationResult *retval;

   return retval;
}
