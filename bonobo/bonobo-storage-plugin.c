/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Author:
 *   Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 2000 Maurer IT Systemlösungen (http://www.maurer-it.com)
 */

#include <config.h>
#include <glib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#include "bonobo-storage-plugin.h"

#define PLUGIN_PREFIX "libstorage_"

GList *storage_plugin_list = NULL;

static void
plugin_load (gchar *path)
{
	StoragePlugin *plugin;
	GModule *handle;
	StoragePluginInitFn init_plugin = NULL;

	if (!path) return;
        if (!(handle = g_module_open (path, G_MODULE_BIND_LAZY))) {
                g_warning ("Can't load storage plugin `%s': %s", path,
                           g_module_error ());
                return;
        }
        if (!g_module_symbol (handle, "init_storage_plugin",
                              (gpointer *) &init_plugin)) {
                g_warning ("Can't initialize storage plugin `%s': %s", path,
                           g_module_error ());
                return;
        }

	plugin = g_new0 (StoragePlugin, 1);
	plugin->handle = handle;
	plugin->filename = g_strdup (path);
	if (init_plugin (plugin) == 0) {
		storage_plugin_list = g_list_prepend (storage_plugin_list,
						      plugin);
		return;
	}

	g_module_close (plugin->handle);
	g_free (plugin->filename);
	g_free (plugin);
}

void 
bonobo_storage_load_plugins (void)
{
	DIR *dir;
	struct dirent *de;
	gchar *plugin_name;
	gint len;

	if (!g_module_supported ()) return;

	if (storage_plugin_list) return; /* already loaded */

	if ((dir = opendir (PLUGIN_DIR)) == NULL) return;

	while ((de = readdir (dir)) != NULL){
		len = strlen (de->d_name);

		if (len > (strlen (PLUGIN_PREFIX) + 3) &&
		    strncmp (de->d_name, PLUGIN_PREFIX, 
			     strlen (PLUGIN_PREFIX)) == 0 &&
		    strncmp (de->d_name + len - 3, ".so", 3) == 0){
			plugin_name=g_strconcat (PLUGIN_DIR, "/", 
						 de->d_name, NULL);
			plugin_load (plugin_name);
			g_free (plugin_name);
		}
	}
	closedir (dir);
}


StoragePlugin*
bonobo_storage_plugin_find (const gchar *name)
{
	GList *l;
	StoragePlugin *p;

	g_return_val_if_fail (name != NULL, NULL);

	if (!storage_plugin_list) bonobo_storage_load_plugins ();

	if (!storage_plugin_list) return NULL;

	l = storage_plugin_list;

	while (l) {
		p = (StoragePlugin *) l->data;
		if (!strcmp (p->name, name) &&
		    !strcmp (p->version, VERSION)) {
			return p;
		}
		l = l->next;
	}

	return NULL;
}
