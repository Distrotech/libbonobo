#ifndef _GNOME_OBJECT_H_
#define _GNOME_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>

BEGIN_GNOME_DECLS
 
#define GNOME_OBJECT_TYPE        (gnome_object_get_type ())
#define GNOME_OBJECT(o)          (GTK_CHECK_CAST ((o), GNOME_OBJECT_TYPE, GnomeObject))
#define GNOME_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_OBJECT_TYPE, GnomeObjectClass))
#define GNOME_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), GNOME_OBJECT_TYPE))
#define GNOME_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_OBJECT_TYPE))

typedef struct {
	GtkObject base;

	CORBA_Object object;
	CORBA_Environment ev;
} GnomeObject;

typedef struct {
	GtkObjectClass parent_class;
	void  (*test)(GnomeObject *object);
} GnomeObjectClass;

GtkType      gnome_object_get_type         (void);

GnomeObject *gnome_object_from_servant     (PortableServer_Servant servant);

void         gnome_object_bind_to_servant  (GnomeObject *object,
					    void *servant);

GnomeObject *gnome_object_construct        (GnomeObject *object,
					    CORBA_Object corba_object);

void  gnome_object_drop_binding_by_servant (void *servant);
void  gnome_object_drop_binding            (GnomeObject *obj);

/*
 * Activates a newly created servant.  Just an utility function.
 */
CORBA_Object gnome_object_activate_servant (GnomeObject *object,
					    void *servant);

#ifdef GNORBA_H
GnomeObject *gnome_object_activate_with_repo_id (GoadServerList *list,
						 const char *repo_id,
						 GoadActivationFlags flags,
						 const char **params);
#endif

/* CORBA defaults we provide */
PortableServer_ServantBase__epv gnome_object_base_epv;
POA_GNOME_object__epv gnome_object_epv;
POA_GNOME_object__vepv gnome_object_vepv;

END_GNOME_DECLS

#endif
