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
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-running-context.h>
#include <liboaf/liboaf.h>

/* FIXME: cut and paste nastiness from old bonobo */

typedef struct {
	POA_GNOME_ObjectFactory  servant_placeholder;

	BonoboGenericFactory    *c_factory;
} GenericServant;

struct _BonoboGenericFactoryPrivate
{	
	/* The function factory */
	BonoboFactoryCallback  factory_cb;
	gpointer               factory_closure;

	/* The CORBA Object */
	GNOME_ObjectFactory    corba_objref;

	/* The component_id for this generic factory */
	char *oaf_iid;
};

static GNOME_ObjectFactory
activate_servant (void *servant, gpointer shlib_id)
{
	CORBA_Environment ev;
	GNOME_ObjectFactory o;

	g_return_val_if_fail (servant != NULL, CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	CORBA_free (PortableServer_POA_activate_object (
                bonobo_poa (), servant, &ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &ev);

	CORBA_exception_free (&ev);
	
	return o;
}

PortableServer_ServantBase__epv bonobo_generic_base_epv = { NULL };
POA_GNOME_ObjectFactory__vepv bonobo_generic_factory_vepv;

static GObjectClass *bonobo_generic_factory_parent_class = NULL;

static CORBA_boolean
impl_Bonobo_ObjectFactory_manufactures (PortableServer_Servant  servant,
					const CORBA_char       *obj_oaf_iid,
					CORBA_Environment      *ev)
{
	BonoboGenericFactory *factory = BONOBO_GENERIC_FACTORY (
		((GenericServant *) servant)->c_factory);

	if (! strcmp (obj_oaf_iid, factory->priv->oaf_iid))
		return CORBA_TRUE;

	return CORBA_FALSE;
}

static CORBA_Object
impl_Bonobo_ObjectFactory_create_object (PortableServer_Servant   servant,
					 const CORBA_char        *obj_oaf_iid,
					 const GNOME_stringlist *params,
					 CORBA_Environment       *ev)
{
	BonoboGenericFactoryClass *class;
	BonoboGenericFactory      *factory;
	BonoboObject              *object;

	factory = BONOBO_GENERIC_FACTORY (((GenericServant *) servant)->c_factory);

	class = BONOBO_GENERIC_FACTORY_CLASS (G_OBJECT_GET_CLASS (factory));
	object = (*class->new_generic) (factory, obj_oaf_iid);

	if (!object)
		return CORBA_OBJECT_NIL;
	
	return CORBA_Object_duplicate (bonobo_object_corba_objref (BONOBO_OBJECT (object)), ev);
}

GNOME_ObjectFactory
bonobo_generic_factory_corba_objref (BonoboGenericFactory *factory)
{
	g_return_val_if_fail (factory != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (factory), CORBA_OBJECT_NIL);

	return factory->priv->corba_objref;
}

GNOME_ObjectFactory
bonobo_generic_factory_corba_object_create (BonoboGenericFactory *object, 
					    gpointer              shlib_id)
{
	POA_GNOME_ObjectFactory *servant;
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);

	servant = (POA_GNOME_ObjectFactory *) g_new0 (GenericServant, 1);
	((GenericServant *) servant)->c_factory = g_object_ref (G_OBJECT (object));
	servant->vepv = &bonobo_generic_factory_vepv;

	POA_GNOME_ObjectFactory__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)) {
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	
	return activate_servant (servant, shlib_id);
}

/**
 * bonobo_generic_factory_construct:
 * @factory: The object to be initialized.
 * @corba_factory: The CORBA object which supports the
 * @oaf_iid: The GOAD id that the new factory will implement.
 * Bonobo::GenericFactory interface and which will be used to
 * construct this BonoboGenericFactory Gtk object.
 * @factory_cb: A Multi object factory callback.
 * @user_data: The closure data to be passed to the @factory callback routine.
 *
 * Initializes @c_factory with the command-line arguments and registers
 * the new factory in the name server.
 *
 * Returns: The initialized BonoboGenericFactory object.
 */
