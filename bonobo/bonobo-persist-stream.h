/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_PERSIST_STREAM_H_
#define _BONOBO_PERSIST_STREAM_H_

#include <bonobo/bonobo-persist.h>

BEGIN_GNOME_DECLS

#define BONOBO_PERSIST_STREAM_TYPE        (bonobo_persist_stream_get_type ())
#define BONOBO_PERSIST_STREAM(o)          (GTK_CHECK_CAST ((o), BONOBO_PERSIST_STREAM_TYPE, BonoboPersistStream))
#define BONOBO_PERSIST_STREAM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PERSIST_STREAM_TYPE, BonoboPersistStreamClass))
#define BONOBO_IS_PERSIST_STREAM(o)       (GTK_CHECK_TYPE ((o), BONOBO_PERSIST_STREAM_TYPE))
#define BONOBO_IS_PERSIST_STREAM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PERSIST_STREAM_TYPE))

typedef struct _BonoboPersistStream        BonoboPersistStream;
typedef struct _BonoboPersistStreamPrivate BonoboPersistStreamPrivate;


typedef int        (*BonoboPersistStreamIOFn)  (BonoboPersistStream *ps,
						const Bonobo_Stream  stream,
						void                *closure);
typedef CORBA_long (*BonoboPersistStreamMaxFn) (BonoboPersistStream *ps,
						void                *closure);

struct _BonoboPersistStream {
	BonoboPersist persist;

	gboolean     is_dirty;

	/*
	 * For the sample routines, NULL if we use the ::save and ::load
	 * methods from the class
	 */
	BonoboPersistStreamIOFn  save_fn;
	BonoboPersistStreamIOFn  load_fn;
	BonoboPersistStreamMaxFn get_size_max_fn;
	
	void *closure;

	BonoboPersistStreamPrivate *priv;
};

typedef struct {
	BonoboPersistClass parent_class;

	/*
	 * methods
	 */
	int        (*load)         (BonoboPersistStream *ps,
				    Bonobo_Stream        stream);
	int        (*save)         (BonoboPersistStream *ps,
				    Bonobo_Stream        stream);
	CORBA_long (*get_size_max) (BonoboPersistStream *ps);
} BonoboPersistStreamClass;

GtkType              bonobo_persist_stream_get_type  (void);
void                 bonobo_persist_stream_set_dirty (BonoboPersistStream *ps,
						      gboolean dirty);

BonoboPersistStream *bonobo_persist_stream_new       (BonoboPersistStreamIOFn load_fn,
						      BonoboPersistStreamIOFn save_fn,
						      void *closure);
BonoboPersistStream *bonobo_persist_stream_construct (BonoboPersistStream *ps,
						      Bonobo_PersistStream corba_ps,
						      BonoboPersistStreamIOFn load_fn,
						      BonoboPersistStreamIOFn save_fn,
						      void *closure);

POA_Bonobo_PersistStream__epv *bonobo_persist_stream_get_epv (void);

extern POA_Bonobo_PersistStream__vepv bonobo_persist_stream_vepv;
END_GNOME_DECLS

#endif /* _BONOBO_PERSIST_STREAM_H_ */
