/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "config.h"

#include "oafd.h"
#include "liboaf/liboaf.h"
#include <time.h>

/*** App-specific servant structures ***/

typedef struct
{
	POA_OAF_ObjectDirectory servant;
	PortableServer_POA poa;
	OAF_ServerInfoList attr_servers;
	OAF_CacheTime time_list_changed;

	GHashTable *by_iid;

	CORBA_char *attr_domain;
	const char *attr_hostID;

	guchar is_locked;

	GHashTable *active_servers;
	OAF_CacheTime time_active_changed;

	CORBA_Object self;
}
impl_POA_OAF_ObjectDirectory;

/*** Implementation stub prototypes ***/

static OAF_ServerInfoListCache
	*
impl_OAF_ObjectDirectory__get_servers (impl_POA_OAF_ObjectDirectory * servant,
				       OAF_CacheTime only_if_newer,
				       CORBA_Environment * ev);

static OAF_ServerStateCache
	* impl_OAF_ObjectDirectory_get_active_servers
	(impl_POA_OAF_ObjectDirectory * servant, OAF_CacheTime only_if_newer,
	 CORBA_Environment * ev);

static CORBA_char
	* impl_OAF_ObjectDirectory__get_domain (impl_POA_OAF_ObjectDirectory *
						servant,
						CORBA_Environment * ev);
static CORBA_char
	* impl_OAF_ObjectDirectory__get_hostID (impl_POA_OAF_ObjectDirectory *
						servant,
						CORBA_Environment * ev);
static CORBA_char
	* impl_OAF_ObjectDirectory__get_username (impl_POA_OAF_ObjectDirectory
						  * servant,
						  CORBA_Environment * ev);

