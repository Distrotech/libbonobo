/**
 * bonobo-moniker-util.c
 *
 * Copyright (C) 2000  Helix Code, Inc.
 *
 * Authors:
 *	Michael Meeks    (michael@helixcode.com)
 *	Ettore Perazzoli (ettore@helixcode.com)
 */

#include "bonobo.h"
#include <liboaf/liboaf.h>

struct {
	char *prefix;
	char *oafiid;
} fast_prefix [] = {
	{ "file:",   "OAFIID:Bonobo_Moniker_File"  },
	{ "query:(", "OAFIID:Bonobo_Moniker_Query" },
	{ "!",       "OAFIID:Bonobo_Moniker_Item"  },
	{ "OAFIID:", "OAFIID:Bonobo_Moniker_Oaf"   },
	{ "OAFAID:", "OAFIID:Bonobo_Moniker_Oaf"   },
	{ "new:",    "OAFIID:Bonobo_Moniker_New"   },
	{ "http:",   "OAFIID:Bonobo_Moniker_HTTP"  },
/*
	{ "queue:", "" },
*/
	{ NULL, NULL }
};

static char *
moniker_id_from_nickname (const CORBA_char *name)
{
	int i;

	for (i = 0; fast_prefix [i].prefix; i++) {
		int len = strlen (fast_prefix [i].prefix);

		if (!g_strncasecmp (fast_prefix [i].prefix, name, len)) {

			return fast_prefix [i].oafiid;
		}
	}

	return NULL;
}

Bonobo_Moniker
bonobo_moniker_util_new_from_name_full (Bonobo_Moniker     parent,
					const CORBA_char  *name,
					CORBA_Environment *ev)
{
	Bonobo_Unknown   object;
	Bonobo_Moniker   toplevel, moniker;
	const char       *iid;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (name [0] == '#')
		name++;

	iid = moniker_id_from_nickname (name);

	if (!iid) { /* Do an oaf-query for a handler */
		char *prefix, *query;
		int   len;

		for (len = 0; name [len]; len++) {
			if (name [len] == ':')
				break;
		}

		prefix = g_strndup (name, len);
		
		query = g_strdup_printf (
			"repo_ids.has ('IDL:Bonobo/Moniker:1.0') AND "
			"bonobo:moniker == '%s'", prefix);
		g_free (prefix);
		
		object = oaf_activate (query, NULL, 0, NULL, ev);
		g_free (query);
		
		if (ev->_major != CORBA_NO_EXCEPTION)
			return CORBA_OBJECT_NIL;

		if (object == CORBA_OBJECT_NIL) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Moniker_UnknownPrefix, NULL);
			return CORBA_OBJECT_NIL;
		}
	} else {
		object = oaf_activate_from_id ((gchar *) iid, 0, NULL, ev);

		if (ev->_major != CORBA_NO_EXCEPTION)
			return CORBA_OBJECT_NIL;
		
		if (object == CORBA_OBJECT_NIL) {
			g_warning ("Activating '%s' returned nothing", iid);
			return CORBA_OBJECT_NIL;
		}
	}

	toplevel = Bonobo_Unknown_queryInterface (
		object, "IDL:Bonobo/Moniker:1.0", ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Moniker object '%s' doesn't implement "
			   "the Moniker interface", iid);
		return CORBA_OBJECT_NIL;
	}

	moniker = Bonobo_Moniker_parseDisplayName (toplevel, parent,
						   name, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	bonobo_object_release_unref (toplevel, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	return moniker;
}

CORBA_char *
bonobo_moniker_util_get_parent_name (Bonobo_Moniker     moniker,
				     CORBA_Environment *ev)
{
	Bonobo_Moniker parent;
	CORBA_char    *name;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, NULL);

	parent = Bonobo_Moniker__get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION ||
	    parent == CORBA_OBJECT_NIL)
		return NULL;
	
	name = Bonobo_Moniker_getDisplayName (parent, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		name = NULL;

	bonobo_object_release_unref (parent, ev);

	return name;
}

Bonobo_Unknown
bonobo_moniker_util_qi_return (Bonobo_Unknown     object,
			       const CORBA_char  *requested_interface,
			       CORBA_Environment *ev)
{
	Bonobo_Unknown retval = CORBA_OBJECT_NIL;

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Object is NIL");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	retval = Bonobo_Unknown_queryInterface (
		object, requested_interface, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto release_unref_object;
	
	if (retval == CORBA_OBJECT_NIL) {
		g_warning ("No interface '%s' on object", requested_interface);
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		goto release_unref_object;
	}

 release_unref_object:	
	bonobo_object_release_unref (object, ev);

	if (retval != CORBA_OBJECT_NIL)
		return CORBA_Object_duplicate (retval, ev);
	else
		return CORBA_OBJECT_NIL;
}

