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
#include <liboaf/oaf-async.h>

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

/*
 * get_full_interface_name:
 * @ifname: original name: can be in form Bonobo/Control
 *
 * Return value: full name eg. IDL:Bonobo/Control:1.0
 **/
static gchar *
get_full_interface_name (const char *ifname)
{
	int len, had_ver;
	const char *a;
	char *retval, *b;

	g_return_val_if_fail (ifname != NULL, NULL);

	len = strlen (ifname);
	retval = g_new (char, len + 4 + 4 + 1);

	strcpy (retval, "IDL:");
	a = ifname;
	b = retval + 4;

	if (ifname [0] == 'I' &&
	    ifname [1] == 'D' &&
	    ifname [2] == 'L' &&
	    ifname [3] == ':')
		a += 4;

	for (had_ver = 0; *a; a++, b++) {
		if (*a == ':')
			had_ver = 1;
		*b = *a;
	}

	if (!had_ver)
		strcpy (b, ":1.0");

	return retval;
}

static gchar *
query_from_name (const char *name)
{
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

	return query;
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

	if (!name [0])
		return bonobo_object_dup_ref (parent, ev);

	if (name [0] == '#')
		name++;

	if (!(iid = moniker_id_from_nickname (name))) {
		char *query;

		query = query_from_name (name);

		object = oaf_activate (query, NULL, 0, NULL, ev);

		g_free (query);
		
		if (BONOBO_EX (ev))
			return CORBA_OBJECT_NIL;

		if (object == CORBA_OBJECT_NIL) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Moniker_UnknownPrefix, NULL);
			return CORBA_OBJECT_NIL;
		}
	} else {
		object = oaf_activate_from_id ((gchar *) iid, 0, NULL, ev);

		if (BONOBO_EX (ev))
			return CORBA_OBJECT_NIL;
		
		if (object == CORBA_OBJECT_NIL) {
			g_warning ("Activating '%s' returned nothing", iid);
			return CORBA_OBJECT_NIL;
		}
	}

	toplevel = Bonobo_Unknown_queryInterface (
		object, "IDL:Bonobo/Moniker:1.0", ev);

	if (BONOBO_EX(ev))
		return CORBA_OBJECT_NIL;

	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Moniker object '%s' doesn't implement "
			   "the Moniker interface", iid);
		return CORBA_OBJECT_NIL;
	}

	moniker = Bonobo_Moniker_parseDisplayName (toplevel, parent,
						   name, ev);
	if (BONOBO_EX(ev))
		return CORBA_OBJECT_NIL;

	bonobo_object_release_unref (toplevel, ev);
	if (BONOBO_EX(ev))
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

	if (BONOBO_EX(ev) ||
	    parent == CORBA_OBJECT_NIL)
		return NULL;
	
	name = Bonobo_Moniker_getDisplayName (parent, ev);

	if (BONOBO_EX(ev))
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

	if (BONOBO_EX(ev))
		return CORBA_OBJECT_NIL;
	
	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Object is NIL");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	retval = Bonobo_Unknown_queryInterface (
		object, requested_interface, ev);

	if (BONOBO_EX(ev))
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

	for (; str [i]; i++) {

		if (str [i] == '\\' && str [i + 1])
			i++;
		else if (str [i] == '!' ||
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

	if (BONOBO_EX(ev))
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
	Bonobo_Unknown        retval;
	char                 *real_if;
	
	g_return_val_if_fail (interface_name != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	real_if = get_full_interface_name (interface_name);

	init_default_resolve_options (&options);

	retval = Bonobo_Moniker_resolve (moniker, &options, real_if, ev);

	g_free (real_if);

	return retval;
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

	if (BONOBO_EX(ev))
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

	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;

	retval = bonobo_moniker_client_resolve_default (
		moniker, interface_name, ev);

	bonobo_object_release_unref (moniker, ev);

	if (BONOBO_EX (ev))
		return CORBA_OBJECT_NIL;
	
	return retval;
}

typedef struct {
	char                *name;
	BonoboMonikerAsyncFn cb;
	gpointer             user_data;
	guint                timeout_usec;
	Bonobo_Unknown       moniker;
} parse_async_ctx_t;

static void
parse_async_ctx_free (parse_async_ctx_t *ctx)
{
	if (ctx) {
		g_free (ctx->name);
		g_free (ctx);
	}
}

static void
async_parse_cb (BonoboAsyncReply  *reply,
		CORBA_Environment *ev,
		gpointer           user_data)
{
	parse_async_ctx_t *ctx = user_data;

	if (BONOBO_EX (ev))
		ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
	else {
		Bonobo_Moniker retval;

		bonobo_async_demarshal (reply, &retval, NULL);

		ctx->cb (retval, ev, ctx->user_data);
	}

	bonobo_object_release_unref (ctx->moniker, ev);
	parse_async_ctx_free (ctx);
}

static void
async_activation_cb (CORBA_Object activated_object, 
		     const char  *error_reason, 
		     gpointer     user_data)
{
	parse_async_ctx_t *ctx = user_data;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (error_reason) { /* badly designed oaf interface */

		CORBA_exception_set (&ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_UnknownPrefix, NULL);

		ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
		parse_async_ctx_free (ctx);
	} else {
		ctx->moniker = Bonobo_Unknown_queryInterface (
			activated_object, "IDL:Bonobo/Moniker:1.0", &ev);

		if (BONOBO_EX (&ev)) {
			ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
			parse_async_ctx_free (ctx);
		
		} else if (ctx->moniker == CORBA_OBJECT_NIL) {
			CORBA_exception_set (&ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
			ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
			parse_async_ctx_free (ctx);
		} else {
			static const CORBA_TypeCode args [] = {
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
			gpointer arg_values [2] = { &obj, &ctx->name };
	
			bonobo_async_invoke (&method, async_parse_cb, ctx,
					     ctx->timeout_usec,
					     ctx->moniker, arg_values, &ev);
			
			if (BONOBO_EX (&ev)) {
				ctx->cb (CORBA_OBJECT_NIL, &ev, ctx->user_data);
				parse_async_ctx_free (ctx);
			}

			bonobo_object_release_unref (activated_object, &ev);
		}
	}

	CORBA_exception_free (&ev);
}

void
bonobo_moniker_client_new_from_name_async (const CORBA_char    *name,
					   CORBA_Environment   *ev,
					   guint                timeout_usec,
					   BonoboMonikerAsyncFn cb,
					   gpointer             user_data)
{
	parse_async_ctx_t *ctx;
	const char        *iid;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (name != NULL);

	if (!name [0]) {
		cb (CORBA_OBJECT_NIL, ev, user_data);
		return;
	}

	if (name [0] == '#')
		name++;

	ctx = g_new0 (parse_async_ctx_t, 1);
	ctx->name         = g_strdup (name);
	ctx->cb           = cb;
	ctx->user_data    = user_data;
	ctx->timeout_usec = timeout_usec;
	ctx->moniker      = CORBA_OBJECT_NIL;

	if (!(iid = moniker_id_from_nickname (name))) {
		char *query;

		query = query_from_name (name);

		oaf_activate_async (query, NULL, 0,
				    async_activation_cb, ctx, ev);

		g_free (query);
	} else
		oaf_activate_from_id_async ((gchar *) iid, 0,
					    async_activation_cb, ctx, ev);
}

typedef struct {
	Bonobo_Moniker       moniker;
	BonoboMonikerAsyncFn cb;
	gpointer             user_data;
} resolve_async_ctx_t;

static void
resolve_async_cb (BonoboAsyncReply  *handle,
		  CORBA_Environment *ev,
		  gpointer           user_data)
{
	resolve_async_ctx_t *ctx = user_data;

	if (BONOBO_EX (ev))
		ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
	else {
		Bonobo_Unknown object;
		bonobo_async_demarshal (handle, &object, NULL);
		ctx->cb (object, ev, ctx->user_data);
	}

	bonobo_object_release_unref (ctx->moniker, ev);
	g_free (ctx);
}

void
bonobo_moniker_resolve_async (Bonobo_Moniker         moniker,
			      Bonobo_ResolveOptions *options,
			      const char            *interface_name,
			      CORBA_Environment     *ev,
			      guint                  timeout_usec,
			      BonoboMonikerAsyncFn   cb,
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
	resolve_async_ctx_t *ctx;
	
	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (options != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);

	ctx = g_new0 (resolve_async_ctx_t, 1);
	ctx->cb = cb;
	ctx->user_data = user_data;
	ctx->moniker = bonobo_object_dup_ref (moniker, ev);

	bonobo_async_invoke (&method, resolve_async_cb, ctx,
			     timeout_usec, ctx->moniker, arg_values, ev);
}

void
bonobo_moniker_resolve_async_default (Bonobo_Moniker       moniker,
				      const char          *interface_name,
				      CORBA_Environment   *ev,
				      guint                timeout_usec,
				      BonoboMonikerAsyncFn cb,
				      gpointer             user_data)
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


typedef struct {
	guint                timeout_usec;
	char                *interface_name;
	BonoboMonikerAsyncFn cb;
	gpointer             user_data;
} get_object_async_ctx_t;

static void
get_object_async_ctx_free (get_object_async_ctx_t *ctx)
{
	if (ctx) {
		g_free (ctx->interface_name);
		g_free (ctx);
	}
}

static void
get_async2_cb (Bonobo_Unknown     object,
	       CORBA_Environment *ev,
	       gpointer           user_data)
{
	get_object_async_ctx_t *ctx = user_data;

	ctx->cb (object, ev, ctx->user_data);

	get_object_async_ctx_free (ctx);
}	

static void
get_async1_cb (Bonobo_Unknown     object,
	       CORBA_Environment *ev,
	       gpointer           user_data)
{
	get_object_async_ctx_t *ctx = user_data;

	if (BONOBO_EX (ev)) {
		ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
		get_object_async_ctx_free (ctx);
	} else {
                bonobo_moniker_resolve_async_default (
			object, ctx->interface_name, ev,
			ctx->timeout_usec, get_async2_cb, ctx);

		if (BONOBO_EX (ev)) {
			ctx->cb (CORBA_OBJECT_NIL, ev, ctx->user_data);
			get_object_async_ctx_free (ctx);
		}
	}
}	

void
bonobo_get_object_async (const CORBA_char    *name,
			 const char          *interface_name,
			 CORBA_Environment   *ev,
			 guint                timeout_usec,
			 BonoboMonikerAsyncFn cb,
			 gpointer             user_data)
{
	get_object_async_ctx_t *ctx;

	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (interface_name != NULL);

	ctx = g_new0 (get_object_async_ctx_t, 1);
	ctx->cb = cb;
	ctx->user_data = user_data;
	ctx->interface_name = get_full_interface_name (interface_name);
	ctx->timeout_usec = timeout_usec;

	bonobo_moniker_client_new_from_name_async (
		name, ev, timeout_usec, get_async1_cb, ctx);
}
