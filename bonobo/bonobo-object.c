/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME Unknown interface base implementation
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-object.h>
#include "bonobo.h"

typedef struct {
	int   ref_count;
	GList *objs;
} GnomeAggregateObject;

struct _GnomeObjectPrivate {
	GnomeAggregateObject *ao;
	int destroy_id;
};

enum {
	QUERY_INTERFACE,
	LAST_SIGNAL
};

static guint gnome_object_signals [LAST_SIGNAL];
static GtkObjectClass *gnome_object_parent_class;

/* Assumptions made: sizeof(POA_interfacename) does not change between interfaces */

/**
 * gnome_object_from_servant:
 * @servant: Your servant.
 *
 * CORBA method implementations receive a parameter of type
 * #PortableServer_Servant which is a pointer to the servant that was
 * used to create this specific CORBA object instance.
 *
 * This routine allows the user to get the #GnomeObject (ie, Gtk
 * object wrapper) from the servant.  This #GnomeObject is the Gtk
 * object wrapper associated with the CORBA object instance whose
 * method is being invoked.
 * 
 * Returns: the #GnomeObject wrapper associated with @servant.
 */
GnomeObject *
gnome_object_from_servant (PortableServer_Servant servant)
{
	g_return_val_if_fail (servant != NULL, NULL);

	return GNOME_OBJECT(((GnomeObjectServant *)servant)->gnome_object);
}

/**
 * gnome_object_get_servant:
 * @object: A #GnomeObject which is associated with a servant.
 *
 * Returns: The servant associated with @object, or %NULL if
 * no servant is bound to @object.
 */
PortableServer_Servant
gnome_object_get_servant (GnomeObject *object)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), NULL);

	return object->servant;
}

/**
 * gnome_object_bind_to_servant:
 * @object: the GnomeObject to bind to the servant.
 * @servant: A PortableServer_Servant to bind to the GnomeObject.
 *
 * This routine binds @object to @servant.  It establishes a one to
 * one mapping between the @object and the @servant.  Utility routines
 * are provided to go back and forth.  See gnome_object_from_servant()
 * and gnome_object_get_servant().
 *
 * This routine is used internally by gnome_object_activate_servant().
 */
void
gnome_object_bind_to_servant (GnomeObject *object, void *servant)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (servant != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	object->servant = servant;
	((GnomeObjectServant *)servant)->gnome_object = object;
}

/**
 * gnome_object_ref:
 * @object: A GnomeObject you want to ref-count
 *
 * increments the reference count for the aggregate GnomeObject.
 */
void
gnome_object_ref (GnomeObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));
	g_return_if_fail (object->priv->ao->ref_count != 0);

	object->priv->ao->ref_count++;
}

/**
 * gnome_object_unref:
 * @object: A GnomeObject you want to unref.
 *
 * decrements the reference count for the aggregate GnomeObject.
 */
void
gnome_object_unref (GnomeObject *object)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));
	g_return_if_fail (object->priv->ao->ref_count > 0);

	object->priv->ao->ref_count--;

	if (object->priv->ao->ref_count == 0){
		GnomeAggregateObject *ao = object->priv->ao;
		GList *l;
		
		for (l = ao->objs; l; l = l->next){
			GnomeObject *o = l->data;
			
			gtk_signal_disconnect (GTK_OBJECT (o), o->priv->destroy_id);
			gtk_object_destroy (l->data);
		}

		g_list_free (ao->objs);
		g_free (ao);
	}
}

static void
impl_GNOME_Unknown__destroy (PortableServer_Servant servant, CORBA_Environment *ev)
{
	POA_GNOME_Unknown__fini ((POA_GNOME_Unknown *)servant, ev);
	g_free (servant);
}

static void
impl_GNOME_Unknown_ref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeObject *object;

	object = gnome_object_from_servant (servant);
	object->priv->ao->ref_count++;
}

static void
impl_GNOME_Unknown_unref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeObject *object;

	object = gnome_object_from_servant (servant);

	gnome_object_unref (object);
}

