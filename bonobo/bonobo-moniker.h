/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_MONIKER_H_
#define _GNOME_MONIKER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>

BEGIN_GNOME_DECLS

#define GNOME_MONIKER_TYPE        (gnome_moniker_get_type ())
#define GNOME_MONIKER(o)          (GTK_CHECK_CAST ((o), GNOME_MONIKER_TYPE, GnomeMoniker))
#define GNOME_MONIKER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_MONIKER_TYPE, GnomeMonikerClass))
#define GNOME_IS_MONIKER(o)       (GTK_CHECK_TYPE ((o), GNOME_MONIKER_TYPE))
#define GNOME_IS_MONIKER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_MONIKER_TYPE))

struct _GnomeMoniker;
typedef struct _GnomeMoniker GnomeMoniker;

struct _GnomeMoniker {
	char *goadid, *url;
	GList *items;
};

typedef struct {
	GtkObjectClass parent_class;
} GnomeMonikerClass;

GtkType           gnome_moniker_get_type         (void);
GnomeMoniker	 *gnome_moniker_new              (void);
void              gnome_moniker_set_server       (GnomeMoniker *moniker,
					          const char *goad_id,
					          const char *filename);
void              gnome_moniker_append_item_name (GnomeMoniker *moniker,
						  const char *item_name);
char             *gnome_moniker_get_as_string    (GnomeMoniker *moniker);


END_GNOME_DECLS

#endif
