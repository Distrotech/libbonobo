/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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

static inline GnomeStream *
gnome_stream_from_servant (PortableServer_Servant servant)
{
	return GNOME_STREAM (gnome_object_from_servant (servant));
}

static CORBA_long
impl_read (PortableServer_Servant servant,
	   CORBA_long count,
	   GNOME_Stream_iobuf ** buffer,
	   CORBA_Environment * ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	return CLASS (stream)->read (stream, count, buffer, ev);
}

static CORBA_long
impl_write (PortableServer_Servant servant,
	    const GNOME_Stream_iobuf *buffer,
	    CORBA_Environment *ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	return CLASS (stream)->write (stream, buffer, ev);
}

static CORBA_long
impl_seek (PortableServer_Servant servant,
	   CORBA_long offset,
	   GNOME_Stream_SeekType whence,
	   CORBA_Environment *ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	return CLASS (stream)->seek (stream, offset, whence, ev);
}

static void
impl_truncate (PortableServer_Servant servant,
	       CORBA_long length,
	       CORBA_Environment *ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	CLASS (stream)->truncate (stream, length, ev);
}

static void
impl_copy_to (PortableServer_Servant servant,
	      const CORBA_char *dest,
	      CORBA_long bytes,
	      CORBA_long *read,
	      CORBA_long *written,
	      CORBA_Environment *ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	CLASS (stream)->copy_to (stream, dest, bytes, read, written, ev);
}

static void
impl_commit (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	CLASS (stream)->commit (stream, ev);
}

static void
impl_close (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	CLASS (stream)->close (stream, ev);
}

static CORBA_boolean
impl_eos (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	return CLASS (stream)->eos (stream, ev);
}

static CORBA_long
impl_length (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeStream *stream = gnome_stream_from_servant (servant);

	return CLASS (stream)->length (stream, ev);
}

/**
 * gnome_stream_get_epv:
 */
POA_GNOME_Stream__epv *
gnome_stream_get_epv (gboolean duplicate)
{
	POA_GNOME_Stream__epv *epv;
	static POA_GNOME_Stream__epv sepv = {
		NULL,
		impl_read,
		impl_write,
		impl_seek,
		impl_truncate,
		impl_copy_to,
		impl_commit,
		impl_close,
		impl_eos,
		impl_length
	};

	if(duplicate) {
		epv = g_new0 (POA_GNOME_Stream__epv, 1);
		memcpy(epv, &sepv, sizeof(sepv));
	} else
		epv = &sepv;

	return epv;
}

static void
init_stream_corba_class (void)
{
	/* The VEPV */
	gnome_stream_vepv.GNOME_Unknown_epv = gnome_object_get_epv (FALSE);
	gnome_stream_vepv.GNOME_Stream_epv = gnome_stream_get_epv (FALSE);
}

static void
gnome_stream_class_init (GnomeStreamClass *class)
{
	gnome_stream_parent_class = gtk_type_class (gnome_object_get_type ());

	init_stream_corba_class ();
}

/**
 * gnome_stream_get_type:
 *
 * Returns: the GtkType for a GnomeStream.
 */
GtkType
gnome_stream_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Stream:1.0",
			sizeof (GnomeStream),
			sizeof (GnomeStreamClass),
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


