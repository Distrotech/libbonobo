/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_MONIKER_H_
#define _BONOBO_MONIKER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>

BEGIN_GNOME_DECLS

#define BONOBO_MONIKER_TYPE        (bonobo_moniker_get_type ())
#define BONOBO_MONIKER(o)          (GTK_CHECK_CAST ((o), BONOBO_MONIKER_TYPE, BonoboMoniker))
#define BONOBO_MONIKER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_MONIKER_TYPE, BonoboMonikerClass))
#define BONOBO_IS_MONIKER(o)       (GTK_CHECK_TYPE ((o), BONOBO_MONIKER_TYPE))
#define BONOBO_IS_MONIKER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_TYPE))

struct _BonoboMoniker;
typedef struct _BonoboMoniker BonoboMoniker;
struct _BonoboMonikerPrivate;
typedef struct _BonoboMonikerPrivate BonoboMonikerPrivate;

struct _BonoboMoniker {
	GtkObject parent;

	char *goadid, *url;
	GList *items;
	BonoboMonikerPrivate *priv;
};

typedef struct {
	GtkObjectClass parent_class;
} BonoboMonikerClass;

GtkType           bonobo_moniker_get_type         (void);
BonoboMoniker	 *bonobo_moniker_new              (void);
void              bonobo_moniker_set_server       (BonoboMoniker *moniker,
					          const char *goad_id,
					          const char *filename);
void              bonobo_moniker_append_item_name (BonoboMoniker *moniker,
						  const char *item_name);
char             *bonobo_moniker_get_as_string    (BonoboMoniker *moniker);


END_GNOME_DECLS

#endif
