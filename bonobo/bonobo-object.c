/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-object.c: Bonobo Unknown interface base implementation
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999,2001 Ximian, Inc.
 */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <gobject/gsignal.h>
#include <gobject/gmarshal.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-shlib-factory.h>
#include <bonobo/bonobo-running-context.h>
#include <bonobo/bonobo-marshal.h>
#include <bonobo/bonobo-types.h>
#include <bonobo/bonobo-shutdown.h>

/* Some simple tracking - always on */
static GMutex *bonobo_total_aggregates_lock = NULL;
static glong   bonobo_total_aggregates      = 0;

#ifdef BONOBO_OBJECT_DEBUG
#	define BONOBO_REF_HOOKS
#endif

/* Define to debug user aggregate merging code */
#undef BONOBO_AGGREGATE_DEBUG

/* NB. for a quicker debugging experience define this */
/* # define BONOBO_REF_HOOKS */

/* You almost certainly don't want this */
#undef BONOBO_LIFECYCLE_DEBUG

#ifdef BONOBO_REF_HOOKS
typedef struct {
	const char *fn;
	gboolean    ref;
	int         line;
} BonoboDebugRefData;
#endif

typedef struct {
	int      ref_count;
	gboolean immortal;
	GList   *objs;
#ifdef BONOBO_REF_HOOKS
	GList   *refs;
	int      destroyed;
#endif
} BonoboAggregateObject;

struct _BonoboObjectPrivate {
	BonoboAggregateObject *ao;
};

enum {
	DESTROY,
	SYSTEM_EXCEPTION,
	LAST_SIGNAL
};

static guint bonobo_object_signals [LAST_SIGNAL];
static GObjectClass *bonobo_object_parent_class;

#ifdef BONOBO_REF_HOOKS

static GHashTable *living_ao_ht = NULL;

static void
bonobo_debug_print (char *name, char *fmt, ...)
{
	va_list args;
           
	va_start (args, fmt);
	
	printf ("[%06d]:%-15s ", getpid (), name); 
	vprintf (fmt, args);
	printf ("\n"); 

	va_end (args);
}
#endif /* BONOBO_REF_HOOKS */

/* Do not use this function, it is not what you want; see unref */
static void
bonobo_object_destroy (BonoboAggregateObject *ao)
{
	GList *l;

	g_return_if_fail (ao->ref_count > 0);

	for (l = ao->objs; l; l = l->next) {
		GObject *o = l->data;

		if (o->ref_count >= 1) {
			g_object_ref (o);
			g_signal_emit (o, bonobo_object_signals [DESTROY], 0);
			g_object_unref (o);
		} else
			g_warning ("Serious ref-counting error [%p]", o);
	}
#ifdef BONOBO_REF_HOOKS
	ao->destroyed = TRUE;
#endif
}

static void
bonobo_object_corba_deactivate (BonoboObject *object)
{
	CORBA_Environment        ev;
	PortableServer_ObjectId *oid;

#ifdef BONOBO_LIFECYCLE_DEBUG
	g_warning ("BonoboObject corba deactivate %p", object);
#endif

	g_assert (object->priv->ao == NULL);

	CORBA_exception_init (&ev);

	if (object->corba_objref != CORBA_OBJECT_NIL) {
		bonobo_running_context_remove_object (object->corba_objref);
		CORBA_Object_release (object->corba_objref, &ev);
		object->corba_objref = CORBA_OBJECT_NIL;
	}

	oid = PortableServer_POA_servant_to_id (
		bonobo_poa(), &object->servant, &ev);
	PortableServer_POA_deactivate_object (bonobo_poa (), oid, &ev);

#if 0
	{ /* Used to do this, probably not correct in fact */
		BonoboObjectClass       *klass;
		klass = (BonoboObjectClass *) G_OBJECT_GET_CLASS (object);
		
		if (klass->poa_fini_fn)
			klass->poa_fini_fn (&object->servant, &ev);
		else /* Actually quicker and nicer */
			PortableServer_ServantBase__fini (&object->servant, &ev);
	}
#endif
	
	CORBA_free (oid);
	CORBA_exception_free (&ev);
}

/*
 * bonobo_object_finalize_internal:
 * 
 * This method splits apart the aggregate object, so that each
 * GObject can be finalized individualy.
 *
 * Note that since the (embedded) servant keeps a ref on the
 * GObject, it won't neccessarily be finalized through this
 * routine, but from the poa later.
 */
