#ifndef _GNOME_STREAM_H_
#define _GNOME_STREAM_H_

#include <bonobo/gnome-object.h>

BEGIN_GNOME_DECLS

#define GNOME_STREAM_TYPE        (gnome_stream_get_type ())
#define GNOME_STREAM(o)          (GTK_CHECK_CAST ((o), GNOME_STREAM_TYPE, GnomeStream))
#define GNOME_STREAM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_STREAM_TYPE, GnomeStreamClass))
#define GNOME_IS_STREAM(o)       (GTK_CHECK_TYPE ((o), GNOME_STREAM_TYPE))
#define GNOME_IS_STREAM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_STREAM_TYPE))

#ifndef _GNOME_STORAGE_H_
struct _GnomeStream;
struct _GnomeStorage;
typedef struct _GnomeStream GnomeStream;
typedef struct _GnomeStorage GnomeStorage;
#endif

struct _GnomeStream {
	GnomeObject object;
};

typedef struct {
	GnomeObjectClass parent_class;

	/*
	 * virtual methods
	 */
	CORBA_long (*write)    (GnomeStream *stream, long count,
			        const GNOME_Stream_iobuf *buffer,
			        CORBA_Environment *ev);
	CORBA_long (*read)     (GnomeStream *stream, long count,
				GNOME_Stream_iobuf **buffer,
			        CORBA_Environment *ev);
	void       (*seek)     (GnomeStream *stream,
			        CORBA_long offset, CORBA_long whence,
			        CORBA_Environment *ev);
	void       (*truncate) (GnomeStream *stream,
				const CORBA_long new_size, 
			        CORBA_Environment *ev);
	void       (*copy_to)  (GnomeStream *stream,
				const CORBA_char * dest,
				const CORBA_long bytes,
				CORBA_long *read,
				CORBA_long *written,
				CORBA_Environment *ev);
      void         (*commit)   (GnomeStream *stream,
				CORBA_Environment *ev);
} GnomeStreamClass;

extern POA_GNOME_Stream__vepv gnome_stream_vepv;
extern POA_GNOME_Stream__epv gnome_stream_epv;

GtkType         gnome_stream_get_type     (void);

END_GNOME_DECLS

#endif /* _GNOME_STREAM_H_ */

