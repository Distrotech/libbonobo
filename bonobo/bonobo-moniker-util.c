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
	{ "file:",   "OAFIID:FileMoniker:50425b-8309-45c5-8e08-7a14c2602438"    },
	{ "query:(", "OAFIID:QueryMoniker:fb7f1678-9341-4749-9c5c-c3bb7b93abc1" },
	{ "!",       "OAFIID:ItemMoniker:42d4d657-3be0-4e1a-9c1b-16e8da332e05"  },
	{ "OAFIID:", "OAFIID:OafMoniker:f7fd21cb-4aa9-4802-83e0-d82eabc983a8"   },
	{ "OAFAID:", "OAFIID:OafMoniker:f7fd21cb-4aa9-4802-83e0-d82eabc983a8"   },
	{ "new:",    "OAFIID:NewMoniker:4ebfc80c-95f9-4f66-acfb-1a26b2d04cd4"   },
	{ "http:",   "OAFIID:HTTPMoniker:ae51bf8c-4ac0-493c-8559-001eb6be4a4f"  },
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

Bonobo_Unknown
bonobo_moniker_client_resolve_default (Bonobo_Moniker     moniker,
				       const char        *interface_name,
				       CORBA_Environment *ev)
{
	Bonobo_ResolveOptions options = { 0, -1 };

	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

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
