/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo Unknown interface base implementation
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include "Bonobo.h"
#include "bonobo-object-directory.h"

POA_Bonobo_Unknown__vepv bonobo_object_vepv;

typedef struct {
	int   ref_count;
	GList *objs;
} GnomeAggregateObject;

struct _BonoboObjectPrivate {
	GnomeAggregateObject *ao;
	int destroy_id;
};

enum {
	QUERY_INTERFACE,
	SYSTEM_EXCEPTION,
	OBJECT_GONE,
	LAST_SIGNAL
};

static guint bonobo_object_signals [LAST_SIGNAL];
static GtkObjectClass *bonobo_object_parent_class;

/* Assumptions made: sizeof(POA_interfacename) does not change between interfaces */

/**
 * bonobo_object_from_servant:
 * @servant: Your servant.
 *
 * CORBA method implementations receive a parameter of type
 * #PortableServer_Servant which is a pointer to the servant that was
 * used to create this specific CORBA object instance.
 *
 * This routine allows the user to get the #BonoboObject (ie, Gtk
 * object wrapper) from the servant.  This #BonoboObject is the Gtk
 * object wrapper associated with the CORBA object instance whose
 * method is being invoked.
 *
 * Returns: the #BonoboObject wrapper associated with @servant.
 */
BonoboObject *
bonobo_object_from_servant (PortableServer_Servant servant)
{
	g_return_val_if_fail (servant != NULL, NULL);

	return BONOBO_OBJECT(((BonoboObjectServant *)servant)->bonobo_object);
}

/**
 * bonobo_object_get_servant:
 * @object: A #BonoboObject which is associated with a servant.
 *
 * Returns: The servant associated with @object, or %NULL if
 * no servant is bound to @object.
 */
PortableServer_Servant
bonobo_object_get_servant (BonoboObject *object)
{
	g_return_val_if_fail (BONOBO_IS_OBJECT (object), NULL);

	return object->servant;
}

/**
 * bonobo_object_bind_to_servant:
 * @object: the BonoboObject to bind to the servant.
 * @servant: A PortableServer_Servant to bind to the BonoboObject.
 *
 * This routine binds @object to @servant.  It establishes a one to
 * one mapping between the @object and the @servant.  Utility routines
 * are provided to go back and forth.  See bonobo_object_from_servant()
 * and bonobo_object_get_servant().
 *
 * This routine is used internally by bonobo_object_activate_servant().
 */
void
bonobo_object_bind_to_servant (BonoboObject *object, void *servant)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (servant != NULL);

	object->servant = servant;
	((BonoboObjectServant *)servant)->bonobo_object = object;
}

/**
 * bonobo_object_ref:
 * @object: A BonoboObject you want to ref-count
 *
 * increments the reference count for the aggregate BonoboObject.
 */
void
bonobo_object_ref (BonoboObject *object)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (object->priv->ao->ref_count > 0);

	object->priv->ao->ref_count++;
}

/**
 * bonobo_object_unref:
 * @object: A BonoboObject you want to unref.
 *
 * decrements the reference count for the aggregate BonoboObject.
 */
void
bonobo_object_unref (BonoboObject *object)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (object->priv->ao->ref_count > 0);

	if (object->priv->ao->ref_count == 1) {
		bonobo_object_destroy (object);
	} else {
		object->priv->ao->ref_count--;
	}
}

/**
 * bonobo_object_destroy
 * @object: A BonoboObject you want to destroy.
 *
 * This function is deprecated. Use bonobo_object_unref instead.
 * This is *not* like gtk_object_destroy, which can be done independently
 * of gtk_object_unref and has no effect on the ref count.
 *
 * Destroys an object.  Ignores all reference count.  Used when you want
 * to force the destruction of the composite object (use only if you know
 * what you are doing).
 */
void
bonobo_object_destroy (BonoboObject *object)
{
	GnomeAggregateObject *ao;
	GList *l;

	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (object->priv->ao->ref_count > 0);

	ao = object->priv->ao;

	ao->ref_count = 0;
	for (l = ao->objs; l; l = l->next){
		BonoboObject *o = l->data;

		gtk_signal_disconnect (GTK_OBJECT (o), o->priv->destroy_id);
		gtk_object_destroy (GTK_OBJECT (o));
	}

	g_list_free (ao->objs);
	g_free (ao);
}

static void
impl_Bonobo_Unknown_ref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object;

	object = bonobo_object_from_servant (servant);
	bonobo_object_ref (object);
}

