/*
 * bonobo-property-bag.h: property bag object implementation.
 *
 * Authors:
 *   Nat Friedman   (nat@ximian.com)
 *   Michael Meeks  (michael@ximian.com)
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef __BONOBO_PROPERTY_BAG_H__
#define __BONOBO_PROPERTY_BAG_H__

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-event-source.h>

#include <gobject/gclosure.h>

G_BEGIN_DECLS

#define BONOBO_PROPERTY_READABLE     Bonobo_PROPERTY_READABLE
#define BONOBO_PROPERTY_WRITEABLE    Bonobo_PROPERTY_WRITEABLE
#define BONOBO_PROPERTY_NO_LISTENING Bonobo_PROPERTY_NO_LISTENING 
 
typedef struct _BonoboPropertyBagPrivate BonoboPropertyBagPrivate;
typedef struct _BonoboPropertyBag        BonoboPropertyBag;

typedef struct _BonoboProperty           BonoboProperty;

typedef void (*BonoboPropertyGetFn) (BonoboPropertyBag *bag,
				     BonoboArg         *arg,
				     guint              arg_id,
				     CORBA_Environment *ev,
				     gpointer           user_data);
typedef void (*BonoboPropertySetFn) (BonoboPropertyBag *bag,
				     const BonoboArg   *arg,
				     guint              arg_id,
				     CORBA_Environment *ev,
				     gpointer           user_data);

struct _BonoboPropertyBag {
	BonoboObject             parent;
	BonoboPropertyBagPrivate *priv;
	BonoboEventSource        *es;
};

typedef struct {
	BonoboObjectClass        parent;

	POA_Bonobo_PropertyBag__epv epv;
} BonoboPropertyBagClass;

#define BONOBO_PROPERTY_BAG_TYPE        (bonobo_property_bag_get_type ())
#define BONOBO_PROPERTY_BAG(o)		(G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBag))
#define BONOBO_PROPERTY_BAG_CLASS(k)	(G_TYPE_CHECK_CLASS_CAST((k), BONOBO_PROPERTY_BAG_TYPE, BonoboPropertyBagClass))
#define BONOBO_IS_PROPERTY_BAG(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_PROPERTY_BAG_TYPE))
#define BONOBO_IS_PROPERTY_BAG_CLASS(k)	(G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_PROPERTY_BAG_TYPE))

GType		          
bonobo_property_bag_get_type  (void);

BonoboPropertyBag *
bonobo_property_bag_new	           (BonoboPropertyGetFn get_prop_cb,
			            BonoboPropertySetFn set_prop_cb,
			            gpointer            user_data);

BonoboPropertyBag *
bonobo_property_bag_new_closure   (GClosure          *get_prop,
				   GClosure          *set_prop);

BonoboPropertyBag *
bonobo_property_bag_new_full      (GClosure          *get_prop,
				   GClosure          *set_prop,
				   BonoboEventSource *event_source);

BonoboPropertyBag *
bonobo_property_bag_construct     (BonoboPropertyBag *pb,
				   GClosure          *get_prop,
				   GClosure          *set_prop,
				   BonoboEventSource *event_source);

void                      
bonobo_property_bag_add           (BonoboPropertyBag   *pb,
				   const char          *name,
				   int                  idx,
				   BonoboArgType        type,
				   BonoboArg           *default_value,
				   const char          *doctitle,
				   Bonobo_PropertyFlags flags);

void
bonobo_property_bag_add_full      (BonoboPropertyBag    *pb,
				   const char           *name,
				   int                   idx,
				   BonoboArgType         type,
				   BonoboArg            *default_value,
				   const char           *doctitle,
				   const char           *docstring,
				   Bonobo_PropertyFlags  flags,
				   GClosure             *get_prop,
				   GClosure             *set_prop);

void
bonobo_property_bag_add_gtk_args  (BonoboPropertyBag   *pb,
				   GObject             *object);

GList *
bonobo_property_bag_get_prop_list (BonoboPropertyBag *pb);

G_END_DECLS

#endif /* ! __BONOBO_PROPERTY_BAG_H__ */
