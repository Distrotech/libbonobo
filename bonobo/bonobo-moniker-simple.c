/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-moniker-simple: Simplified object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-simple.h>

static Bonobo_Unknown
simple_resolve (BonoboMoniker               *moniker,
		const Bonobo_ResolveOptions *options,
		const CORBA_char            *requested_interface,
		CORBA_Environment           *ev)
{
	BonoboMonikerSimple *simple;
	GValue               value;
	Bonobo_Unknown       ret;

	g_return_val_if_fail (BONOBO_IS_MONIKER_SIMPLE (moniker),
			      CORBA_OBJECT_NIL);

	simple = BONOBO_MONIKER_SIMPLE (moniker);

	g_value_init (&value, BONOBO_TYPE_CORBA_OBJECT);

	bonobo_closure_invoke (simple->resolve_fn, &value,
			       BONOBO_MONIKER_TYPE, moniker,
			       G_TYPE_POINTER, options,
			       G_TYPE_STRING, requested_interface,
			       BONOBO_TYPE_CORBA_EXCEPTION, ev, 0);

	ret = bonobo_value_get_corba_object (&value);
	g_value_unset (&value);
	
	return ret;
}

static void
simple_finalize (GObject *object)
{
	BonoboMonikerSimple *simple = (BonoboMonikerSimple *) object;

	if (simple->resolve_fn)
		g_closure_unref (simple->resolve_fn);
	simple->resolve_fn = NULL;
}

static void
bonobo_moniker_simple_class_init (BonoboMonikerClass *klass)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;

	klass->resolve = simple_resolve;
	
	gobject_class->finalize = simple_finalize;
}

static void 
bonobo_moniker_simple_init (GObject *object)
{
	/* nothing to do */
}

BONOBO_TYPE_FUNC (BonoboMonikerSimple, 
		  bonobo_moniker_get_type (),
		  bonobo_moniker_simple);

/**
 * bonobo_moniker_simple_construct:
 * @moniker: the moniker to construct
 * @name: the name of the moniker eg. 'file:'
 * @resolve_fn: the function used to resolve the moniker
 * 
 * Constructs a simple moniker
 * 
 * Return value: the constructed moniker or NULL on failure.
 **/
BonoboMoniker *
bonobo_moniker_simple_construct (BonoboMonikerSimple *moniker,
				 const char          *name,
				 GClosure            *resolve_fn)
{
	g_return_val_if_fail (resolve_fn != NULL, NULL);

	moniker->resolve_fn = resolve_fn;

	return bonobo_moniker_construct (
		BONOBO_MONIKER (moniker), name);
}

/**
 * bonobo_moniker_simple_new_gc:
 * @name: the display name for the moniker
 * @resolve_fn: a closure for the resolve process.
 * 
 * Create a new instance of a simplified moniker.
 * 
 * Return value: the moniker object
 **/
BonoboMoniker *
bonobo_moniker_simple_new_gc (const char *name,
			      GClosure   *resolve_fn)
{
	BonoboMoniker *moniker;

	moniker = g_object_new (bonobo_moniker_simple_get_type (), NULL);

	return bonobo_moniker_simple_construct (
		BONOBO_MONIKER_SIMPLE (moniker),
		name, resolve_fn);
}

/**
 * bonobo_moniker_simple_new:
 * @name: the display name for the moniker
 * @resolve_fn: a resolve function for the moniker
 * 
 * Create a new instance of a simplified moniker.
 * 
 * Return value: the moniker object
 **/
BonoboMoniker *
bonobo_moniker_simple_new (const char                  *name,
			   BonoboMonikerSimpleResolveFn resolve_fn)
{
	return bonobo_moniker_simple_new_gc (
		name, g_cclosure_new (G_CALLBACK (resolve_fn), NULL, NULL));
}


