/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-shlib-factory.c: a ShlibFactory object.
 *
 * The BonoboShlibFactory object is used to instantiate new
 * Bonobo::ShlibFactory objects.  It acts as a wrapper for the
 * Bonobo::ShlibFactory CORBA interface, and dispatches to
 * a user-specified factory function whenever its create_object()
 * method is invoked.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Ximian, Inc.
 */
#include <config.h>
#include <gobject/gsignal.h>
#include <gobject/gmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-running-context.h>

static BonoboObjectClass *bonobo_shlib_factory_parent_class = NULL;

struct _BonoboShlibFactoryPrivate {
	int                  live_objects;
	gpointer             oaf_impl_ptr;
};

/**
 * bonobo_shlib_factory_construct:
 * @factory: The object to be initialized.
 * @corba_factory: The CORBA object which supports the
 * @oaf_iid: The GOAD id that the new factory will implement.
 * @oaf_impl_ptr: Oaf shlib handle
 * Bonobo::ShlibFactory interface and which will be used to
 * construct this BonoboShlibFactory Gtk object.
 * @factory_cb: A callback which is used to create new GnomeShlib object instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * Initializes @c_factory with the command-line arguments and registers
 * the new factory in the name server.
 *
 * Returns: The initialized BonoboShlibFactory object.
 */
BonoboShlibFactory *
bonobo_shlib_factory_construct (BonoboShlibFactory    *factory,
				const char            *oaf_iid,
				PortableServer_POA     poa,
				gpointer               oaf_impl_ptr,
				GClosure              *closure)
{
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_SHLIB_FACTORY (factory), NULL);

	factory->priv->live_objects = 0;
	factory->priv->oaf_impl_ptr = oaf_impl_ptr;

        bonobo_activation_plugin_use (poa, oaf_impl_ptr);

	return BONOBO_SHLIB_FACTORY (
		bonobo_generic_factory_construct (
			BONOBO_GENERIC_FACTORY (factory), oaf_iid, closure));
}

/**
 * bonobo_shlib_factory_new_closure:
 * @oaf_iid: The GOAD id that this factory implements
 * @poa: the poa.
 * @oaf_impl_ptr: Oaf shlib handle
 * @factory_closure: A closure which is used to create new BonoboObject instances.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory_closure closure will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the Bonobo::Shlib interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A BonoboShlibFactory object that has an activated
 * Bonobo::ShlibFactory object that has registered with the GNOME
 * name server.
 */
BonoboShlibFactory *
bonobo_shlib_factory_new_closure (const char           *oaf_iid,
				  PortableServer_POA    poa,
				  gpointer              oaf_impl_ptr,
				  GClosure             *factory_closure)
{
	BonoboShlibFactory *factory;

	g_return_val_if_fail (oaf_iid != NULL, NULL);
	g_return_val_if_fail (factory_closure != NULL, NULL);
	
	factory = g_object_new (bonobo_shlib_factory_get_type (), NULL);
	
	return bonobo_shlib_factory_construct (
		factory, oaf_iid, poa, oaf_impl_ptr, factory_closure);
}

/**
 * bonobo_shlib_factory_new:
 * @oaf_iid: The GOAD id that this factory implements
 * @poa: the poa.
 * @oaf_impl_ptr: Oaf shlib handle
 * @factory_cb: A callback which is used to create new BonoboObject instances.
 * @user_data: The closure data to be passed to the @factory callback routine.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory function will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the Bonobo::Shlib interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A BonoboShlibFactory object that has an activated
 * Bonobo::ShlibFactory object that has registered with the GNOME
 * name server.
 */
BonoboShlibFactory *
bonobo_shlib_factory_new (const char           *component_id,
			  PortableServer_POA    poa,
			  gpointer              oaf_impl_ptr,
			  BonoboFactoryCallback factory_cb,
			  gpointer              user_data)
{
	return bonobo_shlib_factory_new_closure (
		component_id, poa, oaf_impl_ptr,
		g_cclosure_new (G_CALLBACK (factory_cb), user_data, NULL));
}

static void
bonobo_shlib_factory_finalize (GObject *object)
{
	BonoboShlibFactory *factory = BONOBO_SHLIB_FACTORY (object);

	/*
	 * We pray this happens only when we have released our
	 * last ref and no one is holding pointers into the soon
	 * to be unloaded shlib, particularly the stack.
	 *
	 * This is achieved by an idle unref handler.
	 */

	/* we dont unload it because of a problem with the GType system */
	/* oaf_plugin_unuse (c_factory->oaf_impl_ptr); */

	g_free (factory->priv);
	
	G_OBJECT_CLASS (bonobo_shlib_factory_parent_class)->finalize (object);
}

static BonoboObject *
bonobo_shlib_factory_new_generic (BonoboGenericFactory *factory,
				  const char           *oaf_iid)
{
	BonoboObject *retval;

	retval = BONOBO_GENERIC_FACTORY_CLASS (
		bonobo_shlib_factory_parent_class)->new_generic (factory, oaf_iid);

	return retval;
}

static void
bonobo_shlib_factory_class_init (BonoboGenericFactoryClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	bonobo_shlib_factory_parent_class = g_type_class_peek_parent (klass);

	klass->new_generic = bonobo_shlib_factory_new_generic;

	object_class->finalize = bonobo_shlib_factory_finalize;
}

static void
bonobo_shlib_factory_init (GObject *object)
{
	BonoboShlibFactory *factory = BONOBO_SHLIB_FACTORY (object);

	factory->priv = g_new0 (BonoboShlibFactoryPrivate, 1);
}


/**
 * bonobo_shlib_factory_get_type:
 *
 * Returns: The GType of the BonoboShlibFactory class.
 */
GType
bonobo_shlib_factory_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (BonoboShlibFactoryClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) bonobo_shlib_factory_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboShlibFactory),
			0, /* n_preallocs */
			(GInstanceInitFunc) bonobo_shlib_factory_init
		};

		type = g_type_register_static (
			bonobo_generic_factory_get_type (),
			"BonoboShlibFactory", &info, 0);
	}

	return type;
}

void
bonobo_shlib_factory_inc_live (BonoboShlibFactory *factory)
{
	g_return_if_fail (BONOBO_IS_SHLIB_FACTORY (factory));

	factory->priv->live_objects++;
}


static gboolean
bonobo_shlib_factory_dec_live_cb (BonoboShlibFactory *factory)
{
	factory->priv->live_objects--;

	if (factory->priv->live_objects <= 0)
		g_object_unref (G_OBJECT (factory));

	return FALSE;
}

void
bonobo_shlib_factory_dec_live (BonoboShlibFactory *factory)
{
	g_return_if_fail (BONOBO_IS_SHLIB_FACTORY (factory));

	g_idle_add ((GSourceFunc) bonobo_shlib_factory_dec_live_cb, factory);
}

static void
destroy_handler (GObject *object, BonoboShlibFactory *factory)
{
	bonobo_shlib_factory_dec_live (factory);
}

void
bonobo_shlib_factory_track_object (BonoboShlibFactory *factory,
				   BonoboObject       *object)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (BONOBO_IS_SHLIB_FACTORY (factory));

	bonobo_shlib_factory_inc_live (factory);

	g_signal_connect (G_OBJECT (object), "destroy",
			  G_CALLBACK (destroy_handler),
			  factory);
}