int
bonobo_moniker_util_seek_std_separator (const CORBA_char *str,
					int               min_idx)
{
	int i;

	g_return_val_if_fail (str != NULL, 0);
	g_return_val_if_fail (min_idx >= 0, 0);

	for (i = 0; i < min_idx; i++) {
		if (!str [i]) {
			g_warning ("Serious separator error, seeking in '%s' < %d",
				   str, min_idx);
			return i;
		}
	}

#warning We need escaping support here 

	for (; str [i]; i++) {

		if (str [i] == '!' ||
		    str [i] == '#')
			break;
	}
	
	return i;
}

Bonobo_Moniker
bonobo_moniker_client_new_from_name (const CORBA_char  *name,
				     CORBA_Environment *ev)
{
	return bonobo_moniker_util_new_from_name_full (
		CORBA_OBJECT_NIL, name, ev);
}

CORBA_char *
bonobo_moniker_client_get_name (Bonobo_Moniker     moniker,
				CORBA_Environment *ev)
{
	CORBA_char *name;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, NULL);

	name = Bonobo_Moniker_getDisplayName (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return NULL;

	return name;
}

static void
init_default_resolve_options (Bonobo_ResolveOptions *options)
{
	options->flags = 0;
	options->timeout = -1;
}

Bonobo_Unknown
bonobo_moniker_client_resolve_default (Bonobo_Moniker     moniker,
				       const char        *interface_name,
				       CORBA_Environment *ev)
{
	Bonobo_ResolveOptions options;
	
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	init_default_resolve_options (&options);

	return Bonobo_Moniker_resolve (moniker, &options, interface_name, ev);
}

BonoboObjectClient *
bonobo_moniker_client_resolve_client_default (Bonobo_Moniker     moniker,
					      const char        *interface_name,
					      CORBA_Environment *ev)
{
	Bonobo_Unknown unknown;

	g_return_val_if_fail (ev != NULL, NULL);

	unknown = bonobo_moniker_client_resolve_default (
		moniker, interface_name, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return NULL;

	if (unknown == CORBA_OBJECT_NIL)
		return NULL;

	return bonobo_object_client_from_corba (unknown);
}

Bonobo_Unknown
bonobo_get_object (const CORBA_char *name,
		   const char        *interface_name,
		   CORBA_Environment *ev)
{
	Bonobo_Moniker moniker;
	Bonobo_Unknown retval;

	moniker = bonobo_moniker_client_new_from_name (name, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	retval = bonobo_moniker_client_resolve_default (
		moniker, interface_name, ev);

	bonobo_object_release_unref (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	return retval;

}
void
bonobo_moniker_client_new_from_name_async (const CORBA_char   *name,
					   CORBA_Environment  *ev,
					   guint               timeout_usec,
					   BonoboAsyncCallback cb,
					   gpointer            user_data)
{
/*	static const CORBA_TypeCode args [] = {
		TC_Object,
		TC_string,
	};
	static const CORBA_TypeCode exceptions [] = {
		TC_Bonobo_Moniker_InvalidSyntax,
		TC_Bonobo_Moniker_UnknownPrefix,
		NULL
	};
	static const BonoboAsyncFlags flags [] = {
		BONOBO_ASYNC_IN,
		BONOBO_ASYNC_IN
	};
	static const BonoboAsyncMethod method = {
		"parseDisplayName", TC_Object, args, 2,
		exceptions, flags
	};
	CORBA_Object obj = CORBA_OBJECT_NIL;
	gpointer arg_values [2] = { &obj, &name };
	
	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (name != CORBA_OBJECT_NIL);

	bonobo_async_invoke (&method, cb, user_data,
	moniker, arg_values, ev);*/
	g_warning ("Unimplemented as yet");
}

void
bonobo_moniker_resolve_async (Bonobo_Moniker         moniker,
			      Bonobo_ResolveOptions *options,
			      const char            *interface_name,
			      CORBA_Environment     *ev,
			      guint                  timeout_usec,
			      BonoboAsyncCallback    cb,
			      gpointer               user_data)
{
	static const CORBA_TypeCode args [] = {
		TC_Bonobo_ResolveOptions,
		TC_string,
	};
	static const CORBA_TypeCode exceptions [] = {
		TC_Bonobo_Moniker_InterfaceNotFound,
		TC_Bonobo_Moniker_UnknownPrefix,
		NULL
	};
	static const BonoboAsyncFlags flags [] = {
		BONOBO_ASYNC_IN,
		BONOBO_ASYNC_IN
	};
	static const BonoboAsyncMethod method = {
		"resolve", TC_Object, args, 2,
		exceptions, flags
	};
	gpointer arg_values [2] = { &options, &interface_name };
	
	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (options != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);

	bonobo_async_invoke (&method, cb, user_data, timeout_usec,
			     moniker, arg_values, ev);
}

void
bonobo_moniker_resolve_async_default (Bonobo_Moniker      moniker,
				      const char         *interface_name,
				      CORBA_Environment  *ev,
				      guint               timeout_usec,
				      BonoboAsyncCallback cb,
				      gpointer            user_data)
{
	Bonobo_ResolveOptions options;


	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);

	init_default_resolve_options (&options);

	bonobo_moniker_resolve_async (moniker, &options, interface_name,
				      ev, timeout_usec, cb, user_data);
}