static CORBA_Object
impl_GNOME_Unknown_query_interface (PortableServer_Servant servant,
				    CORBA_char *repoid,
				    CORBA_Environment *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	GnomeObject *object;
	GtkType type;
	GList *l;
	
	object = gnome_object_from_servant (servant);

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);

	gtk_signal_emit (
		GTK_OBJECT (object), gnome_object_signals [QUERY_INTERFACE],
		repoid, &retval);

	if (!CORBA_Object_is_nil (retval, ev)){
		object->priv->ao->ref_count++;
		return retval;
	}
	
	type = gtk_type_from_name (repoid);
	
	/* Try looking at the gtk types */
	for (l = object->priv->ao->objs; l; l = l->next){
		GnomeObject *tryme = l->data;
		
		if ((type && gtk_type_is_a(GTK_OBJECT(tryme)->klass->type, type)) ||
#ifdef ORBIT_IMPLEMENTS_IS_A
		    CORBA_Object_is_a(tryme->corba_objref, repoid, ev)
#else
		    !strcmp(tryme->corba_objref->object_id, repoid)
#endif
			){
			retval = CORBA_Object_duplicate (tryme->corba_objref, ev);
			break;
		}
	}
	if (!CORBA_Object_is_nil (retval, ev))
		object->priv->ao->ref_count++;
	
	return retval;
}

PortableServer_ServantBase__epv gnome_object_base_epv =
{
	NULL,			/* _private data */
	NULL,                   /* finalize routine */
	NULL,			/* default_POA routine */
};

POA_GNOME_Unknown__epv gnome_object_epv =
{
	NULL,			/* _private */
	&impl_GNOME_Unknown_ref,
	&impl_GNOME_Unknown_unref,
	&impl_GNOME_Unknown_query_interface,
};

POA_GNOME_Unknown__vepv gnome_object_vepv = {
	&gnome_object_base_epv,
	&gnome_object_epv
};

static void
gnome_object_destroy (GtkObject *object)
{
	GnomeObject *gnome_object = GNOME_OBJECT (object);
	void *servant = gnome_object->servant;
	
	if (gnome_object->corba_objref != CORBA_OBJECT_NIL){
		PortableServer_ObjectId *oid;
		CORBA_Object_release (gnome_object->corba_objref, &gnome_object->ev);
		
		oid = PortableServer_POA_servant_to_id(bonobo_poa(), servant, &gnome_object->ev);
		PortableServer_POA_deactivate_object (
			bonobo_poa (), oid, &gnome_object->ev);
		CORBA_free(oid);
	}
	CORBA_exception_free (&gnome_object->ev);

	g_free (gnome_object->priv);
	gnome_object_parent_class->destroy (object);
}

static void
gnome_object_class_init (GnomeObjectClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_object_parent_class = gtk_type_class (gtk_object_get_type ());

	gnome_object_signals [QUERY_INTERFACE] =
		gtk_signal_new ("query_interface",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(GnomeObjectClass,query_interface), 
				gtk_marshal_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 2, GTK_TYPE_POINTER, GTK_TYPE_POINTER); 
	gtk_object_class_add_signals (object_class, gnome_object_signals, LAST_SIGNAL);

	object_class->destroy = gnome_object_destroy;
}

static void
gnome_object_usage_error (GnomeObject *object)
{
	g_error ("Aggregate object member %p has been destroyed\n", object);
}

static void
gnome_object_instance_init (GtkObject *gtk_object)
{
	GnomeObject *object = GNOME_OBJECT (gtk_object);
	
	CORBA_exception_init (&object->ev);
	object->priv = g_new (GnomeObjectPrivate, 1);
	object->priv->destroy_id = gtk_signal_connect (
		gtk_object, "destroy", GTK_SIGNAL_FUNC (gnome_object_usage_error),
		NULL);
	
	object->priv->ao = g_new0 (GnomeAggregateObject, 1);
	object->priv->ao->objs = g_list_append (object->priv->ao->objs, object);
	object->priv->ao->ref_count = 1;
}

/**
 * gnome_object_get_type:
 *
 * Returns: the GtkType associated with the base GnomeObject class type.
 */
GtkType
gnome_object_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/obj:1.0",
			sizeof (GnomeObject),
			sizeof (GnomeObjectClass),
			(GtkClassInitFunc) gnome_object_class_init,
			(GtkObjectInitFunc) gnome_object_instance_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

