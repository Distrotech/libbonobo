/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
} GnomeObjectClient;

typedef struct {
	GnomeObjectClass parent_class;
} GnomeObjectClientClass;

GtkType            gnome_object_client_get_type        (void);

GnomeObjectClient *gnome_object_client_construct       (GnomeObjectClient *object_client,
							CORBA_Object corba_object);

GnomeObjectClient *gnome_object_activate               (const char *object_desc,
							GoadActivationFlags flags);
GnomeObjectClient *gnome_object_activate_with_repo_id  (GoadServerList *list,
							const char *repo_id,
							GoadActivationFlags flags,
							const char **params);
GnomeObjectClient *gnome_object_activate_with_goad_id  (GoadServerList *list,
							const char *goad_id,
							GoadActivationFlags flags,
							const char **params);
GNOME_Unknown      gnome_object_restore_from_url       (const char *goad_id,
							const char *url);
GnomeObjectClient *gnome_object_client_from_corba      (GNOME_Unknown o);

/* Convenience GNOME_Unknown wrappers */
gboolean           gnome_object_client_has_interface   (GnomeObjectClient *object,
							const char *interface_desc,
							CORBA_Environment *opt_ev);
GNOME_Unknown      gnome_object_client_query_interface (GnomeObjectClient *object,
							const char *interface_desc,
							CORBA_Environment *opt_ev);

#endif /* _GNOME_OBJECT_CLIENT_H_ */