static void
impl_Bonobo_Unknown_unref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object;

	object = bonobo_object_from_servant (servant);
	bonobo_object_unref (object);
}

static BonoboObject *
bonobo_object_get_local_interface_from_objref (BonoboObject *object,
					       CORBA_Object  interface)
{
	CORBA_Environment  ev;
	GList             *l;

	if (interface == CORBA_OBJECT_NIL)
		return NULL;

	CORBA_exception_init (&ev);

	for (l = object->priv->ao->objs; l; l = l->next) {
		BonoboObject *tryme = l->data;

		if (CORBA_Object_is_equivalent (interface, tryme->corba_objref, &ev)) {
			CORBA_exception_free (&ev);
			return tryme;
		}

		if (ev._major != CORBA_NO_EXCEPTION) {
			CORBA_exception_free (&ev);
			return NULL;
		}

	}

	CORBA_exception_free (&ev);

	return NULL;
}

/**
 * bonobo_object_query_local_interface:
 * @object: A #BonoboObject which is the aggregate of multiple objects.
 * @repo_id: The id of the interface being queried.
 *
 * Returns: A #BonoboObject for the requested interface.
 */
BonoboObject *
bonobo_object_query_local_interface (BonoboObject *object,
				     const char   *repo_id)
{
	CORBA_Environment  ev;
	BonoboObject      *retval;
	CORBA_Object       corba_retval;
	GtkType            type;
	GList             *l;

	g_return_val_if_fail (BONOBO_IS_OBJECT (object), NULL);

	retval       = NULL;
	corba_retval = CORBA_OBJECT_NIL;

	gtk_signal_emit (
		GTK_OBJECT (object), bonobo_object_signals [QUERY_INTERFACE],
		repo_id, &corba_retval);

	CORBA_exception_init (&ev);

	if (! CORBA_Object_is_nil (corba_retval, &ev)) {
		BonoboObject *local_interface;

		local_interface = bonobo_object_get_local_interface_from_objref (
			object, corba_retval);

		if (local_interface != NULL)
			bonobo_object_ref (object);

		return local_interface;
	}

	type = gtk_type_from_name (repo_id);

	/* Try looking at the gtk types */
	for (l = object->priv->ao->objs; l; l = l->next){
		BonoboObject *tryme = l->data;

		if ((type && gtk_type_is_a (GTK_OBJECT (tryme)->klass->type, type)) ||
#ifdef ORBIT_IMPLEMENTS_IS_A
		    CORBA_Object_is_a (tryme->corba_objref, (char *) repo_id, &ev)
#else
		    !strcmp (tryme->corba_objref->object_id, repoid)
#endif
			){
			retval = tryme;
			break;
		}
	}

	if (retval != NULL)
		bonobo_object_ref (object);

	CORBA_exception_free (&ev);

	return retval;
}

static CORBA_Object
impl_Bonobo_Unknown_query_interface (PortableServer_Servant  servant,
				     const CORBA_char       *repoid,
				     CORBA_Environment      *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboObject *local_interface;

	local_interface = bonobo_object_query_local_interface (
		object, repoid);

	if (local_interface == NULL)
		return CORBA_OBJECT_NIL;

	return CORBA_Object_duplicate (local_interface->corba_objref, ev);
}

/**
 * bonobo_object_get_epv:
 *
 */
POA_Bonobo_Unknown__epv *
bonobo_object_get_epv (void)
{
	POA_Bonobo_Unknown__epv *epv;

	epv = g_new0 (POA_Bonobo_Unknown__epv, 1);

	epv->ref             = impl_Bonobo_Unknown_ref;
	epv->unref           = impl_Bonobo_Unknown_unref;
	epv->query_interface = impl_Bonobo_Unknown_query_interface;

	return epv;
}

static void
init_object_corba_class (void)
{
	bonobo_object_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
}

static void
bonobo_object_object_destroy (GtkObject *object)
{
	BonoboObject *bonobo_object = BONOBO_OBJECT (object);
	void *servant = bonobo_object->servant;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (bonobo_object->corba_objref != CORBA_OBJECT_NIL)
		CORBA_Object_release (bonobo_object->corba_objref, &ev);

	if (servant){
		PortableServer_ObjectId *oid;

		oid = PortableServer_POA_servant_to_id(bonobo_poa(), servant, &ev);
		PortableServer_POA_deactivate_object (bonobo_poa (), oid, &ev);

		POA_Bonobo_Unknown__fini (servant, &ev);
		CORBA_free(oid);
	}
	CORBA_exception_free (&ev);

	g_free (bonobo_object->priv);
	bonobo_object_parent_class->destroy (object);
}

