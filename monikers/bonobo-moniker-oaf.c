/*
 * gnome-moniker-oaf.c: Sample oaf-system based Moniker implementation
 *
 * This is the oaf-activation based Moniker implementation.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 */
#include <config.h>
#include <gnome.h>
#include <bonobo/bonobo.h>
#include <liboaf/liboaf.h>

#include "bonobo-moniker-oaf.h"

static BonoboMonikerClass *bonobo_moniker_oaf_parent_class;

static Bonobo_Moniker 
oaf_parse_display_name (BonoboMoniker     *moniker,
			Bonobo_Moniker     parent,
			const CORBA_char  *name,
			CORBA_Environment *ev)
{
	BonoboMonikerOaf *m_oaf = BONOBO_MONIKER_OAF (moniker);
	int i;

	g_return_val_if_fail (m_oaf != NULL, CORBA_OBJECT_NIL);

	bonobo_moniker_set_parent (moniker, parent, ev);

	i = bonobo_moniker_util_seek_std_separator (name, 0);

	bonobo_moniker_set_name (moniker, name, i);

	if (name [i])
		return bonobo_moniker_util_new_from_name_full (
			bonobo_object_corba_objref (BONOBO_OBJECT (m_oaf)),
			&name [i], ev);
	else
		return bonobo_object_dup_ref (
			bonobo_object_corba_objref (BONOBO_OBJECT (m_oaf)), ev);
}

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
		(char *)bonobo_moniker_get_name (moniker, 0), 0, NULL, ev);

	return bonobo_moniker_util_qi_return (object, requested_interface, ev);
}

static void
bonobo_moniker_oaf_class_init (BonoboMonikerOafClass *klass)
{
	BonoboMonikerClass *mclass = (BonoboMonikerClass *) klass;
	
	bonobo_moniker_oaf_parent_class = gtk_type_class (
		bonobo_moniker_get_type ());

	mclass->parse_display_name = oaf_parse_display_name;
	mclass->resolve            = oaf_resolve;
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
