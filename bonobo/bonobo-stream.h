/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_STREAM_H_
#define _BONOBO_STREAM_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_STREAM_TYPE        (bonobo_stream_get_type ())
#define BONOBO_STREAM(o)          (GTK_CHECK_CAST ((o), BONOBO_STREAM_TYPE, BonoboStream))
#define BONOBO_STREAM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STREAM_TYPE, BonoboStreamClass))
#define BONOBO_IS_STREAM(o)       (GTK_CHECK_TYPE ((o), BONOBO_STREAM_TYPE))
#define BONOBO_IS_STREAM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STREAM_TYPE))

typedef struct {
        BonoboObject object;
} BonoboStream;

typedef struct {
	BonoboObjectClass parent_class;

	/*
	 * virtual methods
	 */
	CORBA_long    (*write)    (BonoboStream *stream,
				   const Bonobo_Stream_iobuf *buffer,
				   CORBA_Environment *ev);
	CORBA_long    (*read)     (BonoboStream *stream, CORBA_long count,
				   Bonobo_Stream_iobuf **buffer,
				   CORBA_Environment *ev);
        CORBA_long    (*seek)     (BonoboStream *stream,
				   CORBA_long offset, Bonobo_Stream_SeekType whence,
				   CORBA_Environment *ev);
        void          (*truncate) (BonoboStream *stream,
				   const CORBA_long new_size, 
				   CORBA_Environment *ev);
	void          (*copy_to)  (BonoboStream *stream,
				   const CORBA_char * dest,
				   const CORBA_long bytes,
				   CORBA_long *read,
				   CORBA_long *written,
				   CORBA_Environment *ev);
        void          (*commit)   (BonoboStream *stream,
				   CORBA_Environment *ev);
        void          (*close)    (BonoboStream *stream,
				   CORBA_Environment *ev);
        CORBA_boolean (*eos)      (BonoboStream *stream,
				   CORBA_Environment *ev);
	CORBA_long    (*length)   (BonoboStream *stream,
				   CORBA_Environment *ev);
} BonoboStreamClass;

POA_Bonobo_Stream__epv *bonobo_stream_get_epv (void);

extern POA_Bonobo_Stream__vepv bonobo_stream_vepv;

GtkType         bonobo_stream_get_type     (void);

END_GNOME_DECLS

#endif /* _BONOBO_STREAM_H_ */

