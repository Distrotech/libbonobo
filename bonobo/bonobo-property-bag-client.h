#ifndef __GNOME_PROPERTY_BAG_CLIENT_H__
#define __GNOME_PROPERTY_BAG_CLIENT_H__

#include <libgnome/gnome-defs.h>
#include <bonobo/gnome-object.h>
#include <bonobo/gnome-stream.h>

BEGIN_GNOME_DECLS

typedef struct {
	GtkObject		        parent;
	GNOME_PropertyBag		corba_pb;
} GnomePropertyBagClient;

typedef struct {
	GtkObjectClass			parent;
} GnomePropertyBagClientClass;

GnomePropertyBagClient  *gnome_property_bag_client_new		      (GNOME_PropertyBag corba_property_bag);
GList			*gnome_property_bag_client_get_properties     (GnomePropertyBagClient *pbc);
GList			*gnome_property_bag_client_get_property_names (GnomePropertyBagClient *pbc);
GNOME_Property		 gnome_property_bag_client_get_property       (GnomePropertyBagClient *pbc,
								       char *property_name);
void			 gnome_property_bag_client_write_to_stream    (GnomePropertyBagClient *pbc,
									GnomeStream *stream);
void			 gnome_property_bag_client_read_from_stream   (GnomePropertyBagClient *pbc,
									GnomeStream *stream);

GtkType			 gnome_property_bag_client_get_type	      (void);

#define GNOME_PROPERTY_BAG_CLIENT_TYPE         (gnome_property_bag_client_get_type ())
#define GNOME_PROPERTY_BAG_CLIENT(o)           (GTK_CHECK_CAST ((o), GNOME_PROPERTY_BAG_CLIENT_TYPE, GnomePropertyBagClient))
#define GNOME_PROPERTY_BAG_CLIENT_CLASS(k)     (GTK_CHECK_CLASS_CAST((k), GNOME_PROPERTY_BAG_CLIENT_TYPE, GnomePropertyBagClientClass))
#define GNOME_IS_PROPERTY_BAG_CLIENT(o)        (GTK_CHECK_TYPE ((o), GNOME_PROPERTY_BAG_CLIENT_TYPE))
#define GNOME_IS_PROPERTY_BAG_CLIENT_CLASS(k)  (GTK_CHECK_CLASS_TYPE ((k), GNOME_PROPERTY_BAG_CLIENT_TYPE))

END_GNOME_DECLS

#endif /* ! ___GNOME_PROPERTY_BAG_CLIENT_H__ */
