/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "liboaf-private.h"

#include <gmodule.h>
/* ORBit-specific hack */
#include <orb/orbit_poa.h>

typedef struct
{
	GModule *loaded;
	int refcount;
	char filename[1];
}
ActivePluginInfo;

static GHashTable *living_by_filename = NULL;

static void
gnome_plugin_unload (gpointer data, gpointer user_data)
{
	ActivePluginInfo *api = user_data;
	g_module_close (api->loaded);
	g_hash_table_remove (living_by_filename, api->filename);
	g_free (api);
}

CORBA_Object
oaf_server_activate_shlib (OAF_ActivationResult * sh, CORBA_Environment * ev)
{
	CORBA_Object retval;
	const OAFPlugin *plugin;
	ActivePluginInfo *local_plugin_info = NULL;
	const OAFPluginObject *pobj;
	int i;
	PortableServer_POA poa;
	CORBA_ORB orb;
	char *filename;
	const char *iid;

	g_return_val_if_fail (sh->res._d == OAF_RESULT_SHLIB,
			      CORBA_OBJECT_NIL);
	g_return_val_if_fail (sh->res._u.res_shlib._length > 0,
			      CORBA_OBJECT_NIL);

	/* The location info is at the end to of the string list */
	filename = sh->res._u.res_shlib._buffer[sh->res._u.res_shlib._length - 1];
	g_message ("%s", filename);
	if (living_by_filename)
		local_plugin_info =
			g_hash_table_lookup (living_by_filename, filename);

	if (!local_plugin_info) {
		/* We have to load the thing from scratch */
		GModule *gmod;
		gboolean success;

		gmod = g_module_open (filename, G_MODULE_BIND_LAZY);
		if (!gmod) {
			return CORBA_OBJECT_NIL;	/* Couldn't load it */
		}
		
		success = g_module_symbol (gmod, "OAF_Plugin_info",
					   (gpointer *) &plugin);
		if (!success) {
			g_module_close (gmod);
			return CORBA_OBJECT_NIL;
		}

		local_plugin_info =
			g_malloc (sizeof (ActivePluginInfo) +
				  strlen (filename) + 1);

		local_plugin_info->refcount = 0;
		local_plugin_info->loaded = gmod;
		strcpy (local_plugin_info->filename, filename);

		if (!living_by_filename)
			living_by_filename =
				g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (living_by_filename,
				     local_plugin_info->filename,
				     local_plugin_info);
	} else {
		int success;

		success =
			g_module_symbol (local_plugin_info->loaded,
					 "OAF_Plugin_info",
					 (gpointer *) & plugin);
		if (!success) {
			return CORBA_OBJECT_NIL;
		}
	}

	retval = CORBA_OBJECT_NIL;

	orb = oaf_orb_get ();
	poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (orb, "RootPOA", ev);

	/* Index into the string list one element from the end to get the iid of the shlib */
	iid = sh->res._u.res_shlib._buffer[sh->res._u.res_shlib._length - 2];
	for (pobj = plugin->plugin_object_list; pobj->iid; pobj++) {
		if (strcmp (iid, pobj->iid) == 0) {
			/* Found a match */
			break;
		}
	}

	if (pobj->iid) {
		/* Activate the shlib */
		retval = pobj->activate (poa, pobj->iid, local_plugin_info, ev);

		if (ev->_major != CORBA_NO_EXCEPTION)
			retval = CORBA_OBJECT_NIL;

		/* Activate the factiories contained in the shlib */
		i =  sh->res._u.res_shlib._length - 2;
		for (i--; i >= 0 && !CORBA_Object_is_nil (retval, ev); i--) {
			CORBA_Object new_retval;
			GNOME_stringlist dummy = { 0 };

			iid = sh->res._u.res_shlib._buffer[i];

			new_retval =
				GNOME_GenericFactory_create_object (retval,
								    (char *)
								    iid,
								    &dummy,
								    ev);
			if (ev->_major != CORBA_NO_EXCEPTION
			    || CORBA_Object_is_nil (new_retval, ev))
				new_retval = CORBA_OBJECT_NIL;

			CORBA_Object_release (retval, ev);
			retval = new_retval;
		}
	}

	return retval;
}

/**
 * oaf_plugin_unuse:
 * @servant: The servant that was created
 * @impl_ptr: The impl_ptr that was passed to the original activation routine
 *
 * Side effects: May arrange for the shared library that the
 * implementation is in to be unloaded.
 *
 * When a shlib plugin for a CORBA object is destroying an
 * implementation, it should call this function to make sure that the
 * shared library is unloaded as needed.
 *
 * Calling this routine is mandatory for activate() routines listed in
 * an OAFPluginObject struct, otherwise Bad Things Will Happen.
 */
void
oaf_plugin_use (PortableServer_Servant servant, gpointer impl_ptr)
{
	ActivePluginInfo *local_plugin_info = impl_ptr;

	local_plugin_info->refcount++;

#if 0
	ORBit_servant_set_deathwatch (servant, &(local_plugin_info->refcount),
				      gnome_plugin_unload, local_plugin_info);
#endif
}

/**
 * oaf_plugin_unuse:
 * @impl_ptr: The impl_ptr that was passed to the activation routine
 *
 * Side effects: May arrange for the shared library that the
 * implementation is in to be unloaded.
 *
 * When a shlib plugin for a CORBA object is destroying an
 * implementation, it should call this function to make sure that the
 * shared library is unloaded as needed.
 */
void
oaf_plugin_unuse (gpointer impl_ptr)
{
	ActivePluginInfo *api;

	g_return_if_fail (impl_ptr);

	api = impl_ptr;

	api->refcount--;

	if (api->refcount <= 0)
		gnome_plugin_unload (&(api->refcount), api);
}
