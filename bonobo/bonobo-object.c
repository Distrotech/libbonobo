/**
 * GNOME object
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include "gnome-object.h"

enum {
	TEST,
	LAST_SIGNAL
};

static guint gnome_object_signals [LAST_SIGNAL];
static GtkObjectClass *gnome_object_parent_class;

static GHashTable *mapping;

GnomeObject *
gnome_object_from_servant (POA_GNOME_object *servant)
{
	g_return_val_if_fail (servant != NULL, NULL);

	if (!mapping)
		return NULL;
	
	return g_hash_table_lookup (mapping, servant);
}

void
gnome_object_bind_to_servant (GnomeObject *object, void *servant)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (servant != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	if (!servant_to_object){
		servant_to_object = g_hash_table_new (g_direct_hash, g_direct_equal);
		object_to_servant = g_hash_table_new (g_direct_hash, g_direct_equal);
	}
	
	g_hash_table_insert (servant_to_object, servant, object);
	g_hash_table_insert (object_to_servant, object, servant);
}

void
gnome_object_drop_binding_by_servant (void *servant)
{
	void *object;
	g_return_if_fail (servant != NULL);

	object = g_hash_table_lookup (servant_to_object, servant);
	g_hash_table_remove (servant_to_object, servant);
	g_hash_table_remove (object_to_servant, object);
}

void
gnome_object_drop_binding (GnomeObject *object)
{
	void *servant;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	servant = g_hash_table_lookup (object_to_servant, object);
	g_hash_table_remove (servant_to_object, servant);
	g_hash_table_remove (object_to_servant, object);
}

static void
impl_GNOME_object__destroy (POA_GNOME_object *servant, CORBA_Environment *ev)
{
	POA_GNOME_object__fini((PortableServer_Servant) servant, ev);
	gnome_object_drop_binding_by_servant (servant);
	g_free (servant);
}

static void
impl_GNOME_object_ref (POA_GNOME_object *servant, CORBA_Environment *ev)
{
	GnomeObject *object;

	object = gnome_object_from_servant (servant);
	gtk_object_ref (GTK_OBJECT (object));
}

static void
impl_GNOME_object_unref (POA_GNOME_object *servant, CORBA_Environment *ev)
{
	GnomeObject *object;

	object = gnome_object_from_servant (servant);
	gtk_object_unref (GTK_OBJECT (object));
}

static CORBA_Object
impl_GNOME_object_query_interface (POA_GNOME_object *servant,
				   CORBA_char *repoid,
				   CORBA_Environment *ev)
{
	CORBA_Object retval;
	GnomeObject *object;

	object = gnome_object_from_servant (servant);

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	retval = object->object;
	return retval;
}

PortableServer_ServantBase__epv gnome_object_base_epv =
{
	NULL,			/* _private data */
	&impl_GNOME_object__destroy,	/* finalize routine */
	NULL,			/* default_POA routine */
};

POA_GNOME_object__epv gnome_object_epv =
{
	NULL,			/* _private */
	&impl_GNOME_object_ref,
	&impl_GNOME_object_unref,
	&impl_GNOME_object_query_interface,
};

POA_GNOME_object__vepv gnome_object_vepv = {
	&gnome_object_base_epv,
	&gnome_object_epv
};

static void
default_test (GnomeObject *object)
{
}

static void
gnome_object_destroy (GtkObject *object)
{
	CORBA_exception_free (GNOME_OBJECT (object)->ev);
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
gnome_object_instance_init (GnomeObject *object)
{
	CORBA_exception_init (&object->ev);
}

GtkType
gnome_object_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/object:1.0",
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

