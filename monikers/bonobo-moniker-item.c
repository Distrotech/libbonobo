/*
 * gnome-moniker-item.c: Sample item-system based Moniker implementation
 *
 * This is the item (container) based Moniker implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 */
#include <config.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>

#include <liboaf/liboaf.h>

#include "bonobo-moniker-item.h"

#define PREFIX_LEN (sizeof ("!") - 1)

static BonoboMonikerClass *bonobo_moniker_item_parent_class;

static Bonobo_Moniker 
item_parse_display_name (BonoboMoniker     *moniker,
			 Bonobo_Moniker     parent,
			 const CORBA_char  *name,
			 CORBA_Environment *ev)
{
	BonoboMonikerItem *m_item = BONOBO_MONIKER_ITEM (moniker);
	int i;

	g_return_val_if_fail (m_item != NULL,
			      CORBA_OBJECT_NIL);

	bonobo_moniker_set_parent (moniker, parent, ev);

	i = bonobo_moniker_util_seek_std_separator (name, PREFIX_LEN);
	bonobo_moniker_set_name (moniker, name, i);

	if (name [i])
		return bonobo_moniker_util_new_from_name_full (
			bonobo_object_corba_objref (BONOBO_OBJECT (m_item)),
			&name [i], ev);
	else
		return bonobo_object_dup_ref (
			bonobo_object_corba_objref (BONOBO_OBJECT (m_item)), ev);
}

static Bonobo_Unknown
item_resolve (BonoboMoniker               *moniker,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char            *requested_interface,
	      CORBA_Environment           *ev)
{
	Bonobo_Moniker       parent;
	Bonobo_ItemContainer container;
	Bonobo_Unknown       containee;
	Bonobo_Unknown       retval = CORBA_OBJECT_NIL;
	
	parent = bonobo_moniker_get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	if (parent == CORBA_OBJECT_NIL) {
		g_warning ("Item moniker with no parent !");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	container = Bonobo_Moniker_resolve (parent, options,
					    "IDL:Bonobo/ItemContainer:1.0", ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto return_unref_parent;

	if (container == CORBA_OBJECT_NIL) {
		g_warning ("Failed to extract a container from our parent");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		goto return_unref_parent;
	}

	containee = Bonobo_ItemContainer_getObjectByName (
		container, bonobo_moniker_get_name (moniker, PREFIX_LEN),
		TRUE, ev);

	bonobo_object_release_unref (container, ev);

	return bonobo_moniker_util_qi_return (containee, requested_interface, ev);

 return_unref_parent:
	bonobo_object_release_unref (parent, ev);

	return retval;
}

static void
bonobo_moniker_item_class_init (BonoboMonikerItemClass *klass)
{
	BonoboMonikerClass *mclass = (BonoboMonikerClass *) klass;
	
	bonobo_moniker_item_parent_class = gtk_type_class (
		bonobo_moniker_get_type ());

	mclass->parse_display_name = item_parse_display_name;
	mclass->resolve            = item_resolve;
}

/**
 * bonobo_moniker_item_get_type:
 *
 * Returns the GtkType for the BonoboMonikerItem class.
 */
GtkType
bonobo_moniker_item_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMonikerItem",
			sizeof (BonoboMonikerItem),
			sizeof (BonoboMonikerItemClass),
			(GtkClassInitFunc) bonobo_moniker_item_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_moniker_get_type (), &info);
	}

	return type;
}
