#ifndef _GNOME_OBJECT_H_
#define _GNOME_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>

BEGIN_GNOME_DECLS

#define GNOME_OBJECT_TYPE        (gnome_object_get_type ())
#define GNOME_OBJECT_ITEM(o)     (GTK_CHECK_CAST ((o), GNOME_OBJECT_TYPE, GnomeObject))
#define GNOME_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_OBJECT_TYPE, GnomeObjectClass))
#define GNOME_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), GNOME_OBJECT_TYPE))
#define GNOME_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_OBJECT_TYPE))

typedef struct {
	GtkObject object;

	GHashTable *interfaces;
} GnomeObject;

typedef struct {
	GtkObjectClass parent_class;
	void  (*test)(GnomeObject *object);
	
} GnomeObjectClass;

GtkType      gnome_object_get_type         (void);
GnomeObject *gnome_object_new              (void);

void         gnome_object_add_interface_1  (GnomeObject *object,
					    GtkObject   *interface);

void         gnome_object_add_interface    (GnomeObject *object,
					    GnomeObject *interface);

GtkObject   *gnome_object_query_interface  (GnomeObject *object,
					    GtkType type);
#endif