static void
bonobo_object_class_init (BonoboObjectClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_object_parent_class = gtk_type_class (gtk_object_get_type ());

	bonobo_object_signals [QUERY_INTERFACE] =
		gtk_signal_new ("query_interface",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(BonoboObjectClass,query_interface),
				gtk_marshal_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 2, GTK_TYPE_POINTER, GTK_TYPE_POINTER);
	bonobo_object_signals [SYSTEM_EXCEPTION] =
		gtk_signal_new ("system_exception",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(BonoboObjectClass,system_exception),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);
	bonobo_object_signals [OBJECT_GONE] =
		gtk_signal_new ("object_gone",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(BonoboObjectClass,object_gone),
				gtk_marshal_NONE__POINTER,
				GTK_TYPE_NONE, 1, GTK_TYPE_POINTER);


	gtk_object_class_add_signals (object_class, bonobo_object_signals, LAST_SIGNAL);

	object_class->destroy = bonobo_object_object_destroy;

	init_object_corba_class ();
}

static void
bonobo_object_usage_error (BonoboObject *object)
{
	g_error ("Aggregate bonobo_object member %p has been destroyed using gtk_object_* methods", object);
}

static void
bonobo_object_instance_init (GtkObject *gtk_object)
{
	BonoboObject *object = BONOBO_OBJECT (gtk_object);

	object->priv = g_new (BonoboObjectPrivate, 1);
	object->priv->destroy_id = gtk_signal_connect (
		gtk_object, "destroy", GTK_SIGNAL_FUNC (bonobo_object_usage_error),
		NULL);

	object->priv->ao = g_new0 (GnomeAggregateObject, 1);
	object->priv->ao->objs = g_list_append (object->priv->ao->objs, object);
	object->priv->ao->ref_count = 1;
}

/**
 * bonobo_object_get_type:
 *
 * Returns: the GtkType associated with the base BonoboObject class type.
 */
GtkType
bonobo_object_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboObject",
			sizeof (BonoboObject),
			sizeof (BonoboObjectClass),
			(GtkClassInitFunc) bonobo_object_class_init,
			(GtkObjectInitFunc) bonobo_object_instance_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_object_activate_servant:
 * @object: a BonoboObject
 * @servant: The servant to activate.
 *
 * This routine activates the @servant which is wrapped inside the
 * @object on the bonobo_poa (which is the default POA).
 *
 * Returns: The CORBA_Object that is wrapped by @object and whose
 * servant is @servant.  Might return CORBA_OBJECT_NIL on failure.
 */
CORBA_Object
bonobo_object_activate_servant (BonoboObject *object, void *servant)
{
	CORBA_Environment ev;
	CORBA_Object o;

	g_return_val_if_fail (BONOBO_IS_OBJECT (object), CORBA_OBJECT_NIL);
	g_return_val_if_fail (servant != NULL, CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &ev);

	CORBA_exception_free (&ev);

	if (o){
		object->corba_objref = o;
		bonobo_object_bind_to_servant (object, servant);
		return o;
	} else
		return CORBA_OBJECT_NIL;

}

/**
 * bonobo_object_construct:
 * @object: The GTK object server wrapper for the CORBA service.
 * @corba_object: the reference to the real CORBA object.
 *
 * Initializes the provided BonoboObject @object. This method is
 * usually invoked from the construct method for other Gtk-based CORBA
 * wrappers that derive from the Bonobo::Unknown interface
 *
 * Returns: the initialized BonoboObject.
 */
BonoboObject *
bonobo_object_construct (BonoboObject *object, CORBA_Object corba_object)
{
	g_return_val_if_fail (BONOBO_IS_OBJECT (object), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);

	object->corba_objref = corba_object;

	/* BonoboObjects are self-owned */
	GTK_OBJECT_UNSET_FLAGS (GTK_OBJECT (object), GTK_FLOATING);

	return object;
}

/**
 * bonobo_object_add_interface:
 * @object: The BonoboObject to which an interface is going to be added.
 * @newobj: The BonoboObject containing the new interface to be added.
 *
 * Adds the interfaces supported by @newobj to the list of interfaces
 * for @object.  This function adds the interfaces supported by
 * @newobj to the list of interfaces support by @object.
 */
