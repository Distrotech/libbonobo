#ifndef _GNOME_COMPONENT_IO_H_
#define _GNOME_COMPONENT_IO_H_

#include <bonobo/gnome-component.h>

BEGIN_GNOME_DECLS

GnomeComponent *gnome_component_load             (GnomeStorage *storage,
						  const char *interface,
						  GnomeClientSite *client_site);

int             gnome_component_save             (GnomeComponent *component,
						  GnomeStorage   *storage,
						  gboolean       same_as_loaded);

GnomeComponent *gnome_component_load_from_stream (GnomeStream *stream,
						  const char *interface);

int             gnome_component_save_to_stream   (GnomeComponent *component,
						  GnomeStream    *stream);

char           *gnome_get_class_id_from_file     (const char *filename);

END_GNOME_DECLS

#endif /* _GNOME_COMPONENT_IO_H_ */
