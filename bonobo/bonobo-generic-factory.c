/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME GenericFactory object.
 *
 * The GnomeGenericFactory object is used to instantiate new
 * GnomeGeneric objects.  It acts as a wrapper for the
 * GNOME::GenericFactory CORBA interface, and dispatches to
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
#include <libgnorba/gnorba.h>
#include <bonobo/bonobo.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-generic-factory.h>

POA_GNOME_GenericFactory__vepv gnome_generic_factory_vepv;

static GnomeObjectClass *gnome_generic_factory_parent_class;

static CORBA_boolean
impl_GNOME_GenericFactory_supports (PortableServer_Servant servant,
				    CORBA_char *obj_goad_id,
				    CORBA_Environment *ev)
{
	g_message ("support invoked\n");
	return TRUE;
}

static CORBA_Object
impl_GNOME_GenericFactory_create_object (PortableServer_Servant servant,
					 CORBA_char *obj_goad_id,
					 GNOME_stringlist *params,
					 CORBA_Environment *ev)
{
	GnomeGenericFactoryClass *class;
	GnomeGenericFactory *factory;
	GnomeObject *object;

	factory = GNOME_GENERIC_FACTORY (gnome_object_from_servant (servant));

	class = GNOME_GENERIC_FACTORY_CLASS (GTK_OBJECT (factory)->klass);
	object = (*class->new_generic)(factory, obj_goad_id);

	if (!object)
		return CORBA_OBJECT_NIL;
	
	return CORBA_Object_duplicate(gnome_object_corba_objref (GNOME_OBJECT (object)), ev);
}

static CORBA_Object
create_gnome_generic_factory (GnomeObject *object)
{
	POA_GNOME_GenericFactory *servant;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	servant = (POA_GNOME_GenericFactory *)g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_generic_factory_vepv;

	POA_GNOME_GenericFactory__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return gnome_object_activate_servant (object, servant);
}

/**
 * gnome_generic_factory_construct:
 * @goad_id: The GOAD id that the new factory will implement.
 * @c_factory: The object to be initialized.
 * @corba_factory: The CORBA object which supports the
 * GNOME::GenericFactory interface and which will be used to
 * construct this GnomeGenericFactory Gtk object.
 * @factory: A callback which is used to create new GnomeGeneric object instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * Initializes @c_factory with the command-line arguments and registers
 * the new factory in the name server.
 *
 * Returns: The initialized GnomeGenericFactory object.
 */
GnomeGenericFactory *
gnome_generic_factory_construct (const char *goad_id,
				 GnomeGenericFactory *c_factory,
				 CORBA_Object         corba_factory,
				 GnomeGenericFactoryFn factory,
				 GnomeFactoryCallback factory_cb,
				 void *data)
{
	CORBA_Environment ev;
	int ret;
	
	g_return_val_if_fail (c_factory != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_GENERIC_FACTORY (c_factory), NULL);
	g_return_val_if_fail (corba_factory != CORBA_OBJECT_NIL, NULL);

	gnome_object_construct (GNOME_OBJECT (c_factory), corba_factory);

	c_factory->factory = factory;
	c_factory->factory_cb = factory_cb;
	c_factory->factory_closure = data;
	c_factory->goad_id = g_strdup (goad_id);

	CORBA_exception_init (&ev);

	ret = goad_server_register (
		NULL, corba_factory, c_factory->goad_id, "server",
		&ev);

	CORBA_exception_free (&ev);

	if (ret != 0){
		gnome_object_unref (GNOME_OBJECT (c_factory));
		return NULL;
	}
	
	return c_factory;
}

/**
 * gnome_generic_factory_new:
 * @goad_id: The GOAD id that this factory implements
 * @factory: A callback which is used to create new GnomeObject instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory function will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the GNOME::Generic interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A GnomeGenericFactory object that has an activated
 * GNOME::GenericFactory object that has registered with the GNOME
 * name server.
 */
GnomeGenericFactory *
gnome_generic_factory_new (const char *goad_id, GnomeGenericFactoryFn factory, void *data)
{
	GnomeGenericFactory *c_factory;
	GNOME_GenericFactory corba_factory;

	g_return_val_if_fail (factory != NULL, NULL);
	
	c_factory = gtk_type_new (gnome_generic_factory_get_type ());

	corba_factory = create_gnome_generic_factory (GNOME_OBJECT (c_factory));
	if (corba_factory == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (c_factory));
		return NULL;
	}
	
	return gnome_generic_factory_construct (
		goad_id, c_factory, corba_factory, factory, NULL, data);
}

