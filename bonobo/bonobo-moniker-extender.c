/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-moniker-extender: extending monikers
 *
 * Author:
 *	Dietmar Maurer (dietmar@maurer-it.com)
 *
 */
#include <config.h>
  
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <liboaf/liboaf.h>

static GtkObjectClass *bonobo_moniker_extender_parent_class;

POA_Bonobo_MonikerExtender__vepv bonobo_moniker_extender_vepv;

#define CLASS(o) BONOBO_MONIKER_EXTENDER_CLASS (GTK_OBJECT (o)->klass)

static inline BonoboMonikerExtender *
bonobo_moniker_extender_from_servant (PortableServer_Servant servant)
{
	return BONOBO_MONIKER_EXTENDER (bonobo_object_from_servant (servant));
}

static Bonobo_Unknown 
impl_Bonobo_MonikerExtender_resolve (PortableServer_Servant servant,
				     const Bonobo_Moniker   parent,
				     const CORBA_char      *display_name,
				     const CORBA_char      *requested_interface,
				     CORBA_Environment     *ev)
{
	BonoboMonikerExtender *extender = bonobo_moniker_extender_from_servant (servant);

	if (extender->resolve)
		return extender->resolve (extender, parent, display_name,
					  requested_interface, ev);
	else
		return CLASS (extender)->resolve (extender, parent, display_name,
						  requested_interface, ev);
}



/**
 * bonobo_moniker_extender_get_epv:
 *
 * Returns: The EPV for the default Bonobo Moniker Extender implementation.
 */
POA_Bonobo_MonikerExtender__epv *
bonobo_moniker_extender_get_epv (void)
{
	POA_Bonobo_MonikerExtender__epv *epv;

	epv = g_new0 (POA_Bonobo_MonikerExtender__epv, 1);

	epv->resolve          = impl_Bonobo_MonikerExtender_resolve;

	return epv;
}

static void
init_moniker_extender_corba_class (void)
{
	/* The VEPV */
	bonobo_moniker_extender_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_moniker_extender_vepv.Bonobo_MonikerExtender_epv = bonobo_moniker_extender_get_epv ();
}

static Bonobo_Unknown
bonobo_moniker_extender_resolve (BonoboMonikerExtender *extender,
				 const Bonobo_Moniker   parent,
				 const CORBA_char      *display_name,
				 const CORBA_char      *requested_interface,
				 CORBA_Environment     *ev)
{

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION, 
			     ex_Bonobo_MonikerExtender_InterfaceNotFound, NULL);

	return CORBA_OBJECT_NIL;
}

static void
bonobo_moniker_extender_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (bonobo_moniker_extender_parent_class)->destroy (object);
}

static void
bonobo_moniker_extender_init (GtkObject *object)
{
}

static void
bonobo_moniker_extender_class_init (BonoboMonikerExtenderClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *)klass;

	bonobo_moniker_extender_parent_class = gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_moniker_extender_destroy;

	klass->resolve = bonobo_moniker_extender_resolve;

	init_moniker_extender_corba_class ();
}

/**
 * bonobo_moniker__extender_get_type:
 *
 * Returns: the GtkType for a BonoboMonikerExtender.
 */
GtkType
bonobo_moniker_extender_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMonikerExtender",
			sizeof (BonoboMonikerExtender),
			sizeof (BonoboMonikerExtenderClass),
			(GtkClassInitFunc) bonobo_moniker_extender_class_init,
			(GtkObjectInitFunc) bonobo_moniker_extender_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_moniker_extender_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Moniker
bonobo_moniker_extender_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_MonikerExtender *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_MonikerExtender *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_moniker_extender_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_MonikerExtender__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)) {
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

BonoboMonikerExtender *
bonobo_moniker_extender_construct (BonoboMonikerExtender *extender,
				   Bonobo_MonikerExtender corba_extender)
{
	return BONOBO_MONIKER_EXTENDER (
		bonobo_object_construct (BONOBO_OBJECT (extender), corba_extender));
}

BonoboMonikerExtender *
bonobo_moniker_extender_new (BonoboMonikerExtenderFn resolve, gpointer data)
{
	BonoboMonikerExtender *extender = NULL;
	Bonobo_MonikerExtender corba_extender;
	
	extender = gtk_type_new (bonobo_moniker_extender_get_type ());

	extender->resolve = resolve;
	extender->data = data;

	corba_extender = bonobo_moniker_extender_corba_object_create (
		BONOBO_OBJECT (extender));

	if (corba_extender == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (extender));
		return NULL;
	}

	return bonobo_moniker_extender_construct (extender, corba_extender);
}

Bonobo_Unknown              
bonobo_moniker_find_extender (const gchar *name, 
			      const gchar *interface, 
			      CORBA_Environment *ev)
{
	gchar            *query;
	OAF_ActivationID  ret_id;
	Bonobo_Unknown    extender;

	query = g_strdup_printf (
		"repo_ids.has ('IDL:Bonobo/MonikerExtender:1.0') AND "
		"repo_ids.has ('%s') AND "
		"(bonobo:monikerextender == '%s')", interface, name);

	extender = oaf_activate (query, NULL, 0, &ret_id, ev);

	g_free (query);

	return extender;
}
