/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-moniker: Object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>

struct _BonoboMonikerPrivate {
	Bonobo_Moniker parent;
	char          *name;
};

static GtkObjectClass *bonobo_moniker_parent_class;

POA_Bonobo_Moniker__vepv bonobo_moniker_vepv;

#define CLASS(o) BONOBO_MONIKER_CLASS (GTK_OBJECT (o)->klass)

static inline BonoboMoniker *
bonobo_moniker_from_servant (PortableServer_Servant servant)
{
	return BONOBO_MONIKER (bonobo_object_from_servant (servant));
}

static Bonobo_Moniker
impl_get_parent (PortableServer_Servant servant,
		 CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->get_parent (moniker, ev);
}

static void
impl_set_parent (PortableServer_Servant servant,
		 const Bonobo_Moniker   value,
		 CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->set_parent (moniker, value, ev);
}

void
bonobo_moniker_set_parent (BonoboMoniker     *moniker,
			   Bonobo_Moniker     parent,
			   CORBA_Environment *ev)
{
	g_return_if_fail (BONOBO_IS_MONIKER (moniker));
	
	if (moniker->priv->parent != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (moniker->priv->parent, ev);

	if (parent != CORBA_OBJECT_NIL)
		moniker->priv->parent =
			bonobo_object_dup_ref (parent, ev);
	else
		moniker->priv->parent = CORBA_OBJECT_NIL;
}

Bonobo_Moniker
bonobo_moniker_get_parent (BonoboMoniker     *moniker,
			   CORBA_Environment *ev)
{
	g_return_val_if_fail (BONOBO_IS_MONIKER (moniker),
			      CORBA_OBJECT_NIL);
	
	if (moniker->priv->parent == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;
	
	return bonobo_object_dup_ref (moniker->priv->parent, ev);
}

const char *
bonobo_moniker_get_name (BonoboMoniker *moniker,
			 int            char_offset)
{	
	g_return_val_if_fail (BONOBO_IS_MONIKER (moniker), "");

	if (moniker->priv->name)
		if (strlen (moniker->priv->name) >= char_offset)
			return & moniker->priv->name [char_offset];

	return "";
}

/**
 * bonobo_moniker_set_name:
 * @moniker: the BonoboMoniker to configure.
 * @name: new name for this moniker.
 * @num_chars: number of characters in name to copy.
 *
 * This functions sets the moniker name in @moniker to be @name.
 */
void
bonobo_moniker_set_name (BonoboMoniker *moniker,
			 const char    *name,
			 int            num_chars)
{
	g_return_if_fail (BONOBO_IS_MONIKER (moniker));

	g_free (moniker->priv->name);
	moniker->priv->name = g_strndup (name, num_chars);
}

static CORBA_char *
bonobo_moniker_default_get_display_name (BonoboMoniker     *moniker,
					 CORBA_Environment *ev)
{
	CORBA_char *ans, *parent_name;
	char       *tmp;
	
	parent_name = bonobo_moniker_util_get_parent_name (
		bonobo_object_corba_objref (BONOBO_OBJECT (moniker)), ev);

	if (BONOBO_EX(ev))
		return NULL;

	if (!parent_name)
		return CORBA_string_dup (moniker->priv->name);

	if (!moniker->priv->name)
		return parent_name;

	tmp = g_strdup_printf ("%s%s", parent_name, moniker->priv->name);

	if (parent_name)
		CORBA_free (parent_name);

	ans = CORBA_string_dup (tmp);

	g_free (tmp);
	
	return ans;
}

static CORBA_char *
impl_get_display_name (PortableServer_Servant servant,
		       CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);
	
	return CLASS (moniker)->get_display_name (moniker, ev);
}

static Bonobo_Moniker
impl_parse_display_name (PortableServer_Servant servant,
			 Bonobo_Moniker         parent,
			 const CORBA_char      *name,
			 CORBA_Environment     *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->parse_display_name (moniker, parent, name, ev);
}

static Bonobo_Unknown
impl_resolve (PortableServer_Servant       servant,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char            *requested_interface,
	      CORBA_Environment           *ev)
{
	BonoboMoniker *moniker = bonobo_moniker_from_servant (servant);

	return CLASS (moniker)->resolve (moniker, options,
					 requested_interface, ev);
}

/**
 * bonobo_moniker_get_epv:
 *
 * Returns: The EPV for the default Bonobo Moniker implementation.
 */
POA_Bonobo_Moniker__epv *
bonobo_moniker_get_epv (void)
{
	POA_Bonobo_Moniker__epv *epv;

	epv = g_new0 (POA_Bonobo_Moniker__epv, 1);

	epv->_get_parent      = impl_get_parent;
	epv->_set_parent      = impl_set_parent;
	epv->getDisplayName   = impl_get_display_name;
	epv->parseDisplayName = impl_parse_display_name;
	epv->resolve          = impl_resolve;

	return epv;
}

static void
init_moniker_corba_class (void)
{
	/* The VEPV */
	bonobo_moniker_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_moniker_vepv.Bonobo_Moniker_epv = bonobo_moniker_get_epv ();
}

static void
bonobo_moniker_destroy (GtkObject *object)
{
	BonoboMoniker *moniker = BONOBO_MONIKER (object);

	if (moniker->priv->parent != CORBA_OBJECT_NIL)
		bonobo_object_release_unref (moniker->priv->parent, NULL);

	g_free (moniker->priv->name);

	g_free (moniker->priv);

	GTK_OBJECT_CLASS (bonobo_moniker_parent_class)->destroy (object);
}

static void
bonobo_moniker_init (GtkObject *object)
{
	BonoboMoniker *moniker = BONOBO_MONIKER (object);

	moniker->priv = g_new (BonoboMonikerPrivate, 1);

	moniker->priv->parent = CORBA_OBJECT_NIL;
	moniker->priv->name   = NULL;
}

static void
bonobo_moniker_class_init (BonoboMonikerClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *)klass;

	bonobo_moniker_parent_class = gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_moniker_destroy;

	klass->get_parent = bonobo_moniker_get_parent;
	klass->set_parent = bonobo_moniker_set_parent;
	klass->get_display_name = bonobo_moniker_default_get_display_name;

	init_moniker_corba_class ();
}

/**
 * bonobo_moniker_get_type:
 *
 * Returns: the GtkType for a BonoboMoniker.
 */
GtkType
bonobo_moniker_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMoniker",
			sizeof (BonoboMoniker),
			sizeof (BonoboMonikerClass),
			(GtkClassInitFunc) bonobo_moniker_class_init,
			(GtkObjectInitFunc) bonobo_moniker_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_moniker_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Moniker
bonobo_moniker_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Moniker *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Moniker *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_moniker_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Moniker__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