static void
bonobo_object_finalize_internal (BonoboAggregateObject *ao)
{
	GList *l;

	g_return_if_fail (ao->ref_count == 0);

	for (l = ao->objs; l; l = l->next) {
		GObject *o = G_OBJECT (l->data);

		if (!o)
			g_error ("Serious bonobo object corruption");
		else {
			g_assert (BONOBO_OBJECT (o)->priv->ao != NULL);
#ifdef BONOBO_REF_HOOKS
			g_assert (BONOBO_OBJECT (o)->priv->ao->destroyed);

			bonobo_debug_print ("finalize", 
					    "[%p] %-20s corba_objref=[%p]"
					    " g_ref_count=%d", o,
					    G_OBJECT_TYPE_NAME (o),
					    BONOBO_OBJECT (o)->corba_objref,
					    G_OBJECT (o)->ref_count);
#endif

			/*
			 * Disconnect the GTK+ object from the aggregate object
			 * and unref it so that it is possibly finalized ---
			 * other parts of GTK+ may still have references to it.
			 *
			 * The GTK+ object was already destroy()ed in
			 * bonobo_object_destroy().
			 */

			BONOBO_OBJECT (o)->priv->ao = NULL;

			bonobo_object_corba_deactivate (BONOBO_OBJECT (o));

			g_object_unref (o);
#ifdef BONOBO_LIFECYCLE_DEBUG
			g_warning ("Done finalize internal on %p", o);
#endif
		}
	}

	g_list_free (ao->objs);
	ao->objs = NULL;

#ifdef BONOBO_REF_HOOKS
	for (l = ao->refs; l; l = l->next)
		g_free (l->data);
	g_list_free (ao->refs);
#endif

	g_free (ao);

	/* Some simple debugging - count aggregate free */
	LINC_MUTEX_LOCK   (bonobo_total_aggregates_lock);
	bonobo_total_aggregates--;
	LINC_MUTEX_UNLOCK (bonobo_total_aggregates_lock);
}


/*
 * bonobo_object_finalize_servant:
 * 
 * This routine is called from either an object de-activation
 * or from the poa. It is called to signal the fact that finaly
 * the object is no longer exposed to the world and thus we
 * can safely loose it's GObject reference, and thus de-allocate
 * the memory associated with it.
 */
static void
bonobo_object_finalize_servant (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object(servant);

#ifdef BONOBO_LIFECYCLE_DEBUG
	g_warning ("BonoboObject Servant finalize %p", object);
#endif

	g_object_unref (G_OBJECT (object));
}

#ifndef bonobo_object_ref
/**
 * bonobo_object_ref:
 * @object: A BonoboObject you want to ref-count
 *
 * Increments the reference count for the aggregate BonoboObject.
 */
void
bonobo_object_ref (BonoboObject *object)
{
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (object->priv->ao->ref_count > 0);

#ifdef BONOBO_REF_HOOKS
	bonobo_object_trace_refs (object, "local", 0, TRUE);
#else
	object->priv->ao->ref_count++;
#endif
}
#endif /* bonobo_object_ref */


#ifndef bonobo_object_unref
/**
 * bonobo_object_unref:
 * @object: A BonoboObject you want to unref.
 *
 * Decrements the reference count for the aggregate BonoboObject.
 */
void
bonobo_object_unref (BonoboObject *object)
{
#ifdef BONOBO_REF_HOOKS
	bonobo_object_trace_refs (object, "local", 0, FALSE);
#else
	BonoboAggregateObject *ao;

	g_return_if_fail (BONOBO_IS_OBJECT (object));

	ao = object->priv->ao;
	g_return_if_fail (ao != NULL);
	g_return_if_fail (ao->ref_count > 0);

	if (ao->immortal)
		ao->ref_count--;
	else {
		if (ao->ref_count == 1)
			bonobo_object_destroy (ao);

		ao->ref_count--;
		
		if (ao->ref_count == 0)
			bonobo_object_finalize_internal (ao);
	}
#endif /* BONOBO_REF_HOOKS */
}
#endif /* bonobo_object_unref */

