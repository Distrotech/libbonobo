/*
 * gnome-moniker-new.c: Sample generic factory 'new' Moniker implementation
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 */
#include <config.h>

#include <liboaf/liboaf.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include "bonobo-moniker-new.h"

static BonoboMonikerClass *bonobo_moniker_new_parent_class;

static Bonobo_Unknown
new_resolve (BonoboMoniker               *moniker,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char            *requested_interface,
	      CORBA_Environment           *ev)
{
	Bonobo_Moniker      parent;
	GNOME_ObjectFactory factory;
	Bonobo_Unknown      containee;
	Bonobo_Unknown      retval = CORBA_OBJECT_NIL;
	GNOME_stringlist params = { 0 };
	
	parent = bonobo_moniker_get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	g_assert (parent != CORBA_OBJECT_NIL);

	factory = Bonobo_Moniker_resolve (parent, options,
					  "IDL:Gnome/ObjectFactory:1.0", ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto return_unref_parent;

	if (factory == CORBA_OBJECT_NIL) {
		g_warning ("Failed to extract a factory from our parent");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		goto return_unref_parent;
	}

	containee = GNOME_ObjectFactory_create_object (
		factory, requested_interface, &params, ev);

	bonobo_object_release_unref (factory, ev);

	return bonobo_moniker_util_qi_return (containee, requested_interface, ev);

 return_unref_parent:
	bonobo_object_release_unref (parent, ev);

	return retval;
}

static void
bonobo_moniker_new_class_init (BonoboMonikerNewClass *klass)
{
	BonoboMonikerClass *mclass = (BonoboMonikerClass *) klass;
	
	bonobo_moniker_new_parent_class = gtk_type_class (
		bonobo_moniker_get_type ());

	mclass->resolve = new_resolve;
}

/**
 * bonobo_moniker_new_get_type:
 *
 * Returns the GtkType for the BonoboMonikerNew class.
 */
GtkType
bonobo_moniker_new_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMonikerNew",
			sizeof (BonoboMonikerNew),
			sizeof (BonoboMonikerNewClass),
			(GtkClassInitFunc) bonobo_moniker_new_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_moniker_get_type (), &info);
	}

	return type;
}

BonoboMoniker *
bonobo_moniker_new_new (void)
{
	return bonobo_moniker_construct (
		gtk_type_new (bonobo_moniker_new_get_type ()),
		CORBA_OBJECT_NIL, "new:");
}