static CORBA_Object
impl_OAF_ObjectDirectory_activate (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   OAF_ActivationContext ac,
				   OAF_ActivationFlags flags,
				   CORBA_Context ctx, CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_lock (impl_POA_OAF_ObjectDirectory * servant,
			       CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_unlock (impl_POA_OAF_ObjectDirectory * servant,
				 CORBA_Environment * ev);

static OAF_RegistrationResult
impl_OAF_ObjectDirectory_register (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   CORBA_Object obj, CORBA_Environment * ev);

static void
impl_OAF_ObjectDirectory_unregister (impl_POA_OAF_ObjectDirectory * servant,
				     OAF_ImplementationID iid,
				     CORBA_Object obj,
				     CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ObjectDirectory_base_epv = {
	NULL,			/* _private data */
	NULL,			/* finalize routine */
	NULL			/* default_POA routine */
};
static POA_OAF_ObjectDirectory__epv impl_OAF_ObjectDirectory_epv = {
	NULL,			/* _private */
	&impl_OAF_ObjectDirectory__get_servers,
	&impl_OAF_ObjectDirectory_get_active_servers,
	&impl_OAF_ObjectDirectory__get_username,
	&impl_OAF_ObjectDirectory__get_hostID,
	&impl_OAF_ObjectDirectory__get_domain,
	&impl_OAF_ObjectDirectory_activate,
	&impl_OAF_ObjectDirectory_lock,
	&impl_OAF_ObjectDirectory_unlock,
	&impl_OAF_ObjectDirectory_register,
	&impl_OAF_ObjectDirectory_unregister
};

/*** vepv structures ***/

static POA_OAF_ObjectDirectory__vepv impl_OAF_ObjectDirectory_vepv = {
	&impl_OAF_ObjectDirectory_base_epv,
	&impl_OAF_ObjectDirectory_epv
};

/*** Stub implementations ***/

#ifdef OAF_DEBUG
static void
od_dump_list (impl_POA_OAF_ObjectDirectory * od)
{
	int i, j, k;

	for (i = 0; i < od->attr_servers._length; i++) {
		g_print ("IID %s, type %s, location %s\n",
			 od->attr_servers._buffer[i].iid,
			 od->attr_servers._buffer[i].server_type,
			 od->attr_servers._buffer[i].location_info);
		for (j = 0; j < od->attr_servers._buffer[i].attrs._length;
		     j++) {
			OAF_Attribute *attr =
				&(od->attr_servers._buffer[i].
				  attrs._buffer[j]);
			g_print ("    %s = ", attr->name);
			switch (attr->v._d) {
			case OAF_A_STRING:
				g_print ("\"%s\"\n", attr->v._u.value_string);
				break;
			case OAF_A_NUMBER:
				g_print ("%f\n", attr->v._u.value_number);
				break;
			case OAF_A_BOOLEAN:
				g_print ("%s\n",
					 attr->v.
					 _u.value_boolean ? "TRUE" : "FALSE");
				break;
			case OAF_A_STRINGV:
				g_print ("[");
				for (k = 0;
				     k < attr->v._u.value_stringv._length;
				     k++) {
					g_print ("\"%s\"",
						 attr->v._u.
						 value_stringv._buffer[k]);
					if (k <
					    (attr->v._u.
					     value_stringv._length - 1))
						g_print (", ");
				}
				g_print ("]\n");
				break;
			}
		}
	}
}
#endif

OAF_ObjectDirectory
OAF_ObjectDirectory_create (PortableServer_POA poa,
			    const char *domain,
			    const char *source_directory,
			    CORBA_Environment * ev)
{
	OAF_ObjectDirectory retval;
	impl_POA_OAF_ObjectDirectory *newservant;
	PortableServer_ObjectId *objid;

	newservant = g_new0 (impl_POA_OAF_ObjectDirectory, 1);
	newservant->servant.vepv = &impl_OAF_ObjectDirectory_vepv;
	newservant->poa = poa;
	POA_OAF_ObjectDirectory__init ((PortableServer_Servant) newservant,
				       ev);
	objid = PortableServer_POA_activate_object (poa, newservant, ev);
	CORBA_free (objid);
	newservant->self = retval =
		PortableServer_POA_servant_to_reference (poa, newservant, ev);

	newservant->attr_domain = g_strdup (domain);
	newservant->attr_hostID = oaf_hostname_get ();
	newservant->by_iid = NULL;
	newservant->attr_servers._buffer =
		OAF_ServerInfo_load (source_directory,
				     &newservant->attr_servers._length,
				     &newservant->by_iid,
				     g_get_user_name (),
				     newservant->attr_hostID,
				     newservant->attr_domain);
	newservant->active_servers =
		g_hash_table_new (g_str_hash, g_str_equal);
	newservant->time_list_changed = time (NULL);

#if defined(OAF_DEBUG) && 0
	od_dump_list (newservant);
#endif

	return CORBA_Object_duplicate (retval, ev);
}

static OAF_ServerInfoListCache *
impl_OAF_ObjectDirectory__get_servers (impl_POA_OAF_ObjectDirectory * servant,
				       OAF_CacheTime only_if_newer,
				       CORBA_Environment * ev)
{
	OAF_ServerInfoListCache *retval;

	retval = OAF_ServerInfoListCache__alloc ();

	retval->_d = (only_if_newer < servant->time_list_changed);
	if (retval->_d) {
		retval->_u.server_list = servant->attr_servers;
		CORBA_sequence_set_release (&retval->_u.server_list,
					    CORBA_FALSE);
	}

	return retval;
}

typedef struct
{
	OAF_ImplementationID *seq;
	int last_used;
}
StateCollectionInfo;

static void
add_active_server (char *key, gpointer value, StateCollectionInfo * sci)
{
	sci->seq[(sci->last_used)++] = CORBA_string_dup (key);
}

static OAF_ServerStateCache *
impl_OAF_ObjectDirectory_get_active_servers (impl_POA_OAF_ObjectDirectory *
					     servant,
					     OAF_CacheTime only_if_newer,
					     CORBA_Environment * ev)
{
	OAF_ServerStateCache *retval;

	retval = OAF_ServerStateCache__alloc ();

	retval->_d = (only_if_newer < servant->time_active_changed);
	if (retval->_d) {
		StateCollectionInfo sci;

		retval->_u.active_servers._length =
			g_hash_table_size (servant->active_servers);
		retval->_u.active_servers._buffer = sci.seq =
			CORBA_sequence_OAF_ImplementationID_allocbuf
			(retval->_u.active_servers._length);
		sci.last_used = 0;

		g_hash_table_foreach (servant->active_servers,
				      (GHFunc) add_active_server, &sci);
		CORBA_sequence_set_release (&(retval->_u.active_servers),
					    CORBA_TRUE);
	}

	return retval;
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_domain (impl_POA_OAF_ObjectDirectory * servant,
				      CORBA_Environment * ev)
{
	return CORBA_string_dup (servant->attr_domain);
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_hostID (impl_POA_OAF_ObjectDirectory * servant,
				      CORBA_Environment * ev)
{
	return CORBA_string_dup (servant->attr_hostID);
}

static CORBA_char *
impl_OAF_ObjectDirectory__get_username (impl_POA_OAF_ObjectDirectory *
					servant, CORBA_Environment * ev)
{
	return CORBA_string_dup (g_get_user_name ());
}

static CORBA_Object
impl_OAF_ObjectDirectory_activate (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   OAF_ActivationContext ac,
				   OAF_ActivationFlags flags,
				   CORBA_Context ctx, CORBA_Environment * ev)
{
	CORBA_Object retval;
	OAF_ServerInfo *si;
	ODActivationInfo ai;

	ai.ac = ac;
	ai.flags = flags;
	ai.ctx = ctx;
	retval = CORBA_OBJECT_NIL;

	si = g_hash_table_lookup (servant->by_iid, iid);

	retval = g_hash_table_lookup (servant->active_servers, iid);

	if (!CORBA_Object_is_nil (retval, ev)
	    && !CORBA_Object_non_existent (retval, ev)
	    && !(flags & OAF_FLAG_IGNORE_EXISTING))
		return CORBA_Object_duplicate (retval, ev);

	if (flags & OAF_FLAG_EXISTING_ONLY)
		return CORBA_OBJECT_NIL;

	if (si)
		retval = od_server_activate (si, &ai, servant->self, ev);

	return retval;
}

static void
impl_OAF_ObjectDirectory_lock (impl_POA_OAF_ObjectDirectory * servant,
			       CORBA_Environment * ev)
{
	while (servant->is_locked)
		g_main_iteration (TRUE);

	servant->is_locked = TRUE;
}

static void
impl_OAF_ObjectDirectory_unlock (impl_POA_OAF_ObjectDirectory * servant,
				 CORBA_Environment * ev)
{
	g_return_if_fail (servant->is_locked);

	servant->is_locked = FALSE;
}

static OAF_RegistrationResult
impl_OAF_ObjectDirectory_register (impl_POA_OAF_ObjectDirectory * servant,
				   OAF_ImplementationID iid,
				   CORBA_Object obj, CORBA_Environment * ev)
{
	CORBA_Object oldobj;

	oldobj = g_hash_table_lookup (servant->active_servers, iid);

	if (!CORBA_Object_is_nil (oldobj, ev)) {
		if (!CORBA_Object_non_existent (oldobj, ev))
			return OAF_REG_ALREADY_ACTIVE;
		else
			CORBA_Object_release (oldobj, ev);
	}


	if (!g_hash_table_lookup (servant->by_iid, iid))
		return OAF_REG_NOT_LISTED;

	g_hash_table_insert (servant->active_servers,
			     oldobj ? iid : g_strdup (iid),
			     CORBA_Object_duplicate (obj, ev));
	servant->time_active_changed = time (NULL);

	return OAF_REG_SUCCESS;
}

static void
impl_OAF_ObjectDirectory_unregister (impl_POA_OAF_ObjectDirectory * servant,
				     OAF_ImplementationID iid,
				     CORBA_Object obj, CORBA_Environment * ev)
{
	char *orig_iid;
	CORBA_Object orig_obj;

	if (!g_hash_table_lookup_extended
	    (servant->active_servers, iid, (gpointer *) & orig_iid,
	     (gpointer *) & orig_obj)
	    || !CORBA_Object_is_equivalent (orig_obj, obj, ev))
		return;

	g_hash_table_remove (servant->active_servers, iid);

	g_free (orig_iid);
	CORBA_Object_release (orig_obj, ev);

	servant->time_active_changed = time (NULL);
}