void
bonobo_object_trace_refs (BonoboObject *object,
			  const char   *fn,
			  int           line,
			  gboolean      ref)
{
#ifdef BONOBO_REF_HOOKS
	BonoboAggregateObject *ao;
	BonoboDebugRefData *descr;
	
	g_return_if_fail (BONOBO_IS_OBJECT (object));
	ao = object->priv->ao;
	g_return_if_fail (ao != NULL);

	descr  = g_new (BonoboDebugRefData, 1);
	ao->refs = g_list_prepend (ao->refs, descr);
	descr->fn = fn;
	descr->ref = ref;
	descr->line = line;

	if (ref) {
		g_return_if_fail (ao->ref_count > 0);
		
		object->priv->ao->ref_count++;
		
		bonobo_debug_print ("ref", "[%p]:[%p]:%s to %d at %s:%d", 
			object, ao,
		        G_OBJECT_TYPE_NAME (object),
			ao->ref_count, fn, line);

	} else { /* unref */
		bonobo_debug_print ("unref", "[%p]:[%p]:%s from %d at %s:%d", 
			object, ao,
			G_OBJECT_TYPE_NAME (object),
			ao->ref_count, fn, line);

		g_return_if_fail (ao->ref_count > 0);

		if (ao->immortal) {
			ao->ref_count--;
			bonobo_debug_print ("unusual", "immortal object");
		} else {
			if (ao->ref_count == 1) {
				bonobo_object_destroy (ao);
				
				g_return_if_fail (ao->ref_count > 0);
			}
			
			/*
			 * If this blows it is likely some loony used
			 * g_object_unref somewhere instead of
			 * bonobo_object_unref, send them my regards.
			 */
			g_assert (object->priv->ao == ao);
			
			ao->ref_count--;
			
			if (ao->ref_count == 0) {
				
				g_assert (g_hash_table_lookup (living_ao_ht, ao) == ao);
				g_hash_table_remove (living_ao_ht, ao);
				
				bonobo_object_finalize_internal (ao);
				
			} else if (ao->ref_count < 0) {
				bonobo_debug_print ("unusual", 
						    "[%p] already finalized", ao);
			}
		}
	}
#else
	if (ref)
		bonobo_object_ref (object);
	else
		bonobo_object_unref (object);
#endif
}

static void
impl_Bonobo_Unknown_ref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object;

	object = bonobo_object_from_servant (servant);

#if defined(BONOBO_REF_HOOKS) && !defined(bonobo_object_ref)
	bonobo_object_trace_refs (object, "remote", 0, TRUE);
#else
	bonobo_object_ref (object);
#endif
}

void
bonobo_object_set_immortal (BonoboObject *object,
			    gboolean      immortal)
{
	BonoboAggregateObject *ao;

	g_return_if_fail (BONOBO_IS_OBJECT (object));
	g_return_if_fail (object->priv != NULL);
	g_return_if_fail (object->priv->ao != NULL);

	ao = object->priv->ao;

	ao->immortal = immortal;
}

/**
 * bonobo_object_dup_ref:
 * @object: a Bonobo_Unknown corba object
 * @opt_ev: an optional exception environment
 * 
 *   This function returns a duplicated CORBA Object reference;
 * it also bumps the ref count on the object. This is ideal to
 * use in any method returning a Bonobo_Object in a CORBA impl.
 * If object is CORBA_OBJECT_NIL it is returned unaffected.
 * 
 * Return value: duplicated & ref'd corba object reference.
 **/
