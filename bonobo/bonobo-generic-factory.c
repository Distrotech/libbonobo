/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo GenericFactory object.
 *
 * The BonoboGenericFactory object is used to instantiate new
 * Bonobo::GenericFactory objects.  It acts as a wrapper for the
 * Bonobo::GenericFactory CORBA interface, and dispatches to
 * a user-specified factory function whenever its create_object()
 * method is invoked.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-generic-factory.h>
#include "bonobo-object-directory.h"

POA_GNOME_ObjectFactory__vepv bonobo_generic_factory_vepv;

static BonoboObjectClass *bonobo_generic_factory_parent_class;

static CORBA_boolean
impl_Bonobo_ObjectFactory_manufactures (PortableServer_Servant  servant,
					 const CORBA_char       *obj_goad_id,
					 CORBA_Environment      *ev)
{
	BonoboGenericFactory *factory = BONOBO_GENERIC_FACTORY (bonobo_object_from_servant (servant));

	if (! strcmp (obj_goad_id, factory->goad_id))
		return CORBA_TRUE;

	return CORBA_FALSE;
}

static CORBA_Object
impl_Bonobo_ObjectFactory_create_object (PortableServer_Servant   servant,
					  const CORBA_char        *obj_goad_id,
					  const GNOME_stringlist *params,
					  CORBA_Environment       *ev)
{
	BonoboGenericFactoryClass *class;
	BonoboGenericFactory      *factory;
	BonoboObject              *object;

	factory = BONOBO_GENERIC_FACTORY (bonobo_object_from_servant (servant));

	class = BONOBO_GENERIC_FACTORY_CLASS (GTK_OBJECT (factory)->klass);
	object = (*class->new_generic) (factory, obj_goad_id);

	if (!object)
		return CORBA_OBJECT_NIL;
	
	return CORBA_Object_duplicate (bonobo_object_corba_objref (BONOBO_OBJECT (object)), ev);
}

