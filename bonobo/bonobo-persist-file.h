/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_PERSIST_FILE_H_
#define _BONOBO_PERSIST_FILE_H_

#include <bonobo/bonobo-persist.h>

BEGIN_GNOME_DECLS

#define BONOBO_PERSIST_FILE_TYPE (bonobo_persist_file_get_type ())
#define BONOBO_PERSIST_FILE(o)   (GTK_CHECK_CAST ((o), BONOBO_PERSIST_FILE_TYPE, BonoboPersistFile))
#define BONOBO_PERSIST_FILE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PERSIST_FILE_TYPE, BonoboPersistFileClass))
#define BONOBO_IS_PERSIST_FILE(o)       (GTK_CHECK_TYPE ((o), BONOBO_PERSIST_FILE_TYPE))
#define BONOBO_IS_PERSIST_FILE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PERSIST_FILE_TYPE))

typedef struct _BonoboPersistFile BonoboPersistFile;
typedef struct _BonoboPersistFilePrivate BonoboPersistFilePrivate;


typedef int (*BonoboPersistFileIOFn)(BonoboPersistFile *pf, const CORBA_char *filename, void *closure);

struct _BonoboPersistFile {
	BonoboPersist persist;

	gboolean     is_dirty;
	char *filename;

	/*
	 * For the sample routines, NULL if we use the ::save and ::load
	 * methods from the class
	 */
	BonoboPersistFileIOFn  save_fn;
	BonoboPersistFileIOFn  load_fn;
	void *closure;

	BonoboPersistFilePrivate *priv;
};

typedef struct {
	BonoboPersistClass parent_class;

	/*
	 * methods
	 */
	int        (*load)(BonoboPersistFile *ps, const CORBA_char *filename);
	int        (*save)(BonoboPersistFile *ps, const CORBA_char *filename);
	char*      (*get_current_file)(BonoboPersistFile *ps);	
} BonoboPersistFileClass;

GtkType             bonobo_persist_file_get_type  (void);
void                bonobo_persist_file_set_dirty (BonoboPersistFile *ps,
						    gboolean dirty);

BonoboPersistFile *bonobo_persist_file_new       (BonoboPersistFileIOFn load_fn,
						BonoboPersistFileIOFn save_fn,
						void *closure);
BonoboPersistFile *bonobo_persist_file_construct (BonoboPersistFile *ps,
						Bonobo_PersistFile corba_ps,
						BonoboPersistFileIOFn load_fn,
						BonoboPersistFileIOFn save_fn,
						void *closure);

POA_Bonobo_PersistFile__epv *bonobo_persist_file_get_epv (void);

extern POA_Bonobo_PersistFile__vepv bonobo_persist_file_vepv;
END_GNOME_DECLS

#endif /* _BONOBO_PERSIST_FILE_H_ */
