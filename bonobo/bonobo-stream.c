/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-stream.c: Stream manipulation, abstract class
 *
 * Author:
 *     Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-stream.h>

static BonoboObjectClass *bonobo_stream_parent_class;

POA_Bonobo_Stream__vepv bonobo_stream_vepv;

#define CLASS(o) BONOBO_STREAM_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboStream *
bonobo_stream_from_servant (PortableServer_Servant servant)
{
	return BONOBO_STREAM (bonobo_object_from_servant (servant));
}

static void
impl_read (PortableServer_Servant servant,
	   CORBA_long             count,
	   Bonobo_Stream_iobuf  **buffer,
	   CORBA_Environment     *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->read (stream, count, buffer, ev);
}

static CORBA_long
impl_write (PortableServer_Servant servant,
	    const Bonobo_Stream_iobuf *buffer,
	    CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	return CLASS (stream)->write (stream, buffer, ev);
}

static CORBA_long
impl_seek (PortableServer_Servant servant,
	   CORBA_long offset,
	   Bonobo_Stream_SeekType whence,
	   CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	return CLASS (stream)->seek (stream, offset, whence, ev);
}

static void
impl_truncate (PortableServer_Servant servant,
	       CORBA_long length,
	       CORBA_Environment *ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

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
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->copy_to (stream, dest, bytes, read, written, ev);
}

static void
impl_commit (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->commit (stream, ev);
}

static void
impl_close (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	CLASS (stream)->close (stream, ev);
}

static CORBA_boolean
impl_eos (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	return CLASS (stream)->eos (stream, ev);
}

static CORBA_long
impl_length (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboStream *stream = bonobo_stream_from_servant (servant);

	return CLASS (stream)->length (stream, ev);
}

/**
 * bonobo_stream_get_epv:
 */
POA_Bonobo_Stream__epv *
bonobo_stream_get_epv (void)
{
	POA_Bonobo_Stream__epv *epv;

	epv = g_new0 (POA_Bonobo_Stream__epv, 1);

	epv->read	= impl_read;
	epv->write	= impl_write;
	epv->seek	= impl_seek;
	epv->truncate	= impl_truncate;
	epv->copy_to	= impl_copy_to;
	epv->commit	= impl_commit;
	epv->close	= impl_close;
	epv->eos	= impl_eos;
	epv->length	= impl_length;

	return epv;
}

static void
init_stream_corba_class (void)
{
	/* The VEPV */
	bonobo_stream_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_stream_vepv.Bonobo_Stream_epv = bonobo_stream_get_epv ();
}

static void
bonobo_stream_class_init (BonoboStreamClass *klass)
{
	bonobo_stream_parent_class = gtk_type_class (bonobo_object_get_type ());

	init_stream_corba_class ();
}

/**
 * bonobo_stream_get_type:
 *
 * Returns: the GtkType for a BonoboStream.
 */
GtkType
bonobo_stream_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStream",
			sizeof (BonoboStream),
			sizeof (BonoboStreamClass),
			(GtkClassInitFunc) bonobo_stream_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_stream_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Stream
bonobo_stream_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Stream *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Stream *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_stream_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Stream__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}
