#ifndef _GNOME_OBJECT_H_
#define _GNOME_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <GnomeObject.h>

BEGIN_GNOME_DECLS
 
#define GNOME_OBJECT_TYPE        (gnome_object_get_type ())
#define GNOME_OBJECT_ITEM(o)     (GTK_CHECK_CAST ((o), GNOME_OBJECT_TYPE, GnomeObject))
#define GNOME_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_OBJECT_TYPE, GnomeObjectClass))
#define GNOME_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), GNOME_OBJECT_TYPE))
#define GNOME_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_OBJECT_TYPE))

typedef struct {
	GtkObject base;

	CORBA_Object object;
	CORBA_environment ev;
} GnomeObject;

typedef struct {
	GtkObjectClass parent_class;
	void  (*test)(GnomeObject *object);
} GnomeObjectClass;

GtkType      gnome_object_get_type         (void);

GnomeObject *gnome_object_from_servant     (POA_GNOME_object *servant);

void         gnome_object_bind_to_servant  (GnomeObject *object,
					    void *servant);
void         gnome_object_drop_binding     (void *servant);



/* CORBA defaults we provide */
PortableServer_ServantBase__epv gnome_object_base_epv;
POA_GNOME_object__epv gnome_object_epv;
POA_GNOME_object__vepv gnome_object_vepv;


#endif
