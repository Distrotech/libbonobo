#ifndef _GNOME_OBJECT_IO_H_
#define _GNOME_OBJECT_IO_H_

typedef enum {
	GNOME_IO_OK,
	
	/* Generic error */
	GNOME_IOERR_GENERAL,

	/* PersistStorage interface not supported by object */
	GNOME_IOERR_PERSIST_NOT_SUPPORTED,
	
} GnomeIOStatus;

void            gnome_persist_stream_save_goad_id  (GNOME_Stream target,
						    const CORBA_char *goad_id,
						    CORBA_Environment *ev);
char           *gnome_persist_stream_load_goad_id  (GNOME_Stream source);
GnomeIOStatus   gnome_persiststream_save_to_stream (GNOME_PersistStream pstream,
						    GNOME_Stream target);
GnomeIOStatus   gnome_object_save_to_stream        (GnomeObject *object,
						    GNOME_Stream stream);
	
#endif /* _GNOME_OBJECT_IO_H_ */
