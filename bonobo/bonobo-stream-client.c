/*
 * gnome-stream-client.c: Helper routines to access a Bonobo_Stream CORBA object
 *
 * Authors:
 *   Nat Friedman    (nat@nat.org)
 *   Miguel de Icaza (miguel@kernel.org).
 *   Michael Meekss  (michael@helixcode.com)
 *
 * Copyright 1999,2000 Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>
#include <bonobo-stream-client.h>

/**
 * bonobo_stream_client_write:
 * @stream: A CORBA Object reference to a Bonobo_Stream
 * @buffer: the buffer to write
 * @size: number of bytes to write
 * @ev: a CORBA environment to return status information.  If this item
 * is NULL, no status information will be returned.
 *
 * This is a helper routine to write @size bytes from @buffer to the @stream
 *
 * Returns: The number of bytes written, or -1 if an error occurs.  In
 * the event of an error, @ev will be filled with the details of the
 * error, unless it is NULL, of course.
 */
CORBA_long
bonobo_stream_client_write (const Bonobo_Stream stream,
			    const void *buffer, const size_t size,
			    CORBA_Environment *ev)
{
	CORBA_long          v;
	Bonobo_Stream_iobuf *buf;
	CORBA_Environment  *local_ev;

	if (size == 0)
		return 0;

	g_return_val_if_fail (buffer != NULL, -1);
	g_return_val_if_fail (stream != CORBA_OBJECT_NIL, -1);

	if (!ev) {
		local_ev = g_new (CORBA_Environment, 1);
		CORBA_exception_init (local_ev);
	} else
		local_ev = ev;

	buf = Bonobo_Stream_iobuf__alloc ();

	if (!buf)
		goto stream_error;

	buf->_buffer = CORBA_sequence_CORBA_octet_allocbuf (size);
	if (buf->_buffer == NULL)
		goto stream_error_buffer;

	buf->_length = size;
	buf->_maximum = size;
	memcpy (buf->_buffer, buffer, size);

	v = Bonobo_Stream_write (stream, buf, ev);

	CORBA_free (buf);

	if (!ev) {
		CORBA_exception_free (local_ev);
		g_free (local_ev);
	}
	
	return v;

stream_error_buffer:
	CORBA_free (buf);

stream_error:
	CORBA_exception_set_system (local_ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);

	if (!ev) {
		CORBA_exception_free (local_ev);
		g_free (local_ev);
	}

	return -1;
}

/**
 * bonobo_stream_client_write_string:
 * @stream: A CORBA object reference to a #Bonobo_Stream.
 * @str: A string.
 * @terminate: Whether or not to write the \0 at the end of the
 * string.
 * @ev: A pointer to a #CORBA_Environment, optionally NULL.
 *
 * This is a helper routine to write the string in @str to @stream.
 * If @terminate is TRUE, a NULL character will be written out at the
 * end of the string.  This function will not return until the entire
 * string has been written out, unless an exception is raised.  See
 * also bonobo_stream_client_write().
 *
 * Returns: The number of bytes written, or -1 if an error
 * occurs.  In the event of an error, @ev will be filled with
 * the details of the error, unless it is NULL.
 */
CORBA_long
bonobo_stream_client_write_string (const Bonobo_Stream stream, const char *str,
				   gboolean terminate, CORBA_Environment *ev)
{
	size_t total_length;
	size_t bytes_written;

	if (str == NULL)
		return 0;

	g_return_val_if_fail (stream != CORBA_OBJECT_NIL, -1);

	total_length = strlen (str) + (terminate ? 1 : 0);
	bytes_written = 0;

	while (bytes_written < total_length) {

		bytes_written +=
			bonobo_stream_client_write (stream,
						    str + bytes_written,
						    total_length - bytes_written,
						    ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			g_warning ("BonoboStreamClient: Exception writing to stream!");
			return bytes_written;
		}
	}

	return bytes_written;
}

/**
 * bonobo_stream_client_printf:
 * @stream: A CORBA object reference to a #Bonobo_Stream.
 * @terminate: Whether or not to null-terminate the string when it is
 * written out to the stream.
 * @ev: A CORBA_Environment pointer, optionally NULL.
 * @fmt: The printf format string.
 *
 * Processes @fmt and the arguments which follow it to produce a
 * string.  Writes this string out to @stream.  This function will not
 * return until the entire string is written out, unless an exception
 * is raised.  See also bonobo_stream_client_write_string() and
 * bonobo_stream_client_write().
 */
CORBA_long
bonobo_stream_client_printf (const Bonobo_Stream stream, const gboolean terminate,
			     CORBA_Environment *ev, const char *fmt, ...)
{
	va_list      args;
	char        *str;
	CORBA_long   retval;

	g_return_val_if_fail (fmt != NULL, -1);

	va_start (args, fmt);
	str = g_strdup_vprintf (fmt, args);
	va_end (args);

	retval = bonobo_stream_client_write_string (stream, str, terminate, ev);

	g_free (str);

	return retval;
}

/**
 * bonobo_stream_client_read_string:
 * @stream: The #Bonobo_Stream from which the string will be read.
 * @str: The string pointer in which the string will be stored.
 * @ev: A pointer to a #CORBA_Environment.
 *
 * Reads a NULL-terminated string from @stream and stores it in a
 * newly-allocated string in @str.
 *
 * Returns: The number of bytes read, or -1 if an error occurs.
 * If an exception occurs, @ev will contain the exception.
 */
CORBA_long
bonobo_stream_client_read_string (const Bonobo_Stream stream, char **str,
				  CORBA_Environment *ev)
{
	Bonobo_Stream_iobuf *buffer;
	GString             *gstr;
	gboolean             all;

	gstr = g_string_sized_new (16);

	for (all = FALSE; !all; ) {
		CORBA_long len;

		/* 128 == a sensible size ? */
		len = Bonobo_Stream_read (stream, 128,
					  &buffer, ev);

		if (len == 0 || ev->_major != CORBA_NO_EXCEPTION)
			all = TRUE;

		else {
			int i;

			for (i = 0; i < len && i < buffer->_length; i++) {
				if (buffer->_buffer [i] == '\0') {
					all = TRUE;
					break;
				} else
					g_string_append_c (gstr, buffer->_buffer [i]);
			}
			CORBA_free (buffer);
		}
	}

	if (ev->_major != CORBA_NO_EXCEPTION) {
		*str = NULL;
		g_string_free (gstr, TRUE);
		return -1;
	} else {
		CORBA_long l;

		l    = gstr->len;
		*str = gstr->str;
		g_string_free (gstr, FALSE);

		return l;
	}
}
