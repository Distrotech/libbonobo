#ifndef _GNOME_STREAM_CLIENT_H_
#define _GNOME_STREAM_CLIENT_H_

#include <bonobo/bonobo.h>

CORBA_long gnome_stream_client_write        (const GNOME_Stream stream,
					     const void *buffer,
					     const size_t size,
					     CORBA_Environment *ev);

CORBA_long gnome_stream_client_write_string (const GNOME_Stream stream,
					     const char *str,
					     const gboolean terminate,
					     CORBA_Environment *ev);

CORBA_long gnome_stream_client_printf       (const GNOME_Stream stream,
					     const gboolean terminate,
					     CORBA_Environment *ev,
					     const char *fmt, ...);

CORBA_long gnome_stream_client_read_string  (const GNOME_Stream stream, char **str,
					     CORBA_Environment *ev);

#endif /* _GNOME_STREAM_CLIENT_H_ */
