
#ifndef __BONOBO_PROPERTY_BAG_H__
#define __BONOBO_PROPERTY_BAG_H__

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboPropertyBagPrivate BonoboPropertyBagPrivate;
typedef struct _BonoboPropertyBag	 BonoboPropertyBag;

typedef enum {
	BONOBO_PROPERTY_UNSTORED        = 1,
	BONOBO_PROPERTY_READ_ONLY       = 2,
	BONOBO_PROPERTY_WRITE_ONLY      = 4,
	BONOBO_PROPERTY_USE_DEFAULT_OPT = 8
} BonoboPropertyFlags;

#define BONOBO_PROPERTY_READABLE(f)  (!((f) & BONOBO_PROPERTY_WRITE_ONLY))
#define BONOBO_PROPERTY_WRITEABLE(f) (!((f) & BONOBO_PROPERTY_READ_ONLY))

#include <bonobo/bonobo-arg.h>

typedef void (*BonoboPropertyGetFn) (BonoboPropertyBag *bag,
				     BonoboArg         *arg,
				     guint              arg_id,
				     gpointer           user_data);
typedef void (*BonoboPropertySetFn) (BonoboPropertyBag *bag,
				     const BonoboArg   *arg,
				     guint              arg_id,
				     gpointer           user_data);

#include <bonobo/bonobo-property.h>

struct _BonoboPropertyBag {
	BonoboObject              parent;
	BonoboPropertyBagPrivate *priv;
};

typedef struct {
	BonoboObjectClass   parent;
} BonoboPropertyBagClass;

#define BONOBO_PROPERTY_BAG_TYPE                (bonobo_property_bag_get_gtk_type ())
#define BONOBO_PROPERTY_BAG(o)		        (GTK_CHECK_CAST ((o), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBag))
#define BONOBO_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBagClass))
#define BONOBO_IS_PROPERTY_BAG(o)		(GTK_CHECK_TYPE ((o), BONOBO_PROPERTY_BAG_TYPE))
#define BONOBO_IS_PROPERTY_BAG_CLASS(k)		(GTK_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_BAG_TYPE))

GtkType		          bonobo_property_bag_get_gtk_type    (void);
BonoboPropertyBag	 *bonobo_property_bag_new	      (BonoboPropertyGetFn get_prop,
							       BonoboPropertySetFn set_prop,
							       gpointer            user_data);

void                      bonobo_property_bag_add              (BonoboPropertyBag  *pb,
								const char         *name,
								int                 idx,
								BonoboArgType       type,
								BonoboArg          *default_value,
								const char         *docstring,
								BonoboPropertyFlags flags);

void                      bonobo_property_bag_add_full         (BonoboPropertyBag  *pb,
								const char         *name,
								int                 idx,
								BonoboArgType       type,
								BonoboArg          *default_value,
								const char         *docstring,
								BonoboPropertyFlags flags,
								BonoboPropertyGetFn get_prop,
								BonoboPropertySetFn set_prop,
								gpointer            user_data);

BonoboArgType             bonobo_property_bag_get_type         (BonoboPropertyBag *pb, const char *name);

/* Modifying properties. */		   		      
void		          bonobo_property_bag_set_value        (BonoboPropertyBag *pb, const char *name,
								const BonoboArg   *value);

BonoboArg                *bonobo_property_bag_get_value        (BonoboPropertyBag *pb, const char *name);
BonoboArg                *bonobo_property_bag_get_default      (BonoboPropertyBag *pb, const char *name);
const char	         *bonobo_property_bag_get_docstring    (BonoboPropertyBag *pb, const char *name);
const BonoboPropertyFlags bonobo_property_bag_get_flags        (BonoboPropertyBag *pb, const char *name);

gboolean		  bonobo_property_bag_has_property     (BonoboPropertyBag *pb, const char *name);

/* A private function, only to be used by persistence implementations. */
GList                    *bonobo_property_bag_get_prop_list    (BonoboPropertyBag *pb);

/* For implementation inheritance. */
POA_Bonobo_PropertyBag__epv *bonobo_property_bag_get_epv	      (void);

END_GNOME_DECLS

#endif /* ! __BONOBO_PROPERTY_BAG_H__ */
