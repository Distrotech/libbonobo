/*
 * gnome-stream-client.c: Helper routines to access a GNOME_Stream CORBA object
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org).
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo.h>
#include <bonobo/gnome-object.h>
#include <gnome-stream-client.h>

/**
 * gnome_stream_client_write:
 * @stream: A CORBA Object reference to a GNOME_Stream
 * @buffer: the buffer to write
 * @size: number of bytes to write
 * @ev: a CORBA environment to return status information
 *
 * This is a helper routine to write @size bytes from @buffer to the @stream
 *
 * Returns: The number of bytes written.  Only if ev->_major is CORBA_NO_EXCEPTION.
 */
CORBA_long
gnome_stream_client_write (GNOME_Stream stream, void *buffer, size_t size, CORBA_Environment *ev)
{
	CORBA_long v;
	GNOME_Stream_iobuf *buf;
	
	buf = GNOME_Stream_iobuf__alloc ();
	if (!buf){
		CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return -1;
	}

	buf->_buffer = CORBA_sequence_CORBA_octet_allocbuf (size);
	if (buf->_buffer == NULL){
		CORBA_free (buf);
		CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return -1;
	}

	buf->_length = size;
	buf->_maximum = size;
	memcpy (buf->_buffer, buffer, size);

	v = GNOME_Stream_write (stream, buf, ev);

	CORBA_free (buf);

	return v;
}
