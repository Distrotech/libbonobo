/**
 * GNOME Persist
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>

/* Parent GTK object class */
static GnomeObjectClass *gnome_persist_parent_class;

static void
init_persist_corba_class (void)
{
}

static void
gnome_persist_class_init (GnomePersistClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_persist_parent_class = gtk_type_class (gnome_object_get_type ());

	/*
	 * Override and initialize methods
	 */
	object_class->destroy = gnome_persist_destroy;

	init_persist_corba_class ();
}

static void
gnome_persist_init (GnomePersist *persist)
{
}

GnomePersist *
gnome_persist_construct (GnomePersist *persist)
{
	g_return_val_if_fail (persist != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PERSIST (persist), NULL);

	gnome_object_construct (GNOME_OBJECT (persist));
}

GtkType
gnome_persist_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Persist:1.0",
			sizeof (GnomePersist),
			sizeof (GnomePersistClass),
			(GtkClassInitFunc) gnome_persist_class_init,
			(GtkObjectInitFunc) gnome_persist_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}


