
#ifndef __GNOME_PROPERTY_BAG_H__
#define __GNOME_PROPERTY_BAG_H__

#include <bonobo/gnome-object.h>

BEGIN_GNOME_DECLS

typedef struct _GnomePropertyBagPrivate GnomePropertyBagPrivate;
typedef struct _GnomePropertyBag	GnomePropertyBag;


typedef enum {
	GNOME_PROPERTY_UNSTORED	       = 1,
	GNOME_PROPERTY_READ_ONLY       = 2,
	GNOME_PROPERTY_USE_DEFAULT_OPT = 4
} GnomePropertyFlags;

#include <bonobo/gnome-property.h>

struct _GnomePropertyBag {
	GnomeObject               parent;
	GnomePropertyBagPrivate *priv;
};

typedef struct {
	GnomeObjectClass   parent;

	/*
	 * Signals.
	 */
	void (*value_changed)	(GnomePropertyBag *pb, char *name, char *type,
				 gpointer old_value, gpointer new_value);
} GnomePropertyBagClass;

GnomePropertyBag	 *gnome_property_bag_new	      (void);
		          				      
GtkType		          gnome_property_bag_get_type	      (void);
							      
/* Creating properties. */		   		      			
void		          gnome_property_bag_add	      (GnomePropertyBag *pb, const char *name,
							       const char *type, gpointer value,
							       gpointer default_value, const char *docstring,
							       GnomePropertyFlags flags);
							      
/* Modifying properties. */		   		      
void		          gnome_property_bag_set_value        (GnomePropertyBag *pb, const char *name,
							       gpointer value);
void		          gnome_property_bag_set_default      (GnomePropertyBag *pb, const char *name,
							       gpointer default_value);
void		          gnome_property_bag_set_docstring    (GnomePropertyBag *pb, const char *name,
							       char *docstring);
void		          gnome_property_bag_set_flags        (GnomePropertyBag *pb, const char *name,
							       GnomePropertyFlags flags);
							      
/* Inspecting properties. */
const char	         *gnome_property_bag_get_prop_type    (GnomePropertyBag *pb, const char *name);
const gpointer	          gnome_property_bag_get_value        (GnomePropertyBag *pb, const char *name);
const gpointer	          gnome_property_bag_get_default      (GnomePropertyBag *pb, const char *name);
const char	         *gnome_property_bag_get_docstring    (GnomePropertyBag *pb, const char *name);
const GnomePropertyFlags  gnome_property_bag_get_flags        (GnomePropertyBag *pb, const char *name);
gboolean		  gnome_property_bag_has_property     (GnomePropertyBag *pb, const char *name);

/*
 * Property types.
 */

/*
 * Mainly used by GnomeProperty for marshaling and demarshaling
 * property values across CORBA.
 */
typedef CORBA_any *(*GnomePropertyBagValueMarshalerFn)	      (const char *type, const gpointer value,
							       gpointer user_data);
typedef gpointer   (*GnomePropertyBagValueDemarshalerFn)      (const char *type, const CORBA_any *any,
							       gpointer user_data);
typedef void       (*GnomePropertyBagValueReleaserFn)         (const char *type, gpointer value,
							       gpointer user_data);
typedef gboolean   (*GnomePropertyBagValueComparerFn)         (const char *type, gpointer value1,
							       gpointer value2, gpointer user_data);

void		          gnome_property_bag_create_type      (GnomePropertyBag *pb, char *type_name,
							       GnomePropertyBagValueMarshalerFn   marshaler,
							       GnomePropertyBagValueDemarshalerFn demarshaler,
							       GnomePropertyBagValueComparerFn    comparer,
							       GnomePropertyBagValueReleaserFn    releaser,
							       gpointer user_data);
CORBA_any		 *gnome_property_bag_value_to_any     (GnomePropertyBag *pb, const char *type,
							       const gpointer value);
gpointer		  gnome_property_bag_any_to_value     (GnomePropertyBag *pb, const char *type,
							       const CORBA_any *any);
gboolean		  gnome_property_bag_compare_values   (GnomePropertyBag *pb, const char *type,
							       gpointer value1, gpointer value2);
							      

/* Persisting/depersisting properties. */
typedef gboolean          (*GnomePropertyBagPersisterFn)      (GnomePropertyBag *pb, const GNOME_Stream stream,
							       gpointer user_data);
typedef gboolean          (*GnomePropertyBagDepersisterFn)    (GnomePropertyBag *pb, const GNOME_Stream stream,
							       gpointer user_data);

void			  gnome_property_bag_set_persister    (GnomePropertyBag              *pb,
							       GnomePropertyBagPersisterFn    persister,
							       GnomePropertyBagDepersisterFn  depersister,
							       gpointer			      user_data);
gpointer		  gnome_property_bag_get_persist_data (GnomePropertyBag *pb);

/* A private function, only to be used by persistence implementations. */
GList			 *gnome_property_bag_get_prop_list    (GnomePropertyBag *pb);

/* For implementation inheritance. */
POA_GNOME_PropertyBag__epv *gnome_property_bag_get_epv	      (void);

#define GNOME_PROPERTY_BAG_TYPE			(gnome_property_bag_get_type ())
#define GNOME_PROPERTY_BAG(o)		        (GTK_CHECK_CAST ((o), GNOME_PROPERTY_BAG_TYPE, GnomePropertyBag))
#define GNOME_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), GNOME_PROPERTY_BAG_TYPE, GnomePropertyBagClass))
#define GNOME_IS_PROPERTY_BAG(o)		(GTK_CHECK_TYPE ((o), GNOME_PROPERTY_BAG_TYPE))
#define GNOME_IS_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_TYPE ((k), GNOME_PROPERTY_BAG_TYPE))

END_GNOME_DECLS

#endif /* ! __GNOME_PROPERTY_BAG_H__ */