static CORBA_Object
create_bonobo_generic_factory (BonoboObject *object)
{
	POA_GNOME_ObjectFactory *servant;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	servant = (POA_GNOME_ObjectFactory *)g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_generic_factory_vepv;

	POA_GNOME_ObjectFactory__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_generic_factory_construct:
 * @goad_id: The GOAD id that the new factory will implement.
 * @c_factory: The object to be initialized.
 * @corba_factory: The CORBA object which supports the
 * Bonobo::GenericFactory interface and which will be used to
 * construct this BonoboGenericFactory Gtk object.
 * @factory: A callback which is used to create new GnomeGeneric object instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * Initializes @c_factory with the command-line arguments and registers
 * the new factory in the name server.
 *
 * Returns: The initialized BonoboGenericFactory object.
 */
BonoboGenericFactory *
bonobo_generic_factory_construct (const char             *goad_id,
				  BonoboGenericFactory   *c_factory,
				  CORBA_Object            corba_factory,
				  BonoboGenericFactoryFn  factory,
				  GnomeFactoryCallback    factory_cb,
				  void                   *data)
{
	CORBA_Environment ev;
	int ret;
	
	g_return_val_if_fail (c_factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (c_factory), NULL);
	g_return_val_if_fail (corba_factory != CORBA_OBJECT_NIL, NULL);

	bonobo_object_construct (BONOBO_OBJECT (c_factory), corba_factory);

	c_factory->factory         = factory;
	c_factory->factory_cb      = factory_cb;
	c_factory->factory_closure = data;
	c_factory->goad_id         = g_strdup (goad_id);

	CORBA_exception_init (&ev);

	ret = od_server_register (corba_factory, c_factory->goad_id);

	CORBA_exception_free (&ev);

	if (ret == OD_REG_ERROR) {
		bonobo_object_unref (BONOBO_OBJECT (c_factory));
		return NULL;
	}
	
	return c_factory;
}

/**
 * bonobo_generic_factory_new:
 * @goad_id: The GOAD id that this factory implements
 * @factory: A callback which is used to create new BonoboObject instances.
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
bonobo_generic_factory_new (const char             *goad_id,
			    BonoboGenericFactoryFn  factory,
			    void                   *data)
{
	BonoboGenericFactory *c_factory;
	GNOME_ObjectFactory corba_factory;

	g_return_val_if_fail (factory != NULL, NULL);
	
	c_factory = gtk_type_new (bonobo_generic_factory_get_type ());

	corba_factory = create_bonobo_generic_factory (BONOBO_OBJECT (c_factory));
	if (corba_factory == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (c_factory));
		return NULL;
	}
	
	return bonobo_generic_factory_construct (
		goad_id, c_factory, corba_factory, factory, NULL, data);
}

/**
 * bonobo_generic_factory_new:
 * @goad_id: The GOAD id that this factory implements
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
BonoboGenericFactory *bonobo_generic_factory_new_multi (
	const char *goad_id,
	GnomeFactoryCallback factory_cb,
	gpointer data)
{
	BonoboGenericFactory *c_factory;
	GNOME_ObjectFactory corba_factory;

	g_return_val_if_fail (factory_cb != NULL, NULL);
	g_return_val_if_fail (goad_id != NULL, NULL);
	
	c_factory = gtk_type_new (bonobo_generic_factory_get_type ());

	corba_factory = create_bonobo_generic_factory (BONOBO_OBJECT (c_factory));
	if (corba_factory == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (c_factory));
		return NULL;
	}
	
	return bonobo_generic_factory_construct (
		goad_id, c_factory, corba_factory, NULL, factory_cb, data);
}


static void
bonobo_generic_factory_finalize (GtkObject *object)
{
	BonoboGenericFactory *c_factory G_GNUC_UNUSED = BONOBO_GENERIC_FACTORY (object);
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	od_server_unregister (BONOBO_OBJECT(c_factory)->corba_objref,
			      c_factory->goad_id);
	CORBA_exception_free (&ev);
	g_free (c_factory->goad_id);
	
	GTK_OBJECT_CLASS (bonobo_generic_factory_parent_class)->destroy (object);
}

static BonoboObject *
bonobo_generic_factory_new_generic (BonoboGenericFactory *factory,
				    const char           *goad_id)
{
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_GENERIC_FACTORY (factory), NULL);

	if(factory->factory_cb)
		return (*factory->factory_cb)(factory, goad_id, factory->factory_closure);
	else
		return (*factory->factory)(factory, factory->factory_closure);
}

static void
init_generic_factory_corba_class (void)
{
	bonobo_generic_factory_vepv.GNOME_ObjectFactory_epv = bonobo_generic_factory_get_epv ();
}

static void
bonobo_generic_factory_class_init (BonoboGenericFactoryClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_generic_factory_parent_class = gtk_type_class (bonobo_object_get_type ());

	object_class->finalize = bonobo_generic_factory_finalize;

	klass->new_generic = bonobo_generic_factory_new_generic;

	init_generic_factory_corba_class ();
}

static void
bonobo_generic_factory_init (BonoboObject *object)
{
}

/**
 * bonobo_generic_factory_get_type:
 *
 * Returns: The GtkType of the BonoboGenericFactory class.
 */
GtkType
bonobo_generic_factory_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboGenericFactory",
			sizeof (BonoboGenericFactory),
			sizeof (BonoboGenericFactoryClass),
			(GtkClassInitFunc) bonobo_generic_factory_class_init,
			(GtkObjectInitFunc) bonobo_generic_factory_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_generic_factory_set:
 * @c_factory: The BonoboGenericFactory object whose callback will be set.
 * @factory: A callback routine which is used to create new object instances.
 * @data: The closure data to be passed to the @factory callback.
 *
 * Sets the callback and callback closure for @c_factory to
 * @factory and @data, respectively.
 */
void
bonobo_generic_factory_set (BonoboGenericFactory     *c_factory,
			      BonoboGenericFactoryFn  factory,
			      void                   *data)
{
	g_return_if_fail (c_factory != NULL);
	g_return_if_fail (BONOBO_IS_GENERIC_FACTORY (c_factory));
	g_return_if_fail (factory != NULL);

	c_factory->factory = factory;
	c_factory->factory_closure = data;
}


/**
 * bonobo_generic_factory_get_epv:
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