void
bonobo_object_add_interface (BonoboObject *object, BonoboObject *newobj)
{
       GnomeAggregateObject *oldao;
       GList *l;

       if (object->priv->ao == newobj->priv->ao)
               return;

       if (newobj->corba_objref == CORBA_OBJECT_NIL)
	       g_warning ("Adding an interface with a NULL Corba objref");

       /*
	* Explanation:
	*   Bonobo Objects should not be assembled after they have been
	*   exposed, or we would be breaking the contract we have with
	*   the other side.
	*
	*   This check is not perfect, but might help some people.
	*/

       oldao = newobj->priv->ao;

       /* Merge the two AggregateObject lists */
       for (l = newobj->priv->ao->objs; l; l = l->next) {
               if (!g_list_find (object->priv->ao->objs, l->data)) {
                       object->priv->ao->objs = g_list_prepend (object->priv->ao->objs, l->data);
                       ((BonoboObject *)l->data)->priv->ao = object->priv->ao;
               }
       }

       g_list_free (oldao->objs);
       g_free (oldao);
}

/**
 * bonobo_object_query_interface:
 * @object: A BonoboObject to be queried for a given interface.
 * @repo_id: The name of the interface to be queried.
 *
 * Returns: The CORBA interface named @repo_id for @object.
 */
CORBA_Object
bonobo_object_query_interface (BonoboObject *object, const char *repo_id)
{
       CORBA_Environment ev;
       CORBA_Object retval;

       CORBA_exception_init(&ev);
       retval = Bonobo_Unknown_query_interface (object->corba_objref, (CORBA_char *)repo_id, &ev);
       if(ev._major != CORBA_NO_EXCEPTION)
               retval = CORBA_OBJECT_NIL;
       CORBA_exception_free (&ev);
       return retval;
}

/**
 * bonobo_object_corba_objref:
 * @object: A BonoboObject whose CORBA object is requested.
 *
 * Returns: The CORBA interface object for which @object is a wrapper.
 */
CORBA_Object
bonobo_object_corba_objref (BonoboObject *object)
{
	g_return_val_if_fail (BONOBO_IS_OBJECT (object), NULL);

	return object->corba_objref;
}

/**
 * bonobo_object_check_env:
 * @object: The object on which we operate
 * @ev: CORBA Environment to check
 *
 * This routine verifies the @ev environment for any fatal system
 * exceptions.  If a system exception occurs, the object raises a
 * "system_exception" signal.  The idea is that GtkObjects which are
 * used to wrap a CORBA interface can use this function to notify
 * the user if a fatal exception has occurred, causing the object
 * to become defunct.
 */
void
bonobo_object_check_env (BonoboObject *object, CORBA_Object obj, CORBA_Environment *ev)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (ev != NULL);

	if (ev->_major == CORBA_NO_EXCEPTION)
		return;

	if (ev->_major == CORBA_SYSTEM_EXCEPTION){
		gtk_signal_emit (
			GTK_OBJECT (object), bonobo_object_signals [SYSTEM_EXCEPTION], obj, ev);
	}
}

/**
 * gnome_unknown_ping:
 * @object: a CORBA object reference of type Bonobo::Unknown
 *
 * Pings the object @object using the ref/unref methods from Bonobo::Unknown.
 * You can use this one to see if a remote object has gone away.
 *
 * Returns: %TRUE if the Bonobo::Unknown @object is alive.
 */
gboolean
gnome_unknown_ping (Bonobo_Unknown object)
{
	CORBA_Environment ev;
	gboolean alive;

	g_return_val_if_fail (object != NULL, FALSE);

	alive = FALSE;
	CORBA_exception_init (&ev);
	Bonobo_Unknown_ref (object, &ev);
	if (ev._major == CORBA_NO_EXCEPTION) {
		Bonobo_Unknown_unref (object, &ev);
		if (ev._major == CORBA_NO_EXCEPTION)
			alive = TRUE;
	}
	CORBA_exception_free (&ev);

	return alive;
}

/**
 * bonobo_object_from_servant:
 * @servant: A Servant that implements the Bonobo::Unknown interface
 *
 * This wraps the servant @servant in a BonoboObject.
 */
BonoboObject *
bonobo_object_new_from_servant (void *servant)
{
	BonoboObject *object;
	CORBA_Object corba_object;

	g_return_val_if_fail (servant != NULL, NULL);

	object = gtk_type_new (bonobo_object_get_type ());
	if (object == NULL)
		return NULL;

	corba_object = bonobo_object_activate_servant (object, servant);
	bonobo_object_construct (object, corba_object);

	return object;
}
