#ifndef __GNOME_PROPERTY_BAG_CLIENT_H__
#define __GNOME_PROPERTY_BAG_CLIENT_H__

#include <libgnome/gnome-defs.h>
#include <bonobo/gnome-object.h>
#include <bonobo/gnome-stream.h>
#include <bonobo/gnome-property-bag.h>

BEGIN_GNOME_DECLS

typedef struct {
	GtkObject		        parent;
	GNOME_PropertyBag		corba_pb;
} GnomePropertyBagClient;

typedef struct {
	GtkObjectClass			parent;
} GnomePropertyBagClientClass;

/* Property bag proxy routines */
GnomePropertyBagClient  *gnome_property_bag_client_new		      (GNOME_PropertyBag corba_property_bag);
GList			*gnome_property_bag_client_get_properties     (GnomePropertyBagClient *pbc);
void                     gnome_property_bag_client_free_properties    (GList *list);
GList			*gnome_property_bag_client_get_property_names (GnomePropertyBagClient *pbc);
GNOME_Property		 gnome_property_bag_client_get_property       (GnomePropertyBagClient *pbc,
								       const char *property_name);
void			 gnome_property_bag_client_persist	      (GnomePropertyBagClient *pbc,
								       GNOME_Stream stream);
void			 gnome_property_bag_client_depersist	      (GnomePropertyBagClient *pbc,
								       GNOME_Stream stream);

GtkType			 gnome_property_bag_client_get_type	      (void);



/*
 *
 * Property querying/manipulation routines.
 *
 * These are just provided as a convenience; you can also manipulate
 * the properties directly.
 *
 */

/* Querying the property type. */
CORBA_TypeCode           gnome_property_bag_client_get_property_type   (GnomePropertyBagClient *pbc,
									const char *propname);

/* Querying property values. */
gboolean		 gnome_property_bag_client_get_value_boolean   (GnomePropertyBagClient *pbc,
									const char *propname);
gshort			 gnome_property_bag_client_get_value_short     (GnomePropertyBagClient *pbc,
									const char *propname);
gushort			 gnome_property_bag_client_get_value_ushort    (GnomePropertyBagClient *pbc,
									const char *propname);
glong			 gnome_property_bag_client_get_value_long      (GnomePropertyBagClient *pbc,
									const char *propname);
gulong			 gnome_property_bag_client_get_value_ulong     (GnomePropertyBagClient *pbc,
								       	const char *propname);
gfloat			 gnome_property_bag_client_get_value_float     (GnomePropertyBagClient *pbc,
								       	const char *propname);
gdouble			 gnome_property_bag_client_get_value_double    (GnomePropertyBagClient *pbc,
								       	const char *propname);
char			*gnome_property_bag_client_get_value_string    (GnomePropertyBagClient *pbc,
								       	const char *propname);
CORBA_any		*gnome_property_bag_client_get_value_any       (GnomePropertyBagClient *pbc,
									const char *propname);

/* Querying property default values. */ 						  		       
gboolean		 gnome_property_bag_client_get_default_boolean (GnomePropertyBagClient *pbc,
									const char *propname);
gshort			 gnome_property_bag_client_get_default_short   (GnomePropertyBagClient *pbc,
									const char *propname);
gushort			 gnome_property_bag_client_get_default_ushort  (GnomePropertyBagClient *pbc,
									const char *propname);
glong			 gnome_property_bag_client_get_default_long    (GnomePropertyBagClient *pbc,
									const char *propname);
gulong			 gnome_property_bag_client_get_default_ulong   (GnomePropertyBagClient *pbc,
									const char *propname);
gfloat			 gnome_property_bag_client_get_default_float   (GnomePropertyBagClient *pbc,
									const char *propname);
gdouble			 gnome_property_bag_client_get_default_double  (GnomePropertyBagClient *pbc,
									const char *propname);
char			*gnome_property_bag_client_get_default_string  (GnomePropertyBagClient *pbc,
								       	const char *propname);
CORBA_any		*gnome_property_bag_client_get_default_any     (GnomePropertyBagClient *pbc,
									const char *propname);

/* Setting property values. */
void			 gnome_property_bag_client_set_value_boolean   (GnomePropertyBagClient *pbc,
								       	const char *propname,
									gboolean value);
void			 gnome_property_bag_client_set_value_short     (GnomePropertyBagClient *pbc,
								       	const char *propname,
									gshort value);
void			 gnome_property_bag_client_set_value_ushort    (GnomePropertyBagClient *pbc,
								       	const char *propname,
									gushort value);
void			 gnome_property_bag_client_set_value_long      (GnomePropertyBagClient *pbc,
								       	const char *propname,
									glong value);
void			 gnome_property_bag_client_set_value_ulong     (GnomePropertyBagClient *pbc,
								       	const char *propname,
									gulong value);
void			 gnome_property_bag_client_set_value_float     (GnomePropertyBagClient *pbc,
								       	const char *propname,
									gfloat value);
void			 gnome_property_bag_client_set_value_double    (GnomePropertyBagClient *pbc,
									const char *propname,
									gdouble value);
void			 gnome_property_bag_client_set_value_string    (GnomePropertyBagClient *pbc,
									const char *propname,
									char *value);
void			 gnome_property_bag_client_set_value_any       (GnomePropertyBagClient *pbc,
									const char *propname,
									CORBA_any *value);

/* Querying other fields and flags. */
char			*gnome_property_bag_client_get_docstring       (GnomePropertyBagClient *pbc,
									const char *propname);
GnomePropertyFlags	 gnome_property_bag_client_get_flags	       (GnomePropertyBagClient *pbc,
									const char *propname);


#define GNOME_PROPERTY_BAG_CLIENT_TYPE        (gnome_property_bag_client_get_type ())
#define GNOME_PROPERTY_BAG_CLIENT(o)          (GTK_CHECK_CAST ((o), GNOME_PROPERTY_BAG_CLIENT_TYPE, GnomePropertyBagClient))
#define GNOME_PROPERTY_BAG_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_PROPERTY_BAG_CLIENT_TYPE, GnomePropertyBagClientClass))
#define GNOME_IS_PROPERTY_BAG_CLIENT(o)       (GTK_CHECK_TYPE ((o), GNOME_PROPERTY_BAG_CLIENT_TYPE))
#define GNOME_IS_PROPERTY_BAG_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_PROPERTY_BAG_CLIENT_TYPE))

END_GNOME_DECLS

#endif /* ! ___GNOME_PROPERTY_BAG_CLIENT_H__ */
