#ifndef _GNOME_STORAGE_H_
#define _GNOME_STORAGE_H_

#include <bonobo/gnome-object.h>

struct _GnomeStoragePrivate;
typedef struct _GnomeStoragePrivate GnomeStoragePrivate;

BEGIN_GNOME_DECLS

typedef struct {
	GnomeObject *object;

	GnomeStoragePrivate storage;
} GnomeStorage;

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

END_GNOME_DECLS

#endif /* _GNOME_STORAGE_H_ */
