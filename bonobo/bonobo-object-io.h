#ifndef _BONOBO_OBJECT_IO_H_
#define _BONOBO_OBJECT_IO_H_

typedef enum {
	GNOME_IO_OK,
	
	/* Generic error */
	GNOME_IOERR_GENERAL,

	/* PersistStorage interface not supported by object */
	GNOME_IOERR_PERSIST_NOT_SUPPORTED,
	
} GnomeIOStatus;

void            bonobo_persist_stream_save_goad_id  (Bonobo_Stream target,
						    const CORBA_char *goad_id,
						    CORBA_Environment *ev);
char           *bonobo_persist_stream_load_goad_id  (Bonobo_Stream source);
GnomeIOStatus   bonobo_persiststream_save_to_stream (Bonobo_PersistStream pstream,
						    Bonobo_Stream target,
						    const char *goad_id);
GnomeIOStatus   bonobo_object_save_to_stream        (BonoboObject *object,
						    Bonobo_Stream stream,
						    const char *goad_id);
	
#endif /* _BONOBO_OBJECT_IO_H_ */
