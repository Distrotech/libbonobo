#ifndef _GNOME_STORAGE_H_
#define _GNOME_STORAGE_H_

#include <bonobo/gnome-object.h>
#include <bonobo/gnome-stream.h>

BEGIN_GNOME_DECLS

#define GNOME_STORAGE_TYPE        (gnome_storage_get_type ())
#define GNOME_STORAGE(o)          (GTK_CHECK_CAST ((o), GNOME_STORAGE_TYPE, GnomeStorage))
#define GNOME_STORAGE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_STORAGE_TYPE, GnomeStorageClass))
#define GNOME_IS_STORAGE(o)       (GTK_CHECK_TYPE ((o), GNOME_STORAGE_TYPE))
#define GNOME_IS_STORAGE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_STORAGE_TYPE))

typedef struct {
        GnomeObject object;
} GnomeStorage;

typedef struct {
	GnomeObjectClass parent_class;

	/*
	 * virtual methods
	 */
	GnomeStream  *(*create_stream)  (GnomeStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	GnomeStream  *(*open_stream)    (GnomeStorage *storage,
					 const CORBA_char *path,
					 GNOME_Storage_OpenMode, CORBA_Environment *ev);
	GnomeStorage *(*create_storage) (GnomeStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	GnomeStorage *(*open_storage)   (GnomeStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	void         (*copy_to)         (GnomeStorage *storage, GNOME_Storage target,
					 CORBA_Environment *ev);
	void         (*rename)          (GnomeStorage *storage,
					 const CORBA_char *path_name,
					 const CORBA_char *new_path_name,
					 CORBA_Environment *ev);
	void         (*commit)          (GnomeStorage *storage,
					 CORBA_Environment *ev);
	GNOME_Storage_directory_list *
	             (*list_contents)   (GnomeStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	void         (*destroy)         (GnomeStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
} GnomeStorageClass;

GtkType         gnome_storage_get_type     (void);
GnomeStorage   *gnome_storage_construct    (GnomeStorage *storage,
					    GNOME_Storage corba_storage);
GnomeStorage   *gnome_storage_file_open    (const char *path,
					    const char *open_mode);
GnomeStorage   *gnome_storage_storage_open (GnomeStorage *storage,
					    const char *path,
					    const char *open_mode);
GnomeStream    *gnome_stream_storage_open (GnomeStorage *storage,
					    const char *path,
					    const char *open_mode);

void gnome_storage_write_class_id (GnomeStorage *storage,
				   char *class_id);

void gnome_stream_write_class_id  (GnomeStream *stream,
				   char *class_id);

extern POA_GNOME_Storage__vepv gnome_storage_vepv;
extern POA_GNOME_Storage__epv gnome_storage_epv;

END_GNOME_DECLS

#endif /* _GNOME_STORAGE_H_ */

