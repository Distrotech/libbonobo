/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-generic-factory.c: a GenericFactory object.
 *
 * The BonoboGenericFactory object is used to instantiate new
 * Bonobo::GenericFactory objects.  It acts as a wrapper for the
 * Bonobo::GenericFactory CORBA interface, and dispatches to
 * a user-specified factory function whenever its create_object()
 * method is invoked.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   ÉRDI Gergõ (cactus@cactus.rulez.org): cleanup
 *
 * Copyright 1999 Helix Code, Inc., 2001 Gergõ Érdi
 */
#include <config.h>
#include <string.h>
#include <gobject/gsignal.h>
#include <gobject/gmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-types.h>
#include <bonobo/bonobo-marshal.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-running-context.h>

#include <bonobo/bonobo-types.h>

struct _BonoboGenericFactoryPrivate {	
	/* The function factory */
	GClosure              *factory_closure;

	/* The component_id for this generic factory */
	char *oaf_iid;
};

static GObjectClass *bonobo_generic_factory_parent_class = NULL;

static CORBA_Object
impl_Bonobo_ObjectFactory_createObject (PortableServer_Servant   servant,
					const CORBA_char        *obj_oaf_iid,
					CORBA_Environment       *ev)
{
	BonoboGenericFactoryClass *class;
	BonoboGenericFactory      *factory;
	BonoboObject              *object;

	factory = BONOBO_GENERIC_FACTORY (bonobo_object (servant));

	class = BONOBO_GENERIC_FACTORY_CLASS (G_OBJECT_GET_CLASS (factory));
	object = (*class->new_generic) (factory, obj_oaf_iid);

	if (!object)
		return CORBA_OBJECT_NIL;
	
	return CORBA_Object_duplicate (BONOBO_OBJREF (object), ev);
}

/**
 * bonobo_generic_factory_construct:
 * @factory: The object to be initialized.
 * @corba_factory: The CORBA object which supports the
 * @oaf_iid: The GOAD id that the new factory will implement.
 * Bonobo::GenericFactory interface and which will be used to
 * construct this BonoboGenericFactory Gtk object.
 * @factory_closure: A Multi object factory closure.
 *
 * Initializes @c_factory with the command-line arguments and registers
 * the new factory in the name server.
 *
 * Returns: The initialized BonoboGenericFactory object.
 */
BonoboGenericFactory *
bonobo_generic_factory_construct (BonoboGenericFactory   *factory,
				  const char             *oaf_iid,
				  GClosure               *factory_closure)
{
	int ret;
	
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (factory), NULL);
	
	factory->priv->factory_closure =
		bonobo_closure_store (factory_closure, bonobo_marshal_OBJECT__STRING);
	factory->priv->oaf_iid    = g_strdup (oaf_iid);
	
	ret = bonobo_activation_active_server_register (oaf_iid, BONOBO_OBJREF (factory));

	if (ret != Bonobo_ACTIVATION_REG_SUCCESS) {
		bonobo_object_unref (BONOBO_OBJECT (factory));
		return NULL;
	}

	return factory;
}

/**
 * bonobo_generic_factory_new_closure:
 * @oaf_iid: The GOAD id that this factory implements
 * @factory_closure: A closure which is used to create new BonoboObject instances.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory_closure closure will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the Bonobo::Generic interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A BonoboGenericFactory object that has an activated
 * Bonobo::GenericFactory object that has registered with the GNOME
 * name server.
 */
BonoboGenericFactory *
bonobo_generic_factory_new_closure (const char *oaf_iid,
				    GClosure   *factory_closure)
{
	BonoboGenericFactory *factory;

	g_return_val_if_fail (oaf_iid != NULL, NULL);
	g_return_val_if_fail (factory_closure != NULL, NULL);
	
	factory = g_object_new (bonobo_generic_factory_get_type (), NULL);

	return bonobo_generic_factory_construct (
		factory, oaf_iid, factory_closure);
}


/**
 * bonobo_generic_factory_new:
 * @oaf_iid: The GOAD id that this factory implements
 * @factory_cb: A callback which is used to create new BonoboObject instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory function will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the Bonobo::Generic interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A BonoboGenericFactory object that has an activated
 * Bonobo::GenericFactory object that has registered with the GNOME
 * name server.
 */
BonoboGenericFactory *
bonobo_generic_factory_new (const char           *oaf_iid,
			    BonoboFactoryCallback factory_cb,
			    gpointer              user_data)
{
	return bonobo_generic_factory_new_closure (
		oaf_iid, g_cclosure_new (G_CALLBACK (factory_cb), user_data, NULL));
}

static void
bonobo_generic_factory_finalize (GObject *object)
{
	BonoboGenericFactory *factory G_GNUC_UNUSED = BONOBO_GENERIC_FACTORY (object);

	if (factory->priv) {
		bonobo_activation_active_server_unregister (
			factory->priv->oaf_iid, BONOBO_OBJREF (factory));

		g_free (factory->priv->oaf_iid);
		g_closure_unref (factory->priv->factory_closure);
		
		g_free (factory->priv);
		factory->priv = 0;
	}
		
	G_OBJECT_CLASS (bonobo_generic_factory_parent_class)->finalize (object);
}

static BonoboObject *
bonobo_generic_factory_new_generic (BonoboGenericFactory *factory,
				    const char           *oaf_iid)
{
	BonoboObject *ret;
	
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (factory), NULL);

	bonobo_closure_invoke (factory->priv->factory_closure,
			       BONOBO_OBJECT_TYPE, &ret,
			       BONOBO_GENERIC_FACTORY_TYPE, factory,
			       BONOBO_TYPE_STRING, oaf_iid, 0);

	return ret;
}

static void
bonobo_generic_factory_class_init (BonoboGenericFactoryClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	POA_Bonobo_GenericFactory__epv *epv = &klass->epv;

	epv->createObject = impl_Bonobo_ObjectFactory_createObject;

	object_class->finalize = bonobo_generic_factory_finalize;

	klass->new_generic = bonobo_generic_factory_new_generic;

	bonobo_generic_factory_parent_class = g_type_class_peek_parent (klass);
}

static void
bonobo_generic_factory_init (GObject *object)
{
	BonoboGenericFactory *factory = BONOBO_GENERIC_FACTORY (object);

	factory->priv = g_new0 (BonoboGenericFactoryPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (BonoboGenericFactory, 
		       Bonobo_GenericFactory,
		       BONOBO_OBJECT_TYPE,
		       bonobo_generic_factory);
