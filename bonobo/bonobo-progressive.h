#ifndef _GNOME_PROGRESSIVE_DATA_SINK_H_
#define _GNOME_PROGRESSIVE_DATA_SINK_H_

#include <bonobo/gnome-object.h>

BEGIN_GNOME_DECLS

#define GNOME_PROGRESSIVE_DATA_SINK_TYPE        (gnome_progressive_data_sink_get_type ())
#define GNOME_PROGRESSIVE_DATA_SINK(o)          (GTK_CHECK_CAST ((o), GNOME_PROGRESSIVE_DATA_SINK_TYPE, GnomeProgressiveDataSink))
#define GNOME_PROGRESSIVE_DATA_SINK_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_PROGRESSIVE_DATA_SINK_TYPE, GnomeProgressiveDataSinkClass))
#define GNOME_IS_PROGRESSIVE_DATA_SINK(o)       (GTK_CHECK_TYPE ((o), GNOME_PROGRESSIVE_DATA_SINK_TYPE))
#define GNOME_IS_PROGRESSIVE_DATA_SINK_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_PROGRESSIVE_DATA_SINK_TYPE))

typedef struct _GnomeProgressiveDataSink GnomeProgressiveDataSink;

/* Callback typedefs. */
typedef int (*GnomeProgressiveDataSinkStartFn)   (GnomeProgressiveDataSink *psink,
						  void *closure);

typedef int (*GnomeProgressiveDataSinkEndFn)     (GnomeProgressiveDataSink *psink, void *closure);

typedef int (*GnomeProgressiveDataSinkAddDataFn) (GnomeProgressiveDataSink *psink,
						  const GNOME_ProgressiveDataSink_iobuf *buffer,
						  void *closure);

typedef int (*GnomeProgressiveDataSinkSetSizeFn) (GnomeProgressiveDataSink *psink,
						  const CORBA_long count, void *closure);

struct _GnomeProgressiveDataSink {
	GnomeObject object;

	/*
	 * These are the callbacks the user can set.  If we use the
	 * default class methods, then these are NULL.
	 */
	GnomeProgressiveDataSinkStartFn start_fn;
	GnomeProgressiveDataSinkEndFn end_fn;
	GnomeProgressiveDataSinkAddDataFn add_data_fn;
	GnomeProgressiveDataSinkSetSizeFn set_size_fn;

	void *closure;
};

typedef struct {
	GnomeObjectClass parent_class;

	/*
	 * Methods.
	 */
	int (*start_fn)    (GnomeProgressiveDataSink *psink);
	int (*end_fn)      (GnomeProgressiveDataSink *psink);
	int (*add_data_fn) (GnomeProgressiveDataSink *psink,
			    const GNOME_ProgressiveDataSink_iobuf *buffer);
	int (*set_size_fn) (GnomeProgressiveDataSink *psink,
			    const CORBA_long count);
			 
} GnomeProgressiveDataSinkClass;


GtkType		gnome_progressive_data_sink_get_type  (void);

GnomeProgressiveDataSink *gnome_progressive_data_sink_new		(GnomeProgressiveDataSinkStartFn start_fn,
									 GnomeProgressiveDataSinkEndFn end_fn,
									 GnomeProgressiveDataSinkAddDataFn add_data_fn,
									 GnomeProgressiveDataSinkSetSizeFn set_size_fn,
									 void *closure);

GnomeProgressiveDataSink *gnome_progressive_data_sink_construct		(GnomeProgressiveDataSink *psink,
									 GNOME_ProgressiveDataSink corba_psink,
									 GnomeProgressiveDataSinkStartFn start_fn,
									 GnomeProgressiveDataSinkEndFn end_fn,
									 GnomeProgressiveDataSinkAddDataFn add_data_fn,
									 GnomeProgressiveDataSinkSetSizeFn set_size_fn,
									 void *closure);
							       

extern POA_GNOME_ProgressiveDataSink__epv gnome_progressive_data_sink_epv;
END_GNOME_DECLS

#endif /* _GNOME_PROGRESSIVE_DATA_SINK_H_ */