Bonobo_Unknown
bonobo_object_dup_ref (Bonobo_Unknown     object,
		       CORBA_Environment *opt_ev)
{
	Bonobo_Unknown    ans;
	CORBA_Environment  *ev, temp_ev;
       
	if (object == CORBA_OBJECT_NIL)
		return CORBA_OBJECT_NIL;

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	Bonobo_Unknown_ref (object, ev);
	ans = CORBA_Object_duplicate (object, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

	return ans;
}

/**
 * bonobo_object_release_unref:
 * @object: a Bonobo_Unknown corba object
 * @opt_ev: an optional exception environment
 * 
 *   This function releases a CORBA Object reference;
 * it also decrements the ref count on the bonobo object.
 * This is the converse of bonobo_object_dup_ref. We
 * tolerate object == CORBA_OBJECT_NIL silently.
 **/
void
bonobo_object_release_unref (Bonobo_Unknown     object,
			     CORBA_Environment *opt_ev)
{
	CORBA_Environment  *ev, temp_ev;

	if (object == CORBA_OBJECT_NIL)
		return;

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	Bonobo_Unknown_unref (object, ev);
	CORBA_Object_release (object, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);
}

static void
impl_Bonobo_Unknown_unref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object;

	object = bonobo_object_from_servant (servant);

#if defined(BONOBO_REF_HOOKS) && !defined(bonobo_object_unref)
	bonobo_object_trace_refs (object, "remote", 0, FALSE);
#else
	bonobo_object_unref (object);
#endif
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

		if (BONOBO_EX (&ev)) {
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
	BonoboObjectClass *klass;
	CORBA_Object       corba_retval = CORBA_OBJECT_NIL;
	GList             *l;

	g_return_val_if_fail (BONOBO_IS_OBJECT (object), NULL);

	corba_retval = CORBA_OBJECT_NIL;

	klass = BONOBO_OBJECT_GET_CLASS (object);
	if (klass->query_interface)
		corba_retval = klass->query_interface (object, repo_id);

	CORBA_exception_init (&ev);

	if (corba_retval != CORBA_OBJECT_NIL) {
		BonoboObject *local_interface;

		local_interface = bonobo_object_get_local_interface_from_objref (
			object, corba_retval);

		if (local_interface != NULL)
			bonobo_object_ref (object);

		return local_interface;
	}

	for (l = object->priv->ao->objs; l; l = l->next){
		BonoboObject *tryme = l->data;

		if (CORBA_Object_is_a (tryme->corba_objref, (char *) repo_id, &ev)) {
			if (BONOBO_EX (&ev)) {
				CORBA_exception_free (&ev);
				continue;
			}
			
			bonobo_object_ref (object);

			return tryme;
		}
	}

	CORBA_exception_free (&ev);

	return NULL;
}

static CORBA_Object
impl_Bonobo_Unknown_queryInterface (PortableServer_Servant  servant,
				    const CORBA_char       *repoid,
				    CORBA_Environment      *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboObject *local_interface;

	local_interface = bonobo_object_query_local_interface (
		object, repoid);

#ifdef BONOBO_REF_HOOKS
	bonobo_debug_print ("query-interface", 
			    "[%p]:[%p]:%s repoid=%s", 
			    object, object->priv->ao,
			    G_OBJECT_TYPE_NAME (object),
			    repoid);
#endif

	if (local_interface == NULL)
		return CORBA_OBJECT_NIL;

	return CORBA_Object_duplicate (local_interface->corba_objref, ev);
}

static void
bonobo_object_epv_init (POA_Bonobo_Unknown__epv *epv)
{
	epv->ref            = impl_Bonobo_Unknown_ref;
	epv->unref          = impl_Bonobo_Unknown_unref;
	epv->queryInterface = impl_Bonobo_Unknown_queryInterface;
}

static void
bonobo_object_finalize_gobject (GObject *gobject)
{
	BonoboObject *object = (BonoboObject *) gobject;

#ifdef BONOBO_LIFECYCLE_DEBUG
	g_warning ("Bonobo Object finalize GObject %p", gobject);
#endif

	g_free (object->priv);

	bonobo_object_parent_class->finalize (gobject);
}

static void
bonobo_object_dummy_destroy (BonoboObject *object)
{
	/* Just to make chaining possibly cleaner */
}

static void
bonobo_object_class_init (BonoboObjectClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	/* Ensure that the signature checking is going to work */
	g_assert (sizeof (POA_Bonobo_Unknown) == sizeof (gpointer) * 2);
	g_assert (sizeof (BonoboObjectHeader) * 2 == sizeof (BonoboObject));

	bonobo_object_parent_class = g_type_class_peek_parent (klass);

	bonobo_object_signals [DESTROY] =
		g_signal_new ("destroy",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboObjectClass,destroy),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__VOID,
			      G_TYPE_NONE, 0);
	bonobo_object_signals [SYSTEM_EXCEPTION] =
		g_signal_new ("system_exception",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (BonoboObjectClass,system_exception),
			      NULL, NULL,
			      bonobo_marshal_VOID__OBJECT_BOXED,
			      G_TYPE_NONE, 2,
			      BONOBO_OBJECT_TYPE | G_SIGNAL_TYPE_STATIC_SCOPE,
			      BONOBO_TYPE_CORBA_EXCEPTION);

	klass->destroy = bonobo_object_dummy_destroy;

	object_class->finalize = bonobo_object_finalize_gobject;
}


static void
do_corba_setup (BonoboObject      *object,
		BonoboObjectClass *klass)
{
	CORBA_Object obj;
	CORBA_Environment ev;
	BonoboObjectClass *xklass;

	CORBA_exception_init (&ev);

	/* Setup the servant structure */
	object->servant._private = NULL;
	object->servant.vepv     = klass->vepv;

	/* Initialize the servant structure with our POA__init fn */
	{
		for (xklass = klass; xklass && !xklass->poa_init_fn;)
			xklass = g_type_class_peek_parent (xklass);
		if (!xklass || !xklass->epv_struct_offset) {
			/*   Also, people using BONOBO_TYPE_FUNC instead of
			 * BONOBO_TYPE_FUNC_FULL might see this; you need
			 * to tell it about the CORBA interface you're
			 * implementing - of course */
			g_warning ("It looks like you used g_type_unique "
				   "instead of b_type_unique on type '%s'",
				   G_OBJECT_CLASS_NAME (klass));
			return;
		}
		xklass->poa_init_fn ((PortableServer_Servant) &object->servant, &ev);
		if (BONOBO_EX (&ev)) {
			g_warning ("Exception initializing servant '%s'",
				   bonobo_exception_get_text (&ev));
			return;
		}
	}

	/* Activate - but we don't need the ObjectId */
	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), &object->servant, &ev));

	if (BONOBO_EX (&ev)) {
		g_warning ("Exception '%s' activating object",
			   bonobo_exception_get_text (&ev));
		return;
	}

	/* Instantiate a CORBA_Object reference for the servant */
	obj = PortableServer_POA_servant_to_reference (
		bonobo_poa (), &object->servant, &ev);

	if (BONOBO_EX (&ev)) {
		g_warning ("Exception '%s' getting reference for servant",
			   bonobo_exception_get_text (&ev));
		return;
	}

	object->corba_objref = obj;
	bonobo_running_context_add_object (obj);

	CORBA_exception_free (&ev);
}

