/**
 * GNOME object
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */

enum {
	TEST,
	LAST_SIGNAL
};

static guint gnome_object_signals [LAST_SIGNAL];
static GtkObjectClass *gnome_object_parent_class;

static void
default_test (GnomeObject *object)
{
}

static void
unref_interface (gpointer key, gpointer value, gpointer user_data)
{
	gtk_object_unref (GTK_OBJECT (value));
}

static void
gnome_object_destroy (GnomeObject *object)
{
	g_hash_table_foreach (object->interfaces, unref_interface, object);
	
	g_hash_table_destroy (object->interfaces);
	
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
				GTK_SIGNAL_OFFSET(GnomeObject), test,
				gtk_marshall_NONE__NONE,
				GTK_TYPE_NONE, 0); 
	gtk_object_class_add_signals (object_class, gnome_object_signals, LAST_SIGNAL);

	object_class->destroy = gnome_object_destroy;
	
	class->test = default_test;
}

static void
gnome_object_init (GnomeObject *object)
{
	object->interfaces = g_hash_table_new (g_direct_hash, g_direct_equal);
}

GtkType
gnome_object_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomeObject",
			sizeof (GnomeObject),
			sizeof (GnomeObjectClass),
			(GtkClassInitFunc) gnome_object_class_init,
			(GtkObjectInitFunc) gnome_object_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

GnomeObject *
gnome_object_new (void)
{
	GnomeObject *object;

	object = gtk_type_new (gnome_object_get_type ());

	return object;
}

void
gnome_object_add_interface_1 (GnomeObject *object, GtkObject *interface)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (interface != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));
	g_return_if_fail (GTK_IS_OBJECT (interface));
	
	g_hash_table_insert (object->interfaces, interface->type, interface);
}

void
gnome_object_add_interface (GnomeObject *object, GnomeObject *interface)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (interface != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));
	g_return_if_fail (GNOME_IS_OBJECT (interface));

	gnome_object_add_interface_1 (object, GTK_OBJECT (interface));
	gnome_object_add_interface_1 (interface, GTK_OBJECT (object));
}

GtkObject *
gnome_object_query_interface (GnomeObject *object, GtkType type)
{
	GtkObject *object;
	
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), NULL);
	
	object = g_hash_table_lookup (object->interfaces, type);

	return object;
}
