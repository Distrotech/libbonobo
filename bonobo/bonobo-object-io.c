/*
 * gnome-object-io.c: Helper routines for loading and saving of objects
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-object-io.h>
#include <bonobo/bonobo-object-client.h>
#include <bonobo/bonobo-stream-client.h>

/** 
 * bonobo_persist_stream_save_goad_id:
 * @target: A Bonobo_Stream object where the @goad_id will be written
 * @goad_id: the GOAD ID to write to the @target stream
 * @ev: Error values are returned here
 *
 * This routine saves the @goad_id in the @target stream.
 */
void
bonobo_persist_stream_save_goad_id (Bonobo_Stream target,
				   const CORBA_char *goad_id,
				   CORBA_Environment *ev)
{
	char *copy;
	int len, slen;
	
	g_return_if_fail (target != CORBA_OBJECT_NIL);
	g_return_if_fail (goad_id != NULL);

	slen = strlen (goad_id) + 1;
	len = sizeof (gint32) + slen;
	copy = g_malloc (len);
	((gint32 *) copy) = slen;
	strcpy (copy + sizeof (gint32), goad_id);
		
	bonobo_stream_client_write (target, copy, len, ev);

	if (ev->_major != CORBA_NO_EXCEPTION){
		CORBA_exception_free (ev);
		return;
	}
}

/**
 * bonobo_persist_stream_load_goad_id:
 * @source: Stream to load the GOAD ID from.
 *
 * Loads a GOAD ID from the @source Bonobo_Stream CORBA object reference.
 *
 * Returns: a pointer to the GOAD ID retrieved from the @source Bonobo_Stream
 * object, or %NULL if an error happens.
 */
char *
bonobo_persist_stream_load_goad_id (Bonobo_Stream source)
{
	CORBA_Environment ev;
	Bonobo_Stream_iobuf *buf;
	CORBA_long bytes, n;
	char *rval;
	
	g_return_val_if_fail (source != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (&ev);
	bytes = Bonobo_Stream_read (source, sizeof (gint32), &buf, &ev);
	if (ev._major != CORBA_NO_EXCEPTION || bytes != sizeof (gint32)){
		CORBA_exception_free (&ev);
		return NULL;
	}

	n = *((gint32 *) buf->_buffer);
	CORBA_free (buf);
	
	bytes = Bonobo_Stream_read (source, n, &buf, &ev);
	if (ev._major != CORBA_NO_EXCEPTION || bytes != n){
		CORBA_exception_free (&ev);
		return NULL;
	}
	
	/*
	 * Sanity check: the goad-id should be NULL terminated
	 */
	if (buf->_buffer [n-1] != 0){
		CORBA_free (buf);
		return NULL;
	}

	rval = g_strdup (buf->_buffer);
	CORBA_free (buf);
	CORBA_exception_free (&ev);

	return rval;
}

/**
 * bonobo_persiststream_save_to_stream:
 * @pstream: A Bonobo_PersistStream CORBA reference.
 * @stream: A Bonobo_Stream CORBA reference to save object on
 *
 * Queries the goad_id for the @pstream object, and saves this on  @object in the
 * @stream and then the object in @pstream is saved.
 *
 * Returns: The IO status for the operation.  Might return %GNOME_IOERR_PERSIST_NOT_SUPPORTED
 * if @object does not support the IDL:Bonobo/PersistStream:1.0 interface
 */
GnomeIOStatus
bonobo_persiststream_save_to_stream (Bonobo_PersistStream pstream, Bonobo_Stream target,
				    const char *goad_id)
{
	CORBA_Environment ev;
	
	g_return_val_if_fail (pstream != CORBA_OBJECT_NIL, GNOME_IOERR_GENERAL);
	g_return_val_if_fail (target != CORBA_OBJECT_NIL, GNOME_IOERR_GENERAL);
	
	CORBA_exception_init (&ev);

	bonobo_persist_stream_save_goad_id (target, goad_id, &ev);

	Bonobo_PersistStream_save (pstream, target, "", &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
		CORBA_exception_free (&ev);
		return GNOME_IOERR_GENERAL;
	}

	return GNOME_IO_OK;
}

/**
 * bonobo_object_save_to_stream:
 * @object: A BonoboObject
 * @stream: A Bonobo_Stream CORBA reference to save object on
 *
 * Saves the BonoboObject @object in the @stream.
 *
 * Returns: The IO status for the operation.  Might return %GNOME_IOERR_PERSIST_NOT_SUPPORTED
 * if @object does not support the IDL:Bonobo/PersistStream:1.0 interface
 */
GnomeIOStatus
bonobo_object_save_to_stream (BonoboObject *object, Bonobo_Stream stream,
			     const char *goad_id)
{
	Bonobo_PersistStream pstream;
	
	g_return_val_if_fail (object != NULL, GNOME_IOERR_GENERAL);
	g_return_val_if_fail (BONOBO_IS_OBJECT (object), GNOME_IOERR_GENERAL);
	g_return_val_if_fail (stream != CORBA_OBJECT_NIL, GNOME_IOERR_GENERAL);

	pstream = bonobo_object_query_interface (
		BONOBO_OBJECT (object), "IDL:Bonobo/PersistStream:1.0");
	
	if (pstream != CORBA_OBJECT_NIL)
		return GNOME_IOERR_PERSIST_NOT_SUPPORTED;

	return bonobo_persiststream_save_to_stream (pstream, stream, goad_id);
	
	return GNOME_IO_OK;
}

