#ifndef _GNOME_CONTAINER_H_
#define _GNOME_CONTAINER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/gnome-object.h>
#include <bonobo/gnome-moniker.h>
#include <bonobo/gnome-container.h>

BEGIN_GNOME_DECLS
 
#define GNOME_CONTAINER_TYPE        (gnome_container_get_type ())
#define GNOME_CONTAINER(o)          (GTK_CHECK_CAST ((o), GNOME_CONTAINER_TYPE, GnomeContainer))
#define GNOME_CONTAINER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_CONTAINER_TYPE, GnomeContainerClass))
#define GNOME_IS_CONTAINER(o)       (GTK_CHECK_TYPE ((o), GNOME_CONTAINER_TYPE))
#define GNOME_IS_CONTAINER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_CONTAINER_TYPE))

typedef GList GnomeClientSiteList;

typedef struct {
	GnomeObject base;

	GnomeClientSiteList *client_sites;
	
	GnomeMoniker *moniker;
} GnomeContainer;

typedef struct {
	GnomeObjectClass parent_class;
	
} GnomeContainerClass;

GtkType       gnome_container_get_type    (void);
GnomeObject  *gnome_container_new         (void);
GnomeObject  *gnome_container_construct   (GnomeContainer *container);
GnomeMoniker *gnome_container_get_moniker (GnomeContainer *container);

void          gnome_container_add         (GnomeContainer *container,
					   GnomeObject    *object);

void         gnome_container_remove       (GnomeContainer *container,
					   GnomeObject    *object);

#endif