static void
bonobo_object_instance_init (GObject    *g_object,
			     GTypeClass *klass)
{
	BonoboObject *object = BONOBO_OBJECT (g_object);
	BonoboAggregateObject *ao;

#ifdef BONOBO_OBJECT_DEBUG
	g_warning ("bonobo_object_instance init '%s' '%s' -> %p",
		   G_OBJECT_TYPE_NAME (g_object),
		   G_OBJECT_CLASS_NAME (klass), object);
#endif
	/* Some simple debugging - count aggregate allocate */
	LINC_MUTEX_LOCK   (bonobo_total_aggregates_lock);
	bonobo_total_aggregates++;
	LINC_MUTEX_UNLOCK (bonobo_total_aggregates_lock);

	/* Setup aggregate */
	ao            = g_new0 (BonoboAggregateObject, 1);
	ao->objs      = g_list_append (ao->objs, object);
	ao->ref_count = 1;

	/* Setup Private fields */
	object->priv = g_new (BonoboObjectPrivate, 1);
	object->priv->ao = ao;

	/* Setup signatures */
	object->object_signature  = BONOBO_OBJECT_SIGNATURE;
	object->servant_signature = BONOBO_SERVANT_SIGNATURE;

	/* Though this make look strange, destruction of this object
	   can only occur when the servant is deactivated by the poa.
	   The poa maintains its own ref count over method invocations
	   and delays finalization which happens only after:
	   bonobo_object_finalize_servant: is invoked */
	g_object_ref (g_object);

#ifdef BONOBO_REF_HOOKS
	{
		bonobo_debug_print ("create", "[%p]:[%p]:%s to %d", object, ao,
				    g_type_name (G_TYPE_FROM_CLASS (klass)),
				    ao->ref_count);

		g_assert (g_hash_table_lookup (living_ao_ht, ao) == NULL);
		g_hash_table_insert (living_ao_ht, ao, ao);
	}
#endif

	do_corba_setup (object, BONOBO_OBJECT_CLASS (klass));
}

/**
 * bonobo_object_get_type:
 *
 * Returns: the GType associated with the base BonoboObject class type.
 */
GType
bonobo_object_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (BonoboObjectClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) bonobo_object_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboObject),
			0, /* n_preallocs */
			(GInstanceInitFunc) bonobo_object_instance_init
		};
		
		type = g_type_register_static (G_TYPE_OBJECT, "BonoboObject",
					       &info, 0);

#ifdef BONOBO_REF_HOOKS
		living_ao_ht = g_hash_table_new (NULL, NULL);
#endif
	}

	return type;
}

#ifdef BONOBO_REF_HOOKS
static void
bonobo_ao_debug_foreach (gpointer key, gpointer value, gpointer user_data)
{
	BonoboAggregateObject *ao = value;
	GList *l;

	g_return_if_fail (ao != NULL);

	bonobo_debug_print ("object-status", 
			    "[%p] %-20s ref_count=%d, interfaces=%d", ao, "",
			    ao->ref_count, g_list_length (ao->objs));
		
	for (l = ao->objs; l; l = l->next) {
		BonoboObject *object = BONOBO_OBJECT (l->data);
		
		bonobo_debug_print ("", "[%p] %-20s corba_objref=[%p]"
				    " g_ref_count=%d", object,
				    G_OBJECT_TYPE_NAME (object),
				    object->corba_objref,
				    G_OBJECT (object)->ref_count);
	}

	l = g_list_last (ao->refs);

	if (l)
		bonobo_debug_print ("referencing" ,"");

	for (; l; l = l->prev) {
		BonoboDebugRefData *descr = l->data;

		bonobo_debug_print ("", "%-7s - %s:%d", 
				    descr->ref ? "ref" : "unref",
				    descr->fn, descr->line);
	}
}
#endif

void
bonobo_object_shutdown (void)
{
#ifdef BONOBO_REF_HOOKS
	
	bonobo_debug_print ("shutdown-start", 
		"-------------------------------------------------");

	if (living_ao_ht)
		g_hash_table_foreach (living_ao_ht,
				      bonobo_ao_debug_foreach, NULL);

	bonobo_debug_print ("living-objects",
			    "living bonobo objects count = %d",
			    g_hash_table_size (living_ao_ht));

	bonobo_debug_print ("shutdown-end", 
		"-------------------------------------------------");
#endif
	if (bonobo_total_aggregates > 0)
		g_warning ("Leaked %ld bonobo objects total",
			   bonobo_total_aggregates);
}

