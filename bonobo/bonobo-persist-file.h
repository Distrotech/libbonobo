/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_PERSIST_FILE_H_
#define _GNOME_PERSIST_FILE_H_

#include <bonobo/gnome-persist.h>

BEGIN_GNOME_DECLS

#define GNOME_PERSIST_FILE_TYPE (gnome_persist_file_get_type ())
#define GNOME_PERSIST_FILE(o)   (GTK_CHECK_CAST ((o), GNOME_PERSIST_FILE_TYPE, GnomePersistFile))
#define GNOME_PERSIST_FILE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_PERSIST_FILE_TYPE, GnomePersistFileClass))
#define GNOME_IS_PERSIST_FILE(o)       (GTK_CHECK_TYPE ((o), GNOME_PERSIST_FILE_TYPE))
#define GNOME_IS_PERSIST_FILE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_PERSIST_FILE_TYPE))

typedef struct _GnomePersistFile GnomePersistFile;


typedef int (*GnomePersistFileIOFn)(GnomePersistFile *ps, const CORBA_char *filename, void *closure);

struct _GnomePersistFile {
	GnomePersist persist;

	gboolean     is_dirty;
	char *filename;

	/*
	 * For the sample routines, NULL if we use the ::save and ::load
	 * methods from the class
	 */
	GnomePersistFileIOFn  save_fn;
	GnomePersistFileIOFn  load_fn;
	void *closure;
};

typedef struct {
	GnomePersistClass parent_class;

	/*
	 * methods
	 */
	int        (*load)(GnomePersistFile *ps, const CORBA_char *filename);
	int        (*save)(GnomePersistFile *ps, const CORBA_char *filename);
	char*      (*get_current_file)(GnomePersistFile *ps);	
} GnomePersistFileClass;

GtkType             gnome_persist_file_get_type  (void);
void                gnome_persist_file_set_dirty (GnomePersistFile *ps,
						    gboolean dirty);

GnomePersistFile *gnome_persist_file_new       (const char *goad_id,
						GnomePersistFileIOFn load_fn,
						GnomePersistFileIOFn save_fn,
						void *closure);
GnomePersistFile *gnome_persist_file_construct (GnomePersistFile *ps,
						GNOME_PersistFile corba_ps,
						const char *goad_id,
						GnomePersistFileIOFn load_fn,
						GnomePersistFileIOFn save_fn,
						void *closure);

extern POA_GNOME_PersistFile__epv gnome_persist_file_epv;
END_GNOME_DECLS

#endif /* _GNOME_PERSIST_FILE_H_ */
