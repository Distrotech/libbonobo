#ifndef _GNOME_BONOBO_OBJECT_IO_H_
#define _GNOME_BONOBO_OBJECT_IO_H_

#include <bonobo/gnome-component.h>
#include <bonobo/gnome-storage.h>

BEGIN_GNOME_DECLS

GnomeBonoboObject *gnome_bonobo_object_load             (GnomeStorage *storage,
							 const char *interface,
							 GnomeClientSite *client_site);

int                gnome_bonobo_object_save             (GnomeBonoboObject *bonobo_object,
							 GnomeStorage   *storage,
							 gboolean       same_as_loaded);

GnomeBonoboObject *gnome_bonobo_object_load_from_stream (GnomeStream *stream,
							 const char *interface);

int                gnome_bonobo_object_save_to_stream   (GnomeBonoboObject *bonobo_object,
							 GnomeStream    *stream);

char              *gnome_get_class_id_from_file         (const char *filename);

END_GNOME_DECLS

#endif /* _GNOME_BONOBO_OBJECT_IO_H_ */