void
bonobo_object_init (void)
{
	bonobo_total_aggregates_lock = linc_mutex_new ();
}

/**
 * bonobo_object_add_interface:
 * @object: The BonoboObject to which an interface is going to be added.
 * @newobj: The BonoboObject containing the new interface to be added.
 *
 * Adds the interfaces supported by @newobj to the list of interfaces
 * for @object.  This function adds the interfaces supported by
 * @newobj to the list of interfaces support by @object. It should never
 * be used when the object has been exposed to the world. This is a firm
 * part of the contract.
 */
void
bonobo_object_add_interface (BonoboObject *object, BonoboObject *newobj)
{
       BonoboAggregateObject *oldao, *ao;
       GList *l;

       g_return_if_fail (object->priv->ao->ref_count > 0);
       g_return_if_fail (newobj->priv->ao->ref_count > 0);

       if (object->priv->ao == newobj->priv->ao)
               return;

       if (newobj->corba_objref == CORBA_OBJECT_NIL)
	       g_warning ("Adding an interface with a NULL Corba objref");

       /*
	* Explanation:
	*   Bonobo Objects should not be assembled after they have been
	*   exposed, or we would be breaking the contract we have with
	*   the other side.
	*/

       ao = object->priv->ao;
       oldao = newobj->priv->ao;
       ao->ref_count = ao->ref_count + oldao->ref_count - 1;

#ifdef BONOBO_REF_HOOKS		
       bonobo_debug_print ("add_interface", 
			   "[%p]:[%p]:%s to [%p]:[%p]:%s ref_count=%d", 
			   object, object->priv->ao,
			   G_OBJECT_TYPE_NAME (object),
			   newobj, newobj->priv->ao,
			   G_OBJECT_TYPE_NAME (newobj),
			   ao->ref_count);
#endif

       /* Merge the two AggregateObject lists */
       for (l = oldao->objs; l; l = l->next) {
	       BonoboObject *new_if = l->data;

#ifdef BONOBO_AGGREGATE_DEBUG
	       GList *i;
	       CORBA_Environment ev;
	       CORBA_char *new_id;

	       CORBA_exception_init (&ev);

	       new_id = ORBit_small_get_type_id (new_if->corba_objref, &ev);

	       for (i = ao->objs; i; i = i->next) {
		       BonoboObject *old_if = i->data;

		       if (old_if == new_if)
			       g_error ("attempting to merge identical "
					"interfaces [%p]", new_if);
		       else {
			       CORBA_char *old_id;

			       old_id = ORBit_small_get_type_id (old_if->corba_objref, &ev);
			       
			       if (!strcmp (new_id, old_id))
				       g_error ("Aggregating two BonoboObject that implement "
						"the same interface '%s' [%p]", new_id, new_if);
			       CORBA_free (old_id);
		       }
	       }

	       CORBA_free (new_id);
	       CORBA_exception_free (&ev);
#endif

	       ao->objs = g_list_prepend (ao->objs, new_if);
	       new_if->priv->ao = ao;
       }

       g_assert (newobj->priv->ao == ao);

#ifdef BONOBO_REF_HOOKS
       g_assert (g_hash_table_lookup (living_ao_ht, oldao) == oldao);
       g_hash_table_remove (living_ao_ht, oldao);
       ao->refs = g_list_concat (ao->refs, oldao->refs);
#endif

       g_list_free (oldao->objs);
       g_free (oldao);

       /* Some simple debugging - count aggregate free */
       LINC_MUTEX_LOCK   (bonobo_total_aggregates_lock);
       bonobo_total_aggregates--;
       LINC_MUTEX_UNLOCK (bonobo_total_aggregates_lock);
}

/**
 * bonobo_object_query_interface:
 * @object: A BonoboObject to be queried for a given interface.
 * @repo_id: The name of the interface to be queried.
 * @opt_ev: optional exception environment
 *
 * Returns: The CORBA interface named @repo_id for @object.
 */
