#ifndef _GNOME_MONIKER_H_
#define _GNOME_MONIKER_H_

#include <bonobo/gnome-bind-context.h>
#include <bonobo/gnome-persist-stream.h>

BEGIN_GNOME_DECLS

typedef struct {
	GnomePersistStream persist_stream;
} GnomeMoniker;

typedef struct {
	GnomePersistStreamClass parent_class;
} GnomeMonikerClass;

GnomeBindContext *gnome_bind_context_new (void);
GnomeMoniker     *gnome_parse_display_name (GnomeBindContext *bind,
					    const char *display_name,
					    int *chars_parsed);
					    
END_GNOME_DECLS

#endif
