/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-context.h: Handle Global Component contexts.
 *
 * Author:
 *     Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-activation-context.h>

static GHashTable *bonobo_contexts = NULL;

void
bonobo_context_add (const CORBA_char *context_name,
		    Bonobo_Unknown    context)
{
	g_return_if_fail (context != CORBA_OBJECT_NIL);

	if (!bonobo_contexts) {
		bonobo_contexts = g_hash_table_new (
			g_str_hash, g_str_equal);
		g_atexit (bonobo_context_shutdown);
	}

	g_hash_table_insert (bonobo_contexts,
			     g_strdup (context_name),
			     bonobo_object_dup_ref (context, NULL));
}

Bonobo_Unknown
bonobo_context_get (const CORBA_char  *context_name,
		    CORBA_Environment *opt_ev)
{
	Bonobo_Unknown ret;

	g_return_val_if_fail (context_name != NULL, CORBA_OBJECT_NIL);

	if ((ret = g_hash_table_lookup (bonobo_contexts, context_name)))
		return bonobo_object_dup_ref (ret, opt_ev);
	else
		return CORBA_OBJECT_NIL;
}

void
bonobo_context_init (void)
{
	BonoboObject *object;

	object = bonobo_activation_context_new ();
	bonobo_context_add ("Activation", 
			    bonobo_object_corba_objref (object));
	bonobo_object_unref (object);
}

static gboolean
context_destroy (char *key, Bonobo_Unknown handle, gpointer dummy)
{
	g_free (key);
	bonobo_object_release_unref (handle, NULL);
	return TRUE;
}

void
bonobo_context_shutdown (void)
{
	if (!bonobo_contexts)
		return;

	g_hash_table_foreach_remove (
		bonobo_contexts, (GHRFunc) context_destroy, NULL);
	g_hash_table_destroy (bonobo_contexts);
	bonobo_contexts = NULL;
}
