#include "liboaf-private.h"

#include <gmodule.h>

typedef struct {
  int refcount;
  char *filename;
  GModule *loaded;
  int mainloop_level_load;
  guint unload_id;
  gboolean unload_is_quit;
} ActivePluginInfo;

static GHashTable *living_by_filename = NULL;

static void
oaf_plugin_use(ActivePluginInfo *plugin)
{
  plugin->refcount++;

  if(plugin->unload_id != -1) {
    if(plugin->unload_is_quit)
      gtk_quit_remove(plugin->unload_id);
    else
      g_source_remove(plugin->unload_id);
    plugin->unload_id = -1;
  }
}

CORBA_Object
oaf_server_activate_shlib(OAF_ServerInfo *si,
			  CORBA_Environment *ev)
{
  CORBA_Object retval;
  const OAFPlugin *plugin;
  ActivePluginInfo *local_plugin_info = NULL;
  const OAFPluginObject *pobj;
  
  if (living_by_filename)
    local_plugin_info = g_hash_table_lookup(living_by_filename, si->location_info);

  if(!local_plugin_info)
    {
      /* We have to load the thing from scratch */
      GModule *gmod;
      char *ctmp;
      gboolean success;
      OAFPlugin *plugin;

      gmod = g_module_open(si->location_info, G_MODULE_BIND_LAZY);

#if 0
      if(!gmod && *(si->location_info) == '/')
	{
	  ctmp = gnome_libdir_file(si->location_info);
	  if(ctmp)
	    {
	      gmod = g_module_open(ctmp, G_MODULE_BIND_LAZY);
	      g_free(ctmp);
	    }
	}
#endif

      if(!gmod)
	return CORBA_OBJECT_NIL; /* Couldn't load it */

      success = g_module_symbol(gmod, "OAF_Plugin_info",
				(gpointer *)&plugin);
      if(!success)
	{
	  g_module_close(gmod);
	  return CORBA_OBJECT_NIL;
	}

      local_plugin_info = g_new0(ActivePluginInfo, 1);

      local_plugin_info->loaded = gmod;
      local_plugin_info->filename = g_strdup(si->location_info);
      local_plugin_info->mainloop_level_load = gtk_main_level();
      local_plugin_info->unload_id = -1;

      if(!living_by_filename)
	living_by_filename = g_hash_table_new(g_str_hash, g_str_equal);

      g_hash_table_insert(living_by_filename, local_plugin_info->filename, local_plugin_info);
    }

  retval = CORBA_OBJECT_NIL;

  oaf_plugin_use(local_plugin_info);
  
  for (pobj = plugin->plugin_object_list; pobj->iid; pobj++)
    {
      if (!strcmp (si->iid, pobj->iid))
	break;
    }
  
  if (pobj->iid) {
    PortableServer_POA poa;
    CORBA_ORB orb;

    orb = oaf_orb_get();
    poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", ev);
    
    retval = pobj->activate(poa, pobj->iid, local_plugin_info, ev);

    if(ev->_major != CORBA_NO_EXCEPTION)
      retval = CORBA_OBJECT_NIL;
  }

  if(CORBA_Object_is_nil(retval, ev))
    oaf_plugin_unuse(local_plugin_info);

  return retval;
}

static gboolean
gnome_plugin_unload(ActivePluginInfo *api)
{
  g_return_val_if_fail(api->refcount <= 0, FALSE);

  g_module_close(api->loaded);
  g_hash_table_remove(living_by_filename, api->filename);
  g_free(api->filename);
  g_free(api);

  return FALSE;
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
oaf_plugin_unuse(gpointer impl_ptr)
{
  ActivePluginInfo *api;

  api = impl_ptr;

  g_return_if_fail(api);

  api->refcount--;

  if(api->refcount <= 0) {
    if(api->unload_id != -1)
      g_warning("Plugin %s already queued for unload!", api->filename);

    if(gtk_main_level() <= api->mainloop_level_load) {
      api->unload_is_quit = FALSE;
      api->unload_id = g_idle_add_full(G_PRIORITY_LOW, gnome_plugin_unload, api, NULL);
    } else {
      api->unload_is_quit = TRUE;
      api->unload_id = gtk_quit_add(api->mainloop_level_load + 1, gnome_plugin_unload, api);
    }
  }
}
