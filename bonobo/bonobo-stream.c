/*
 * gnome-stream: Stream manipulation, abstract class
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org).
 *
 */
#include <config.h>
#include <bonobo/gnome-stream.h>

static GnomeObjectClass *gnome_stream_parent_class;

POA_GNOME_Stream__vepv gnome_stream_vepv;

#define CLASS(o) GNOME_STREAM_CLASS(GTK_OBJECT(o)->klass)
static void
init_stream_corba_class (void)
{
	/* The EPV for the Storage */
	gnome_stream_epv.read = impl_read;
	gnome_stream_epv.write = impl_write;
	gnome_stream_epv.seek = impl_seek;
	gnome_stream_epv.truncate = impl_truncate;
	gnome_stream_epv.copy_to = impl_copy_to;
	gnome_stream_epv.commit = impl_commit;

	/* The VEPV */
	gnome_stream_vepv.GNOME_object_epv = &gnome_object_epv;
	gnome_stream_vepv.GNOME_Stream_epv = &gnome_stream_epv;
}

static void
gnome_stream_class_init (GnomeStreamClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_stream_parent_class = gtk_type_class (gnome_object_get_type ());
	object_class->destroy = gnome_stream_destroy;

	init_stream_corba_class ();
}

GtkType
gnome_storage_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Stream:1.0",
			sizeof (GnomeStorage),
			sizeof (GnomeStorageClass),
			(GtkClassInitFunc) gnome_stream_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}