BonoboGenericFactory *
bonobo_generic_factory_construct (BonoboGenericFactory   *factory,
				  GNOME_ObjectFactory     corba_factory,
				  const char             *oaf_iid,
				  BonoboFactoryCallback   factory_cb,
				  gpointer                user_data)
{
	CORBA_Environment ev;
	int ret;
	
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (factory), NULL);
	g_return_val_if_fail (corba_factory != CORBA_OBJECT_NIL, NULL);

	factory->priv->factory_cb      = factory_cb;
	factory->priv->factory_closure = user_data;
	factory->priv->oaf_iid         = g_strdup (oaf_iid);

	CORBA_exception_init (&ev);
	factory->priv->corba_objref  = CORBA_Object_duplicate (corba_factory, &ev);
	CORBA_exception_free (&ev);
	
	ret = oaf_active_server_register (oaf_iid, corba_factory);

	if (ret == OAF_REG_ERROR) {
		CORBA_Environment ev;
		CORBA_exception_init (&ev);
		g_object_unref (G_OBJECT (factory));
		CORBA_Object_release (corba_factory, &ev);
		CORBA_exception_free (&ev);
		return NULL;
	}

	return factory;
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
bonobo_generic_factory_new (const char            *oaf_iid,
			    BonoboFactoryCallback  factory_cb,
			    gpointer               data)
{
	BonoboGenericFactory *factory;
	GNOME_ObjectFactory corba_factory;

	g_return_val_if_fail (factory_cb != NULL, NULL);
	g_return_val_if_fail (oaf_iid != NULL, NULL);
	
	factory = g_object_new (bonobo_generic_factory_get_type (), NULL);
	
	corba_factory = bonobo_generic_factory_corba_object_create (factory, factory_cb);
	if (corba_factory == CORBA_OBJECT_NIL) {
		g_object_unref (G_OBJECT (factory));
		return NULL;
	}
	
	return bonobo_generic_factory_construct (factory, corba_factory,
						 oaf_iid, factory_cb, data);
}


static void
bonobo_generic_factory_finalize (GObject *object)
{
	BonoboGenericFactory *factory G_GNUC_UNUSED = BONOBO_GENERIC_FACTORY (object);
	CORBA_Environment ev;

	if (factory->priv) {
		CORBA_exception_init (&ev);
		oaf_active_server_unregister (factory->priv->oaf_iid,
					      factory->priv->corba_objref);
		CORBA_Object_release (factory->priv->corba_objref, &ev);
		CORBA_exception_free (&ev);
		g_free (factory->priv->oaf_iid);

		g_free (factory->priv);
		factory->priv = 0;
	}
		
	G_OBJECT_CLASS (bonobo_generic_factory_parent_class)->finalize (object);
}

static BonoboObject *
bonobo_generic_factory_new_generic (BonoboGenericFactory *factory,
				    const char           *oaf_iid)
{
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (factory), NULL);

	return factory->priv->factory_cb (factory, oaf_iid,
					  factory->priv->factory_closure);
}

static void
bonobo_generic_factory_class_init (BonoboGenericFactoryClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	bonobo_generic_factory_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = bonobo_generic_factory_finalize;

	klass->new_generic = bonobo_generic_factory_new_generic;

	bonobo_generic_factory_vepv._base_epv = &bonobo_generic_base_epv;
	bonobo_generic_factory_vepv.GNOME_ObjectFactory_epv = bonobo_generic_factory_get_epv ();
}

static void
bonobo_generic_factory_init (GObject *object)
{
	BonoboGenericFactory *factory = BONOBO_GENERIC_FACTORY (object);

	factory->priv = g_new0 (BonoboGenericFactoryPrivate, 1);
}

/**
 * bonobo_generic_factory_get_type:
 *
 * Returns: The GType of the BonoboGenericFactory class.
 */
GType
bonobo_generic_factory_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (BonoboGenericFactoryClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) bonobo_generic_factory_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboGenericFactory),
			0, /* n_preallocs */
			(GInstanceInitFunc) bonobo_generic_factory_init
		};

		type = g_type_register_static (
			G_TYPE_OBJECT, "BonoboGenericFactory", &info, 0);
	}

	return type;
}

#if 0
/**
 * bonobo_generic_factory_set:
 * @factory: The BonoboGenericFactory object whose callback will be set.
 * @factory_cb: A callback routine which is used to create new object instances.
 * @user_data: The closure data to be passed to the @factory callback.
 *
 * Sets the callback and callback closure for @factory to
 * @factory_cb and @user_data, respectively.
 */
void
bonobo_generic_factory_set (BonoboGenericFactory  *factory,
			    BonoboFactoryCallback  factory_cb,
			    gpointer               user_data)
{
	g_return_if_fail (factory != NULL);
	g_return_if_fail (BONOBO_IS_GENERIC_FACTORY (factory));
	g_return_if_fail (factory_cb != NULL);

	factory->priv->factory_cb = factory;
	factory->priv->factory_closure = user_data;
}
#endif

/**
 * bonobo_generic_factory_get_epv:
 *
 * Returns: The EPV for the default BonoboGenericFactory implementation.  
 */
POA_GNOME_ObjectFactory__epv *
bonobo_generic_factory_get_epv (void)
{
	POA_GNOME_ObjectFactory__epv *epv;

	epv = g_new0 (POA_GNOME_ObjectFactory__epv, 1);

	epv->manufactures  = impl_Bonobo_ObjectFactory_manufactures;
	epv->create_object = impl_Bonobo_ObjectFactory_create_object;

	return epv;
}

