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

/*
 * If you're using a custom servant for your CORBA objects, just make
 * sure that the second element is a 'gpointer' to hold the GnomeObject
 * pointer for servant->GnomeObject translation
 */
typedef struct {
	POA_GNOME_obj servant_placeholder;
	gpointer gnome_object;
} GnomeObjectServant;

typedef struct {
	GtkObject base;

	CORBA_Object object;
	CORBA_Environment ev;
	gpointer servant;
} GnomeObject;

typedef struct {
	GtkObjectClass parent_class;
	void  (*test)(GnomeObject *object);
} GnomeObjectClass;

GtkType      gnome_object_get_type         (void);
GnomeObject *gnome_object_construct        (GnomeObject *object,
					    CORBA_Object corba_object);

GnomeObject *gnome_object_from_servant     (PortableServer_Servant servant);
void         gnome_object_bind_to_servant  (GnomeObject *object,
					    void *servant);

CORBA_Object gnome_object_activate_servant (GnomeObject *object
					    void *servant);

/* CORBA default vector methods we provide */
extern PortableServer_ServantBase__epv gnome_obj_base_epv;
extern POA_GNOME_obj__epv           gnome_obj_epv;
extern POA_GNOME_obj__vepv          gnome_obj_vepv;

END_GNOME_DECLS

#endif