CORBA_Object
bonobo_object_query_interface (BonoboObject      *object,
			       const char        *repo_id,
			       CORBA_Environment *opt_ev)
{
	CORBA_Object retval;
	CORBA_Environment  *ev, temp_ev;
       
	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	retval = Bonobo_Unknown_queryInterface (
		object->corba_objref, repo_id, ev);

	if (BONOBO_EX (ev))
		retval = CORBA_OBJECT_NIL;

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

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
 * "system_exception" signal.  The idea is that GObjects which are
 * used to wrap a CORBA interface can use this function to notify
 * the user if a fatal exception has occurred, causing the object
 * to become defunct.
 */
void
bonobo_object_check_env (BonoboObject      *object,
			 CORBA_Object       obj,
			 CORBA_Environment *ev)
{
	g_return_if_fail (ev != NULL);
	g_return_if_fail (BONOBO_IS_OBJECT (object));

	if (!BONOBO_EX (ev))
		return;

	if (ev->_major == CORBA_SYSTEM_EXCEPTION)
		g_signal_emit (
			G_OBJECT (object),
			bonobo_object_signals [SYSTEM_EXCEPTION],
			0, obj, ev);
}

/**
 * bonobo_unknown_ping:
 * @object: a CORBA object reference of type Bonobo::Unknown
 * @opt_ev: optional exception environment
 *
 * Pings the object @object using the ref/unref methods from Bonobo::Unknown.
 * You can use this one to see if a remote object has gone away.
 *
 * Returns: %TRUE if the Bonobo::Unknown @object is alive.
 */
gboolean
bonobo_unknown_ping (Bonobo_Unknown     object,
		     CORBA_Environment *opt_ev)
{
	gboolean alive;
	CORBA_Environment  *ev, temp_ev;
       
	g_return_val_if_fail (object != NULL, FALSE);

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	alive = FALSE;

	Bonobo_Unknown_ref (object, ev);
	if (!BONOBO_EX (ev)) {
		Bonobo_Unknown_unref (object, ev);
		if (!BONOBO_EX (ev))
			alive = TRUE;
	}

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

	return alive;
}

void
bonobo_object_dump_interfaces (BonoboObject *object)
{
	BonoboAggregateObject *ao;
	GList                 *l;
	CORBA_Environment      ev;

	g_return_if_fail (BONOBO_IS_OBJECT (object));

	ao = object->priv->ao;
	
	CORBA_exception_init (&ev);

	fprintf (stderr, "references %d\n", ao->ref_count);
	for (l = ao->objs; l; l = l->next) {
		BonoboObject *o = l->data;
		
		g_return_if_fail (BONOBO_IS_OBJECT (o));

		if (o->corba_objref != CORBA_OBJECT_NIL) {
			CORBA_char   *type_id;

			type_id = ORBit_small_get_type_id (o->corba_objref, &ev);
			fprintf (stderr, "I/F: '%s'\n", type_id);
			CORBA_free (type_id);
		} else
			fprintf (stderr, "I/F: NIL error\n");
	}

	CORBA_exception_free (&ev);
}

static gboolean
idle_unref_fn (BonoboObject *object)
{
	bonobo_object_unref (object);

	return FALSE;
}

void
bonobo_object_idle_unref (BonoboObject *object)
{
	g_idle_add ((GSourceFunc) idle_unref_fn, object);
}

static void
unref_list (GSList *l)
{
	for (; l; l = l->next)
		bonobo_object_unref (l->data);
}

/**
 * bonobo_object_list_unref_all:
 * @list: A list of BonoboObjects *s
 * 
 *  This routine unrefs all valid objects in
 * the list and then removes them from @list if
 * they have not already been so removed.
 **/
void
bonobo_object_list_unref_all (GList **list)
{
	GList *l;
	GSList *unrefs = NULL, *u;

	g_return_if_fail (list != NULL);

	for (l = *list; l; l = l->next) {
		if (l->data && !BONOBO_IS_OBJECT (l->data))
			g_warning ("Non object in unref list");
		else if (l->data)
			unrefs = g_slist_prepend (unrefs, l->data);
	}

	unref_list (unrefs);

	for (u = unrefs; u; u = u->next)
		*list = g_list_remove (*list, u->data);

	g_slist_free (unrefs);
}

/**
 * bonobo_object_list_unref_all:
 * @list: A list of BonoboObjects *s
 * 
 *  This routine unrefs all valid objects in
 * the list and then removes them from @list if
 * they have not already been so removed.
 **/
void
bonobo_object_slist_unref_all (GSList **list)
{
	GSList *l;
	GSList *unrefs = NULL, *u;

	g_return_if_fail (list != NULL);

	for (l = *list; l; l = l->next) {
		if (l->data && !BONOBO_IS_OBJECT (l->data))
			g_warning ("Non object in unref list");
		else if (l->data)
			unrefs = g_slist_prepend (unrefs, l->data);
	}

	unref_list (unrefs);

	for (u = unrefs; u; u = u->next)
		*list = g_slist_remove (*list, u->data);

	g_slist_free (unrefs);
}

/**
 * bonobo_object:
 * @p: a pointer to something
 * 
 * This function can be passed a BonoboObject * or a
 * PortableServer_Servant, and it will return a BonoboObject *.
 * 
 * Return value: a BonoboObject or NULL on error.
 **/
BonoboObject *
bonobo_object (gpointer p)
{
	BonoboObjectHeader *header;

	if (!p)
		return NULL;

	header = (BonoboObjectHeader *) p;

	if (header->object_signature == BONOBO_OBJECT_SIGNATURE)
		return (BonoboObject *) p;

	else if (header->object_signature == BONOBO_SERVANT_SIGNATURE)
		return (BonoboObject *)(((guchar *) header) -
					BONOBO_OBJECT_HEADER_SIZE);

	g_warning ("Serious servant -> object mapping error '%p'", p);

	return NULL;
}

/**
 * bonobo_type_setup:
 * @type: The type to initialize
 * @init_fn: the POA_init function for the CORBA interface or NULL
 * @fini_fn: NULL or a custom POA free fn.
 * @epv_struct_offset: the offset in the class structure where the epv is or 0
 * 
 *   This function initializes a type derived from BonoboObject, such that
 * when you instantiate a new object of this type with g_type_new the
 * CORBA object will be correctly created and embedded.
 * 
 * Return value: TRUE on success, FALSE on error.
 **/
gboolean
bonobo_type_setup (GType             type,
		   BonoboObjectPOAFn init_fn,
		   BonoboObjectPOAFn fini_fn,
		   int               epv_struct_offset)
{
	GType       p, b_type;
	int           depth;
	BonoboObjectClass *klass;
	gpointer     *vepv;

	/* Setup our class data */
	klass = g_type_class_ref (type);
	klass->epv_struct_offset = epv_struct_offset;
	klass->poa_init_fn       = init_fn;
	klass->poa_fini_fn       = fini_fn;

	/* Calculate how far down the tree we are in epvs */
	b_type = bonobo_object_get_type ();
	for (depth = 0, p = type; p && p != b_type;
	     p = g_type_parent (p)) {
		BonoboObjectClass *xklass;

		xklass = g_type_class_peek (p);

		if (xklass->epv_struct_offset)
			depth++;
	}
	if (!p) {
		g_warning ("Trying to inherit '%s' from a BonoboObject, but "
			   "no BonoboObject in the ancestory",
			   g_type_name (type));
		return FALSE;
	}

#ifdef BONOBO_OBJECT_DEBUG
	g_warning ("We are at depth %d with type '%s'",
		   depth, g_type_name (type));
#endif

	/* Setup the Unknown epv */
	bonobo_object_epv_init (&klass->epv);
	klass->epv._private = NULL;

	klass->base_epv._private = NULL;
	klass->base_epv.finalize = bonobo_object_finalize_servant;
	klass->base_epv.default_POA = NULL;

	vepv = g_new0 (gpointer, depth + 2);
	klass->vepv = (POA_Bonobo_Unknown__vepv *) vepv;
	klass->vepv->_base_epv = &klass->base_epv;
	klass->vepv->Bonobo_Unknown_epv = &klass->epv;

	/* Build our EPV */
	if (depth > 0) {
		int i;

		for (p = type, i = depth; i > 0;) {
			BonoboObjectClass *xklass;

			xklass = g_type_class_peek (p);

			if (xklass->epv_struct_offset) {
				vepv [i + 1] = ((guchar *)klass) +
					xklass->epv_struct_offset;
				i--;
			}

			p = g_type_parent (p);
		}
	}

	return TRUE;
}

/**
 * bonobo_type_unique:
 * @parent_type: the parent GType
 * @init_fn: a POA initialization function
 * @fini_fn: a POA finialization function or NULL
 * @epv_struct_offset: the offset into the struct that the epv
 * commences at, or 0 if we are inheriting a plain GObject
 * from a BonoboObject, adding no new CORBA interfaces
 * @info: the standard GTypeInfo.
 * 
 * This function is the main entry point for deriving bonobo
 * server interfaces.
 * 
 * Return value: the constructed GType.
 **/
GType
bonobo_type_unique (GType             parent_type,
		    BonoboObjectPOAFn init_fn,
		    BonoboObjectPOAFn fini_fn,
		    int               epv_struct_offset,
		    const GTypeInfo  *info,
		    const gchar      *type_name)
{
	GType       type;

	/*
	 * Since we call g_type_class after the g_type_unique
	 * and before we can return the type to the get_type fn.
	 * it is possible we can re-enter here through eg. a
	 * type check macro, hence we need this guard.
	 */
	if ((type = g_type_from_name (type_name)))
		return type;

	type = g_type_register_static (parent_type, type_name, info, 0);
	if (!type)
		return 0;

	if (bonobo_type_setup (type, init_fn, fini_fn,
			       epv_struct_offset))
		return type;
	else
		return 0;
}
