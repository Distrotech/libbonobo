#ifndef _GNOME_UNKNOWN_CLIENT_H_
#define _GNOME_UNKNOWN_CLIENT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-object.h>

#define GNOME_UNKNOWN_CLIENT_TYPE        (gnome_unknown_client_get_type ())
#define GNOME_UNKNOWN_CLIENT(o)          (GTK_CHECK_CAST ((o), GNOME_UNKNOWN_CLIENT_TYPE, GnomeUnknownClient))
#define GNOME_UNKNOWN_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_UNKNOWN_CLIENT_TYPE, GnomeUnknownClientClass))
#define GNOME_IS_UNKNOWN_CLIENT(o)       (GTK_CHECK_TYPE ((o), GNOME_UNKNOWN_CLIENT_TYPE))
#define GNOME_IS_UNKNOWN_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_UNKNOWN_CLIENT_TYPE))

typedef struct {
	GnomeUnknown parent;

	/*
	 * Some objects might use this, not always used.
	 */
	GList *view_frames;		/* List of GnomeViewFrames. */
} GnomeUnknownClient;

typedef struct {
	GnomeUnknownClass parent_class;
} GnomeUnknownClientClass;

GtkType            gnome_unknown_client_get_type       (void);

GnomeUnknownClient *gnome_unknown_client_construct      (GnomeUnknownClient *object_client,
						       CORBA_Object corba_object);

GnomeUnknownClient *gnome_unknown_activate_with_repo_id (GoadServerList *list,
						       const char *repo_id,
						       GoadActivationFlags flags,
						       const char **params);
GnomeUnknownClient *gnome_unknown_activate_with_goad_id (GoadServerList *list,
						       const char *goad_id,
						       GoadActivationFlags flags,
						       const char **params);
#endif /* _GNOME_UNKNOWN_CLIENT_H_ */
