#ifndef __BONOBO_PROPERTY_BAG_CLIENT_H__
#define __BONOBO_PROPERTY_BAG_CLIENT_H__

#include <stdarg.h>
#include <libgnome/gnome-defs.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-property-bag.h>

BEGIN_GNOME_DECLS

typedef struct {
	GtkObject		        parent;
	Bonobo_PropertyBag		corba_pb;
} BonoboPropertyBagClient;

typedef struct {
	GtkObjectClass			parent;
} BonoboPropertyBagClientClass;

/* Property bag proxy routines */
BonoboPropertyBagClient *bonobo_property_bag_client_new                (Bonobo_PropertyBag corba_property_bag);
GList			*bonobo_property_bag_client_get_properties     (BonoboPropertyBagClient *pbc);
void                     bonobo_property_bag_client_free_properties    (GList *list);
GList			*bonobo_property_bag_client_get_property_names (BonoboPropertyBagClient *pbc);
Bonobo_Property		 bonobo_property_bag_client_get_property       (BonoboPropertyBagClient *pbc,
									const char *property_name);
void			 bonobo_property_bag_client_persist	       (BonoboPropertyBagClient *pbc,
									Bonobo_Stream stream);
void			 bonobo_property_bag_client_depersist	       (BonoboPropertyBagClient *pbc,
									Bonobo_Stream stream);

GtkType			 bonobo_property_bag_client_get_type	       (void);

char                    *bonobo_property_bag_client_setv               (BonoboPropertyBagClient *pb,
									const char              *first_arg,
									va_list                  var_args);
char                    *bonobo_property_bag_client_getv               (BonoboPropertyBagClient *pb,
									const char              *first_arg,
									va_list                  var_args);

/*
 *
 * Property querying/manipulation routines.
 *
 * These are just provided as a convenience; you can also manipulate
 * the properties directly.
 *
 */

/* Querying the property type. */
CORBA_TypeCode           bonobo_property_bag_client_get_property_type   (BonoboPropertyBagClient *pbc,
									const char *propname);

/* Querying property values. */
gboolean		 bonobo_property_bag_client_get_value_boolean   (BonoboPropertyBagClient *pbc,
									const char *propname);
gshort			 bonobo_property_bag_client_get_value_short     (BonoboPropertyBagClient *pbc,
									const char *propname);
gushort			 bonobo_property_bag_client_get_value_ushort    (BonoboPropertyBagClient *pbc,
									const char *propname);
glong			 bonobo_property_bag_client_get_value_long      (BonoboPropertyBagClient *pbc,
									const char *propname);
gulong			 bonobo_property_bag_client_get_value_ulong     (BonoboPropertyBagClient *pbc,
								       	const char *propname);
gfloat			 bonobo_property_bag_client_get_value_float     (BonoboPropertyBagClient *pbc,
								       	const char *propname);
gdouble			 bonobo_property_bag_client_get_value_double    (BonoboPropertyBagClient *pbc,
								       	const char *propname);
char			*bonobo_property_bag_client_get_value_string    (BonoboPropertyBagClient *pbc,
								       	const char *propname);
CORBA_any		*bonobo_property_bag_client_get_value_any       (BonoboPropertyBagClient *pbc,
									const char *propname);

/* Querying property default values. */ 						  		       
gboolean		 bonobo_property_bag_client_get_default_boolean (BonoboPropertyBagClient *pbc,
									const char *propname);
gshort			 bonobo_property_bag_client_get_default_short   (BonoboPropertyBagClient *pbc,
									const char *propname);
gushort			 bonobo_property_bag_client_get_default_ushort  (BonoboPropertyBagClient *pbc,
									const char *propname);
glong			 bonobo_property_bag_client_get_default_long    (BonoboPropertyBagClient *pbc,
									const char *propname);
gulong			 bonobo_property_bag_client_get_default_ulong   (BonoboPropertyBagClient *pbc,
									const char *propname);
gfloat			 bonobo_property_bag_client_get_default_float   (BonoboPropertyBagClient *pbc,
									const char *propname);
gdouble			 bonobo_property_bag_client_get_default_double  (BonoboPropertyBagClient *pbc,
									const char *propname);
char			*bonobo_property_bag_client_get_default_string  (BonoboPropertyBagClient *pbc,
								       	const char *propname);
CORBA_any		*bonobo_property_bag_client_get_default_any     (BonoboPropertyBagClient *pbc,
									const char *propname);

/* Setting property values. */
void			 bonobo_property_bag_client_set_value_boolean   (BonoboPropertyBagClient *pbc,
								       	const char *propname,
									gboolean value);
void			 bonobo_property_bag_client_set_value_short     (BonoboPropertyBagClient *pbc,
								       	const char *propname,
									gshort value);
void			 bonobo_property_bag_client_set_value_ushort    (BonoboPropertyBagClient *pbc,
								       	const char *propname,
									gushort value);
void			 bonobo_property_bag_client_set_value_long      (BonoboPropertyBagClient *pbc,
								       	const char *propname,
									glong value);
void			 bonobo_property_bag_client_set_value_ulong     (BonoboPropertyBagClient *pbc,
								       	const char *propname,
									gulong value);
void			 bonobo_property_bag_client_set_value_float     (BonoboPropertyBagClient *pbc,
								       	const char *propname,
									gfloat value);
void			 bonobo_property_bag_client_set_value_double    (BonoboPropertyBagClient *pbc,
									const char *propname,
									gdouble value);
void			 bonobo_property_bag_client_set_value_string    (BonoboPropertyBagClient *pbc,
									const char *propname,
									const char *value);
void			 bonobo_property_bag_client_set_value_any       (BonoboPropertyBagClient *pbc,
									const char *propname,
									CORBA_any *value);

/* Querying other fields and flags. */
char			*bonobo_property_bag_client_get_docstring       (BonoboPropertyBagClient *pbc,
									const char *propname);
BonoboPropertyFlags	 bonobo_property_bag_client_get_flags	       (BonoboPropertyBagClient *pbc,
									const char *propname);


#define BONOBO_PROPERTY_BAG_CLIENT_TYPE        (bonobo_property_bag_client_get_type ())
#define BONOBO_PROPERTY_BAG_CLIENT(o)          (GTK_CHECK_CAST ((o), BONOBO_PROPERTY_BAG_CLIENT_TYPE, BonoboPropertyBagClient))
#define BONOBO_PROPERTY_BAG_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_BAG_CLIENT_TYPE, BonoboPropertyBagClientClass))
#define BONOBO_IS_PROPERTY_BAG_CLIENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_PROPERTY_BAG_CLIENT_TYPE))
#define BONOBO_IS_PROPERTY_BAG_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_BAG_CLIENT_TYPE))

END_GNOME_DECLS

#endif /* ! ___BONOBO_PROPERTY_BAG_CLIENT_H__ */
