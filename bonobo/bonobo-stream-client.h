#ifndef _GNOME_STREAM_CLIENT_H_
#define _GNOME_STREAM_CLIENT_H_

CORBA_long gnome_stream_client_write (GNOME_Stream stream,
				      void *buffer,
				      size_t size,
				      CORBA_Environment *ev);

#endif /* _GNOME_STREAM_CLIENT_H_ */
