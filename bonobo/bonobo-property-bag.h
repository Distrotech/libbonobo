
#ifndef __BONOBO_PROPERTY_BAG_H__
#define __BONOBO_PROPERTY_BAG_H__

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboPropertyBagPrivate BonoboPropertyBagPrivate;
typedef struct _BonoboPropertyBag	BonoboPropertyBag;


typedef enum {
	BONOBO_PROPERTY_UNSTORED	       = 1,
	BONOBO_PROPERTY_READ_ONLY       = 2,
	BONOBO_PROPERTY_USE_DEFAULT_OPT = 4
} BonoboPropertyFlags;

#include <bonobo/bonobo-property.h>

struct _BonoboPropertyBag {
	BonoboObject               parent;
	BonoboPropertyBagPrivate *priv;
};

typedef struct {
	BonoboObjectClass   parent;

	/*
	 * Signals.
	 */
	void (*value_changed)	(BonoboPropertyBag *pb, char *name, char *type,
				 gpointer old_value, gpointer new_value);
} BonoboPropertyBagClass;

BonoboPropertyBag	 *bonobo_property_bag_new	      (void);
		          				      
GtkType		          bonobo_property_bag_get_type	      (void);
							      
/* Creating properties. */		   		      			
void		          bonobo_property_bag_add	      (BonoboPropertyBag *pb, const char *name,
							       const char *type, gpointer value,
							       gpointer default_value, const char *docstring,
							       BonoboPropertyFlags flags);
							      
/* Modifying properties. */		   		      
void		          bonobo_property_bag_set_value        (BonoboPropertyBag *pb, const char *name,
							       gpointer value);
void		          bonobo_property_bag_set_default      (BonoboPropertyBag *pb, const char *name,
							       gpointer default_value);
void		          bonobo_property_bag_set_docstring    (BonoboPropertyBag *pb, const char *name,
							       char *docstring);
void		          bonobo_property_bag_set_flags        (BonoboPropertyBag *pb, const char *name,
							       BonoboPropertyFlags flags);
							      
/* Inspecting properties. */
const char	         *bonobo_property_bag_get_prop_type    (BonoboPropertyBag *pb, const char *name);
gconstpointer	          bonobo_property_bag_get_value        (BonoboPropertyBag *pb, const char *name);
gconstpointer	          bonobo_property_bag_get_default      (BonoboPropertyBag *pb, const char *name);
const char	         *bonobo_property_bag_get_docstring    (BonoboPropertyBag *pb, const char *name);
const BonoboPropertyFlags  bonobo_property_bag_get_flags        (BonoboPropertyBag *pb, const char *name);
gboolean		  bonobo_property_bag_has_property     (BonoboPropertyBag *pb, const char *name);

/*
 * Property types.
 */

/*
 * Mainly used by BonoboProperty for marshaling and demarshaling
 * property values across CORBA.
 */
typedef CORBA_any *(*BonoboPropertyBagValueMarshalerFn)	      (const char *type, gconstpointer value,
							       gpointer user_data);
typedef gpointer   (*BonoboPropertyBagValueDemarshalerFn)      (const char *type, const CORBA_any *any,
							       gpointer user_data);
typedef void       (*BonoboPropertyBagValueReleaserFn)         (const char *type, gpointer value,
							       gpointer user_data);
typedef gboolean   (*BonoboPropertyBagValueComparerFn)         (const char *type, gpointer value1,
							       gpointer value2, gpointer user_data);

void		          bonobo_property_bag_create_type      (BonoboPropertyBag *pb, char *type_name,
							       BonoboPropertyBagValueMarshalerFn   marshaler,
							       BonoboPropertyBagValueDemarshalerFn demarshaler,
							       BonoboPropertyBagValueComparerFn    comparer,
							       BonoboPropertyBagValueReleaserFn    releaser,
							       gpointer user_data);
CORBA_any		 *bonobo_property_bag_value_to_any     (BonoboPropertyBag *pb, const char *type,
							       gconstpointer value);
gpointer		  bonobo_property_bag_any_to_value     (BonoboPropertyBag *pb, const char *type,
							       const CORBA_any *any);
gboolean		  bonobo_property_bag_compare_values   (BonoboPropertyBag *pb, const char *type,
							       gpointer value1, gpointer value2);
							      

/* Persisting/depersisting properties. */
typedef gboolean          (*BonoboPropertyBagPersisterFn)      (BonoboPropertyBag *pb, const Bonobo_Stream stream,
							       gpointer user_data);
typedef gboolean          (*BonoboPropertyBagDepersisterFn)    (BonoboPropertyBag *pb, const Bonobo_Stream stream,
							       gpointer user_data);

void			  bonobo_property_bag_set_persister    (BonoboPropertyBag              *pb,
							       BonoboPropertyBagPersisterFn    persister,
							       BonoboPropertyBagDepersisterFn  depersister,
							       gpointer			      user_data);
gpointer		  bonobo_property_bag_get_persist_data (BonoboPropertyBag *pb);

/* A private function, only to be used by persistence implementations. */
GList			 *bonobo_property_bag_get_prop_list    (BonoboPropertyBag *pb);

/* For implementation inheritance. */
POA_Bonobo_PropertyBag__epv *bonobo_property_bag_get_epv	      (void);

#define BONOBO_PROPERTY_BAG_TYPE			(bonobo_property_bag_get_type ())
#define BONOBO_PROPERTY_BAG(o)		        (GTK_CHECK_CAST ((o), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBag))
#define BONOBO_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBagClass))
#define BONOBO_IS_PROPERTY_BAG(o)		(GTK_CHECK_TYPE ((o), BONOBO_PROPERTY_BAG_TYPE))
#define BONOBO_IS_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_BAG_TYPE))

END_GNOME_DECLS

#endif /* ! __BONOBO_PROPERTY_BAG_H__ */
