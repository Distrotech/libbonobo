/*
 * gnome-moniker-cache.c: 
 *
 * Author:
 *	Dietmar Maurer (dietmar@helixcode.com)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <stdlib.h>

#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-init.h>

#include <liboaf/liboaf.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-simple.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-main.h>

#include <gtk/gtk.h>

#include "bonobo-moniker-cache.h"

static int running_objects;
static BonoboGenericFactory *cache_factory = NULL;

static Bonobo_Unknown
cache_resolve (BonoboMoniker               *moniker,
	       const Bonobo_ResolveOptions *options,
	       const CORBA_char            *requested_interface,
	       CORBA_Environment           *ev)
{
	g_warning ("cache_resolve: not implemented");

	return CORBA_OBJECT_NIL; /* use the extender */
}

static void
moniker_cache_destroy_cb (BonoboMoniker *config, void *data)
{
	gpointer impl_ptr;

	running_objects--;

	if (running_objects > 0)
		return;

	impl_ptr = gtk_object_get_data (GTK_OBJECT (cache_factory), 
					"OAF_IMPL_PTR");

	oaf_plugin_unuse (impl_ptr);
      
	bonobo_object_unref (BONOBO_OBJECT (cache_factory));

	cache_factory = NULL;
}

static BonoboMoniker *
bonobo_moniker_cache_new (void)
{
	return bonobo_moniker_simple_new (
		"cache:", cache_resolve);
}

static BonoboObject *
bonobo_cache_factory (BonoboGenericFactory *this,
		      void *closure)
{
	BonoboMoniker *cache_moniker = bonobo_moniker_cache_new ();

	running_objects++;

	gtk_signal_connect (GTK_OBJECT (cache_moniker), "destroy",
			    GTK_SIGNAL_FUNC (moniker_cache_destroy_cb), NULL);
	
	return BONOBO_OBJECT (cache_moniker);
}

static CORBA_Object
make_cache_factory (PortableServer_POA poa,
		    const char *iid,
		    gpointer impl_ptr,
		    CORBA_Environment *ev)
{
        CORBA_Object object_ref;

	cache_factory = bonobo_generic_factory_new (
		"OAFIID:Bonobo_Moniker_cacheFactory",
		bonobo_cache_factory, NULL);	

	gtk_object_set_data (GTK_OBJECT (cache_factory), "OAF_IMPL_PTR", 
			     impl_ptr);

        object_ref = bonobo_object_corba_objref (BONOBO_OBJECT(cache_factory));
        if (object_ref == CORBA_OBJECT_NIL 
            || ev->_major != CORBA_NO_EXCEPTION) {
                printf ("Server cannot get objref\n");
                return CORBA_OBJECT_NIL;
        }

        oaf_plugin_use (poa, impl_ptr);

        return object_ref;
}

static const OAFPluginObject cache_plugin_list[] = {
        {
                "OAFIID:Bonobo_Moniker_cacheFactory",
                make_cache_factory
        },
        {
                NULL
  	}
};

const OAFPlugin OAF_Plugin_info = {
        cache_plugin_list,
        "bonobo cache moniker"
};
