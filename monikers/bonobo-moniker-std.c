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
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-main.h>

#include <gtk/gtk.h>

#include "bonobo-moniker-std.h"

static BonoboGenericFactory *std_factory = NULL;

static BonoboObject *
bonobo_std_moniker_factory (BonoboGenericFactory *this,
			    const char           *object_id,
			    void                 *data)
{
	g_return_val_if_fail (object_id != NULL, NULL);

	if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_File"))

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
			"file:", bonobo_moniker_file_resolve));

	else if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_Item"))

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
			"!", bonobo_moniker_item_resolve));
	
	else if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_Oaf"))

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
			"oafiid:", bonobo_moniker_oaf_resolve));

	else if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_Cache"))

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
			"cache:", bonobo_moniker_cache_resolve));

	else if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_New"))

		return BONOBO_OBJECT (bonobo_moniker_simple_new (
			"new:", bonobo_moniker_new_resolve));

/*
 * Deprecated until Miguel likes it.
 *
 *	else if (!strcmp (object_id, "OAFIID:Bonobo_Moniker_Query"))
 *		
 *		return BONOBO_OBJECT (bonobo_moniker_simple_new (
 *			"query:", bonobo_moniker_query_resolve));
 */


	else if (!strcmp (object_id, "OAFIID:Bonobo_MonikerExtender_file"))
		
		return BONOBO_OBJECT (bonobo_moniker_extender_new (
			bonobo_file_extender_resolve, NULL));

	else if (!strcmp (object_id, "OAFIID:Bonobo_MonikerExtender_stream"))
		
		return BONOBO_OBJECT (bonobo_moniker_extender_new (
			bonobo_stream_extender_resolve, NULL));

	else
		g_warning ("Failing to manufacture a '%s'", object_id);

	return NULL;
}

static CORBA_Object
make_std_factory (PortableServer_POA poa,
		  const char *iid,
		  gpointer impl_ptr,
		  CORBA_Environment *ev)
{
	BonoboShlibFactory *f;
        CORBA_Object object_ref;

	f = bonobo_shlib_factory_new_multi (
	        "OAFIID:Bonobo_Moniker_stdFactory", poa, impl_ptr,
		bonobo_std_moniker_factory, NULL);
	
        object_ref = bonobo_object_corba_objref (BONOBO_OBJECT (f));

        if (object_ref == CORBA_OBJECT_NIL 
            || ev->_major != CORBA_NO_EXCEPTION) {
                printf ("Server cannot get objref\n");
                return CORBA_OBJECT_NIL;
        }
	
        return object_ref;
}

static const OAFPluginObject std_plugin_list[] = {
        {
                "OAFIID:Bonobo_Moniker_stdFactory",
                make_std_factory
        },
        {
                NULL
  	}
};


const OAFPlugin OAF_Plugin_info = {
        std_plugin_list,
        "bonobo standard moniker"
};


