/*
 * gnome-moniker-oaf.c: Sample oaf-system based Moniker implementation
 *
 * This is the oaf-activation based Moniker implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 */
#include <config.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>

#include <liboaf/liboaf.h>

#include "bonobo-moniker-oaf.h"

static BonoboMonikerClass *bonobo_moniker_oaf_parent_class;

static Bonobo_Unknown
oaf_resolve (BonoboMoniker               *moniker,
	     const Bonobo_ResolveOptions *options,
	     const CORBA_char            *requested_interface,
	     CORBA_Environment           *ev)
{
	Bonobo_Moniker       parent;
	Bonobo_Unknown       object;
	
	parent = bonobo_moniker_get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	if (parent != CORBA_OBJECT_NIL) {
		bonobo_object_release_unref (parent, ev);

		g_warning ("wierd; oafid moniker with a parent; strange");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	object = oaf_activate_from_id (
		(char *) bonobo_moniker_get_name (moniker), 0, NULL, ev);

	return bonobo_moniker_util_qi_return (object, requested_interface, ev);
}

static void
bonobo_moniker_oaf_class_init (BonoboMonikerOafClass *klass)
{
	BonoboMonikerClass *mclass = (BonoboMonikerClass *) klass;
	
	bonobo_moniker_oaf_parent_class = gtk_type_class (
		bonobo_moniker_get_type ());

	mclass->resolve = oaf_resolve;
}

/**
 * bonobo_moniker_oaf_get_type:
 *
 * Returns the GtkType for the BonoboMonikerOaf class.
 */
GtkType
bonobo_moniker_oaf_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMonikerOaf",
			sizeof (BonoboMonikerOaf),
			sizeof (BonoboMonikerOafClass),
			(GtkClassInitFunc) bonobo_moniker_oaf_class_init,
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
bonobo_moniker_oaf_new (void)
{
	return bonobo_moniker_construct (
		gtk_type_new (bonobo_moniker_oaf_get_type ()),
		CORBA_OBJECT_NIL, "");
}