/**
 * gnome_generic_factory_new:
 * @goad_id: The GOAD id that this factory implements
 * @factory_cb: A callback which is used to create new GnomeObject instances.
 * @data: The closure data to be passed to the @factory callback routine.
 *
 * This is a helper routine that simplifies the creation of factory
 * objects for GNOME objects.  The @factory function will be
 * invoked by the CORBA server when a request arrives to create a new
 * instance of an object supporting the GNOME::Generic interface.
 * The factory callback routine is passed the @data pointer to provide
 * the creation function with some state information.
 *
 * Returns: A GnomeGenericFactory object that has an activated
 * GNOME::GenericFactory object that has registered with the GNOME
 * name server.
 */
GnomeGenericFactory *gnome_generic_factory_new_multi (
	const char *goad_id,
	GnomeFactoryCallback factory_cb,
	gpointer data)
{
	GnomeGenericFactory *c_factory;
	GNOME_GenericFactory corba_factory;

	g_return_val_if_fail (factory_cb != NULL, NULL);
	g_return_val_if_fail (goad_id != NULL, NULL);
	
	c_factory = gtk_type_new (gnome_generic_factory_get_type ());

	corba_factory = create_gnome_generic_factory (GNOME_OBJECT (c_factory));
	if (corba_factory == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (c_factory));
		return NULL;
	}
	
	return gnome_generic_factory_construct (
		goad_id, c_factory, corba_factory, NULL, factory_cb, data);
}


static void
gnome_generic_factory_finalize (GtkObject *object)
{
	GnomeGenericFactory *c_factory G_GNUC_UNUSED = GNOME_GENERIC_FACTORY (object);
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	goad_server_unregister (NULL, c_factory->goad_id, "server", &ev);
	CORBA_exception_free (&ev);
	g_free (c_factory->goad_id);
	
	GTK_OBJECT_CLASS (gnome_generic_factory_parent_class)->destroy (object);
}

static GnomeObject *
gnome_generic_factory_new_generic (GnomeGenericFactory *factory, const char *goad_id)
{
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_GENERIC_FACTORY (factory), NULL);

	if(factory->factory_cb)
		return (*factory->factory_cb)(factory, goad_id, factory->factory_closure);
	else
		return (*factory->factory)(factory, factory->factory_closure);
}

static void
init_generic_factory_corba_class (void)
{
	gnome_generic_factory_vepv.GNOME_GenericFactory_epv = gnome_generic_factory_get_epv ();
}

static void
gnome_generic_factory_class_init (GnomeGenericFactoryClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_generic_factory_parent_class = gtk_type_class (gnome_object_get_type ());

	object_class->finalize = gnome_generic_factory_finalize;

	class->new_generic = gnome_generic_factory_new_generic;

	init_generic_factory_corba_class ();
}

static void
gnome_generic_factory_init (GnomeObject *object)
{
}

/**
 * gnome_generic_factory_get_type:
 *
 * Returns: The GtkType of the GnomeGenericFactory class.
 */
GtkType
gnome_generic_factory_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomeGenericFactory",
			sizeof (GnomeGenericFactory),
			sizeof (GnomeGenericFactoryClass),
			(GtkClassInitFunc) gnome_generic_factory_class_init,
			(GtkObjectInitFunc) gnome_generic_factory_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}

/**
 * gnome_generic_factory_set:
 * @c_factory: The GnomeGenericFactory object whose callback will be set.
 * @factory: A callback routine which is used to create new object instances.
 * @data: The closure data to be passed to the @factory callback.
 *
 * Sets the callback and callback closure for @c_factory to
 * @factory and @data, respectively.
 */
void
gnome_generic_factory_set (GnomeGenericFactory *c_factory,
			      GnomeGenericFactoryFn factory,
			      void *data)
{
	g_return_if_fail (c_factory != NULL);
	g_return_if_fail (GNOME_IS_GENERIC_FACTORY (c_factory));
	g_return_if_fail (factory != NULL);

	c_factory->factory = factory;
	c_factory->factory_closure = data;
}


/**
 * gnome_generic_factory_get_epv:
 */
POA_GNOME_GenericFactory__epv *
gnome_generic_factory_get_epv (void)
{
	POA_GNOME_GenericFactory__epv *epv;

	epv = g_new0 (POA_GNOME_GenericFactory__epv, 1);

	epv->supports	   = impl_GNOME_GenericFactory_supports;
	epv->create_object = impl_GNOME_GenericFactory_create_object;

	return epv;
}