/** 
 * gnome_object_activate_servant:
 * @object: a GnomeObject
 * @servant: The servant to activate.
 *
 * This routine activates the @servant which is wrapped inside the
 * @object on the bonobo_poa (which is the default POA).
 *
 * Returns: The CORBA_Object that is wrapped by @object and whose
 * servant is @servant.  Might return CORBA_OBJECT_NIL on failure. 
 */
CORBA_Object
gnome_object_activate_servant (GnomeObject *object, void *servant)
{
	CORBA_Object o;

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), CORBA_OBJECT_NIL);
	g_return_val_if_fail (servant != NULL, CORBA_OBJECT_NIL);
	
	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &object->ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &object->ev);

	if (o){
		object->corba_objref = o;
		gnome_object_bind_to_servant (object, servant);
		return o;
	} else
		return CORBA_OBJECT_NIL;
	
}

/**
 * gnome_object_construct:
 * @object: The GTK object server wrapper for the CORBA service.
 * @corba_object: the reference to the real CORBA object.
 *
 * Initializes the provided GnomeObject @object. This method is
 * usually invoked from the construct method for other Gtk-based CORBA
 * wrappers that derive from the GNOME::obj interface
 *
 * Returns: the initialized GnomeObject.
 */
GnomeObject *
gnome_object_construct (GnomeObject *object, CORBA_Object corba_object)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);

	object->corba_objref = corba_object;

	/* GnomeObjects are self-owned */
	GTK_OBJECT_UNSET_FLAGS (GTK_OBJECT (object), GTK_FLOATING);

	return object;
}

/**
 * gnome_object_add_interface:
 * @object: The GnomeObject to which an interface is going to be added.
 * @newobj: The GnomeObject containing the new interface to be added.
 *
 * Adds the interfaces supported by @newobj to the list of interfaces
 * for @object.  This function adds the interfaces supported by
 * @newobj to the list of interfaces support by @object.
 */
void
gnome_object_add_interface (GnomeObject *object, GnomeObject *newobj)
{
       GnomeAggregateObject *oldao;
       GList *l;

       if (object->priv->ao == newobj->priv->ao)
               return;

       /*
	* Explanation:
	*   Bonobo Objects should not be assembled after they have been
	*   exposed, or we would be breaking the contract we have with
	*   the other side.
	*
	*   This check is not perfect, but might help some people.
	*/
       g_return_if_fail (object->priv->ao->ref_count == 1);
       g_return_if_fail (newobj->priv->ao->ref_count == 1);
       
       oldao = newobj->priv->ao;

       /* Merge the two AggregateObject lists */
       for (l = newobj->priv->ao->objs; l; l = l->next){
               if (!g_list_find (object->priv->ao->objs, l->data)){
                       object->priv->ao->objs = g_list_prepend (object->priv->ao->objs, l->data);
                       ((GnomeObject *)l->data)->priv->ao = object->priv->ao;
               }
       }

       g_list_free (oldao->objs);
       g_free (oldao);
}

/**
 * gnome_object_query_interface:
 * @object: A GnomeObject to be queried for a given interface.
 * @repo_id: The name of the interface to be queried.
 *
 * Returns: The CORBA interface named @repo_id for @object.
 */
CORBA_Object
gnome_object_query_interface (GnomeObject *object, const char *repo_id)
{
       CORBA_Environment ev;
       CORBA_Object retval;

       CORBA_exception_init(&ev);
       retval = GNOME_Unknown_query_interface (object->corba_objref, (CORBA_char *)repo_id, &ev);
       if(ev._major != CORBA_NO_EXCEPTION)
               retval = CORBA_OBJECT_NIL;
       CORBA_exception_free (&ev);
       return retval;
}

/**
 * gnome_object_corba_objref:
 * @object: A GnomeObject whose CORBA object is requested.
 *
 * Returns: The CORBA interface object for which @object is a wrapper.
 */
CORBA_Object
gnome_object_corba_objref (GnomeObject *object)
{
	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (GNOME_IS_OBJECT (object), NULL);
	
	return object->corba_objref;
}

