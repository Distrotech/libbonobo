#ifndef _GNOME_PERSIST_STREAM_H_
#define _GNOME_PERSIST_STREAM_H_

#include <bonobo/gnome-persist.h>

BEGIN_GNOME_DECLS

#define GNOME_PERSIST_STREAM_TYPE (gnome_persist_stream_get_type ())
#define GNOME_PERSIST_STREAM(o)   (GTK_CHECK_CAST ((o), GNOME_PERSIST_STREAM_TYPE, GnomePersistStream))
#define GNOME_PERSIST_STREAM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_PERSIST_STREAM_TYPE, GnomePersistStreamClass))
#define GNOME_IS_PERSIST_STREAM(o)       (GTK_CHECK_TYPE ((o), GNOME_PERSIST_STREAM_TYPE))
#define GNOME_IS_PERSIST_STREAM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_PERSIST_STREAM_TYPE))

typedef struct _GnomePersistStream GnomePersistStream;

typedef int (*GnomePersistStreamIOFn)(GnomePersistStream *ps, const GNOME_Stream stream, void *closure);

struct _GnomePersistStream {
	GnomePersist persist;

	gboolean     is_dirty;

	/*
	 * For the sample routines, NULL if we use the ::save and ::load
	 * methods from the class
	 */
	GnomePersistStreamIOFn save_fn;
	GnomePersistStreamIOFn load_fn;
	void *closure;
};

typedef struct {
	GnomePersistClass parent_class;

	/*
	 * methods
	 */
	int        (*load)(GnomePersistStream *ps, const GNOME_Stream stream);
	int        (*save)(GnomePersistStream *ps, const GNOME_Stream stream);
	CORBA_long (*get_size_max) (GnomePersistStream *ps);
} GnomePersistStreamClass;

GtkType             gnome_persist_stream_get_type  (void);
void                gnome_persist_stream_set_dirty (GnomePersistStream *ps,
						    gboolean dirty);

GnomePersistStream *gnome_persist_stream_new       (GnomePersistStreamIOFn load_fn,
						    GnomePersistStreamIOFn save_fn,
						    void *closure);
GnomePersistStream *gnome_persist_stream_construct (GnomePersistStream *ps,
						    GNOME_PersistStream corba_ps,
						    GnomePersistStreamIOFn load_fn,
						    GnomePersistStreamIOFn save_fn,
						    void *closure);

extern POA_GNOME_PersistStream__epv gnome_persist_stream_epv;
END_GNOME_DECLS

#endif /* _GNOME_PERSIST_STREAM_H_ */
