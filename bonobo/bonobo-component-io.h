#ifndef _GNOME_EMBEDDABLE_IO_H_
#define _GNOME_EMBEDDABLE_IO_H_

#include <bonobo/gnome-embeddable.h>
#include <bonobo/gnome-storage.h>

BEGIN_GNOME_DECLS

GnomeEmbeddable *gnome_embeddable_load             (GnomeStorage *storage,
							 const char *interface,
							 GnomeClientSite *client_site);

int                gnome_embeddable_save             (GnomeEmbeddable *bonobo_object,
							 GnomeStorage   *storage,
							 gboolean       same_as_loaded);

GnomeEmbeddable *gnome_embeddable_load_from_stream (GnomeStream *stream,
							 const char *interface);

int                gnome_embeddable_save_to_stream   (GnomeEmbeddable *bonobo_object,
							 GnomeStream    *stream);

char              *gnome_get_class_id_from_file         (const char *filename);

END_GNOME_DECLS

#endif /* _GNOME_EMBEDDABLE_IO_H_ */
