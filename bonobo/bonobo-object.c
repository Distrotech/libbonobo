/**
 * GNOME object
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-object.h>
#include "bonobo.h"
#include "gtk-interfaces.h"

enum {
	TEST,
	LAST_SIGNAL
};

static guint gnome_object_signals [LAST_SIGNAL];
static GtkObjectClass *gnome_object_parent_class;

/* Assumptions made: sizeof(POA_interfacename) does not change between interfaces */

/**
 * gnome_object_from_servant:
 * @servant: Your servant.
 *
 * CORBA method implementations receive a parameter of type
 * PortableServer_Servant which is a pointer to the servant that was
 * used to create this specific CORBA object instance
 *
 * This routine allows the user to get the GnomeObject (ie, Gtk object
 * wrapper) from the servant.
 * 
 * Returns: the GnomeObject wrapper associated to this servant.
 */
GnomeObject *
gnome_object_from_servant (PortableServer_Servant servant)
{
	g_return_val_if_fail (servant != NULL, NULL);

	return GNOME_OBJECT(((GnomeObjectServant *)servant)->gnome_object);
}

/**
 * gnome_object_bind_to_servant:
 * @object: the GnomeObject to bind to the servant.
 * @servant: A PortableServer_Servant to bind to the GnomeObject
 *
 * This routine binds the @object to the @servant.  It establishes a
 * one to one mapping between the @object and the @servant.  Utility
 * routines are provides to go back and forth.
 *
 * This routine is used internally by the
 * gnome_object_activate_servant().
 */
void
gnome_object_bind_to_servant (GnomeObject *object, void *servant)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (servant != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	object->servant = servant;
	((GnomeObjectServant *)servant)->gnome_object = object;
}

static void
impl_GNOME_obj__destroy (PortableServer_Servant servant, CORBA_Environment *ev)
{
	POA_GNOME_obj__fini ((POA_GNOME_obj *)servant, ev);
	g_free (servant);
}

static void
impl_GNOME_obj_ref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeObject *object;

	object = gnome_object_from_servant (servant);
	gtk_object_ref (GTK_OBJECT (object));
	gtk_object_sink (GTK_OBJECT (object));
}

static void
impl_GNOME_obj_unref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeObject *object;

	object = gnome_object_from_servant (servant);
	gtk_object_unref (GTK_OBJECT (object));
}

static CORBA_Object
impl_GNOME_obj_query_interface (PortableServer_Servant servant,
				const CORBA_char *repoid,
				CORBA_Environment *ev)
{
	CORBA_Object retval;
	GnomeObject *object;
	GtkType type;
	GtkObject *interface;
	
	object = gnome_object_from_servant (servant);

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);

	type = gtk_type_from_name (repoid);

	/*
	 * Type has not been registered
	 */
	if (type == 0)
		return CORBA_OBJECT_NIL;

	interface = gtk_object_query_interface (GTK_OBJECT (object), type);
	if (!interface)
		return CORBA_OBJECT_NIL;
	
	if (GNOME_IS_OBJECT (interface)){
		GnomeObject *io = GNOME_OBJECT (interface);
		
		gtk_object_ref (interface);
		return CORBA_Object_duplicate (io->object, &io->ev);
	}
	return CORBA_OBJECT_NIL;
}

PortableServer_ServantBase__epv gnome_obj_base_epv =
{
	NULL,			/* _private data */
	&impl_GNOME_obj__destroy,	/* finalize routine */
	NULL,			/* default_POA routine */
};

POA_GNOME_obj__epv gnome_obj_epv =
{
	NULL,			/* _private */
	&impl_GNOME_obj_ref,
	&impl_GNOME_obj_unref,
	&impl_GNOME_obj_query_interface,
};

POA_GNOME_obj__vepv gnome_obj_vepv = {
	&gnome_obj_base_epv,
	&gnome_obj_epv
};

static void
default_test (GnomeObject *object)
{
}

static void
gnome_object_destroy (GtkObject *object)
{
	GnomeObject *gnome_object = GNOME_OBJECT (object);
	void *servant = gnome_object->servant;
	
	if (gnome_object->object != CORBA_OBJECT_NIL){
		PortableServer_POA_deactivate_object (
			bonobo_poa (), servant, &gnome_object->ev);
		CORBA_Object_release (gnome_object->object, &gnome_object->ev);
	}
	CORBA_exception_free (&gnome_object->ev);
	gnome_object_parent_class->destroy (object);
}

static void
gnome_object_class_init (GnomeObjectClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_object_parent_class = gtk_type_class (gtk_object_get_type ());

	gnome_object_signals [TEST] =
		gtk_signal_new ("test",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(GnomeObjectClass,test), 
				gtk_marshal_NONE__NONE,
				GTK_TYPE_NONE, 0); 
	gtk_object_class_add_signals (object_class, gnome_object_signals, LAST_SIGNAL);

	object_class->destroy = gnome_object_destroy;
	
	class->test = default_test;
}

static void
gnome_object_instance_init (GtkObject *gtk_object)
{
	GnomeObject *object = GNOME_OBJECT (gtk_object);
	
	CORBA_exception_init (&object->ev);
}

/**
 * gnome_object_get_type:
 *
 * Returns the GtkType associated for the base GnomeObject type
 */
GtkType
gnome_object_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/obj:1.0",
			sizeof (GnomeObject),
			sizeof (GnomeObjectClass),
			(GtkClassInitFunc) gnome_object_class_init,
			(GtkObjectInitFunc) gnome_object_instance_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

/** 
 * gnome_object_activate_servant:
 * @object: a GnomeObject
 * @servant: The servant to activate.
 *
 * This routine activates the @servant which is wrapped inside the
 * @object on the bonobo_poa (which is the default POA).
 *
 * Returns: The CORBA_Object that is wrapped by @object and whose
 * servant is @servant.  Might return CORBA_OBJECT_NIL on failure. 
 */
CORBA_Object
gnome_object_activate_servant (GnomeObject *object, void *servant)
{
	CORBA_Object o;

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), CORBA_OBJECT_NIL);
	g_return_val_if_fail (servant != NULL, CORBA_OBJECT_NIL);
	
	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &object->ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &object->ev);

	if (o){
		gnome_object_bind_to_servant (object, servant);
		return o;
	} else
		return CORBA_OBJECT_NIL;
	
}

/**
 * gnome_object_construct:
 * @object; The GTK object server wrapper for the CORBA service.
 * @corba_object: the reference to the real CORBA object.
 *
 * Constructs a GnomeObject. This method is usually invoked from the
 * construct method for other Gtk-based CORBA wrappers that derive
 * from the GNOME::obj interface
 *
 * This returns a constructed GnomeObject. 
 *
 */
GnomeObject *
gnome_object_construct (GnomeObject *object, CORBA_Object corba_object)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);

	object->object = CORBA_Object_duplicate (corba_object, &object->ev);

	return object;
}




