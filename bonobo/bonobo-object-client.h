#ifndef _GNOME_OBJECT_CLIENT_H_
#define _GNOME_OBJECT_CLIENT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-object.h>

#define GNOME_OBJECT_CLIENT_TYPE        (gnome_object_client_get_type ())
#define GNOME_OBJECT_CLIENT(o)          (GTK_CHECK_CAST ((o), GNOME_OBJECT_CLIENT_TYPE, GnomeObjectClient))
#define GNOME_OBJECT_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_OBJECT_CLIENT_TYPE, GnomeObjectClientClass))
#define GNOME_IS_OBJECT_CLIENT(o)       (GTK_CHECK_TYPE ((o), GNOME_OBJECT_CLIENT_TYPE))
#define GNOME_IS_OBJECT_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_OBJECT_CLIENT_TYPE))

typedef struct {
	GnomeObject parent;

	/*
	 * Some objects might use this, not always used.
	 */
	GList *view_frames;		/* List of GnomeViewFrames. */
} GnomeObjectClient;

typedef struct {
	GnomeObjectClass parent_class;
} GnomeObjectClientClass;

GtkType            gnome_object_client_get_type       (void);

GnomeObjectClient *gnome_object_client_construct      (GnomeObjectClient *object_client,
						       CORBA_Object corba_object);

GnomeObjectClient *gnome_object_activate_with_repo_id (GoadServerList *list,
						       const char *repo_id,
						       GoadActivationFlags flags,
						       const char **params);
GnomeObjectClient *gnome_object_activate_with_goad_id (GoadServerList *list,
						       const char *goad_id,
						       GoadActivationFlags flags,
						       const char **params);
#endif /* _GNOME_OBJECT_CLIENT_H_ */
