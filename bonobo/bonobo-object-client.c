/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-object-client.c:
 *   This handles the client-view of a remote GNOME object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <bonobo/gnome-object-client.h>

/* These are for the Moniker activation */
#include <bonobo/gnome-stream.h>
#include <bonobo/gnome-moniker-client.h>
#include <bonobo/gnome-stream-fs.h>

static GnomeObjectClass *gnome_object_client_parent_class;

/**
 * gnome_object_client_construct:
 * @object_client: The GnomeObjectClient object to construct
 * @corba_object: The remote CORBA object this object_client refers to
 *
 * Initializes @object_client with the CORBA object for the
 * GNOME_ObjectClient interface provided in @corba_object.
 *
 * Returns: the initialized GnomeObjectClient object.
 */
GnomeObjectClient *
gnome_object_client_construct (GnomeObjectClient *object_client, CORBA_Object corba_object)
{
	g_return_val_if_fail (object_client != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_OBJECT_CLIENT (object_client), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);
	
	GNOME_OBJECT (object_client)->corba_objref = corba_object;

	return object_client;
}

/**
 * gnome_object_activate_with_repo_id:
 * @list: Preloaded list of servers or NULL.
 * @repo_id: CORBA interface repository id that we want to activate.
 * @flags: Goad activation flags.
 * @params: parameters passed to the factory
 *
 * Activates a service and wraps it on the GnomeObjectClient object.
 * The service activated should support the GNOME::object interface.
 *
 * This routine used goad_server_activate_with_repo_id() routine from
 * libgnorba.
 */
GnomeObjectClient *
gnome_object_activate_with_repo_id (GoadServerList *list,
				    const char *repo_id,
				    GoadActivationFlags flags,
				    const char **params)
{
	CORBA_Object corba_object;
	GnomeObjectClient *object;

	g_warning ("Activating objects by repo_id is a bad idea, try using the goad_id instead");
	
	corba_object = goad_server_activate_with_repo_id (NULL, repo_id, 0, NULL);
	if (corba_object == CORBA_OBJECT_NIL)
		return NULL;
	
	object = gtk_type_new (gnome_object_client_get_type ());
	gnome_object_client_construct (object, corba_object);

	return object;
}

/**
 * gnome_object_activate_with_goad_id:
 * @list: Preloaded list of servers or NULL.
 * @goad_id: GOAD service identification.
 * @flags: Goad activation flags.
 * @params: parameters passed to the factory
 *
 * Activates the service represented by goad_id.
 * The service activated should support the GNOME::object interface.
 *
 * This routine used goad_server_activate_with_repo_id() routine from
 * libgnorba.
 */
GnomeObjectClient *
gnome_object_activate_with_goad_id (GoadServerList *list,
				    const char *goad_id,
				    GoadActivationFlags flags,
				    const char **params)
{
	CORBA_Object corba_object;
	GnomeObjectClient *object;

	corba_object = goad_server_activate_with_id (NULL, goad_id, 0, NULL);
	if (corba_object == CORBA_OBJECT_NIL)
		return NULL;
	
	object = gtk_type_new (gnome_object_client_get_type ());
	gnome_object_client_construct (object, corba_object);

	return object;
}

static GList *
parse_moniker_string (const char *desc)
{
	GList *res = NULL;
	GString *s;
	const char *p;
	
	/* Compute number of elements */
	s = g_string_new ("");
	for (p = desc; *p; p++){
		if (*p == '\\'){
			p++;
			if (!*p)
				break;
			g_string_append_c (s, *p);
			continue;
		}
		if (*p == ','){
			res = g_list_append (res, g_strdup (s->str));
			g_string_assign (s, "");
		} else
			g_string_append_c (s, *p);
	}
	if (*s->str)
		res = g_list_append (res, g_strdup (s->str));
	
	g_string_free (s, TRUE);

	return res;
}

GNOME_Unknown
gnome_object_restore_from_url (const char *goad_id, const char *url)
{
#define gnome_object_restore_from_url_defined_here
	CORBA_Object rtn = CORBA_OBJECT_NIL;
	GNOME_PersistFile persist;
	gchar *name;
	CORBA_Environment ev;
	
	name = g_strdup_printf ("url_moniker!%s", url);
	
	/* 1. Check the naming service to see if we're already available */
	rtn = gnome_moniker_find_in_naming_service (name, goad_id);

	if (!rtn) {
		/* 2. fire up that object specified by the goad_id  */
		rtn = goad_server_activate_with_id (
			NULL,		/* name_server list */
			goad_id,	/* server to activate */
			0,		/* GoadActivationFlags */
			0);		/* params for activation */
		
		g_free (name);

		CORBA_exception_init (&ev);

		if (CORBA_Object_is_nil (rtn, &ev)) /* bail */ {

			CORBA_exception_free (&ev);
			return CORBA_OBJECT_NIL;
		}

		CORBA_exception_free (&ev);
	}
	
	CORBA_exception_init (&ev);
	/*
	 * 4. try to feed it the file (first by
	 * PersistFile, then by PersistStream)
	 */
	persist = (GNOME_PersistFile) GNOME_Unknown_query_interface (
		rtn, "IDL:GNOME/PersistFile:1.0", &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		persist = NULL;

	if (persist) {
		gboolean success = FALSE;
		
		GNOME_PersistFile_load (persist, url, &ev);

		if (ev._major == CORBA_NO_EXCEPTION)
			success = TRUE;
		
		GNOME_Unknown_unref (persist, &ev);
		CORBA_Object_release (persist, &ev);

		if (!success){
			GNOME_Unknown_unref (rtn, &ev);
			CORBA_Object_release (rtn, &ev);
			rtn = CORBA_OBJECT_NIL;
		}
	}
	else
	{
		/* doesn't have PersistFile; try with PersistStream */
		GnomeStream *stream;
		
		persist = GNOME_Unknown_query_interface (
			rtn, "IDL:GNOME/PersistStream:1.0", &ev);

		if (ev._major != CORBA_NO_EXCEPTION)
			persist = NULL;

		CORBA_exception_free (&ev);
		
		if (!persist)
			rtn = CORBA_OBJECT_NIL;
		else {
			stream = gnome_stream_fs_open (
				url, GNOME_Storage_READ);
			if (!stream)
				rtn = CORBA_OBJECT_NIL;
			else {
				GNOME_PersistStream_load (
					(GNOME_PersistStream) persist,
					gnome_object_corba_objref (GNOME_OBJECT (stream)),
					&ev);
				
				if (ev._major != CORBA_NO_EXCEPTION)
					rtn = CORBA_OBJECT_NIL;

				GNOME_Unknown_unref (persist, &ev);
				CORBA_Object_release (persist, &ev);


			}
		}
	}
	
	CORBA_exception_free (&ev);
	return (GNOME_Unknown) rtn;
}

static void
moniker_info_list_destroy (GList *moniker_info_list)
{
	g_list_foreach (moniker_info_list, (GFunc) g_free, NULL);
	g_list_free (moniker_info_list);
}

/**
 * gnome_object_activate:
 * @object_desc: Either a string representation of an object moniker
 * or an object goad id.
 * @flags: activation flags
 *
 * Returns: An object created.
 */
GnomeObjectClient *
gnome_object_activate (const char *object_desc, GoadActivationFlags flags)
{
	CORBA_Environment ev;
	GNOME_Unknown obj, last;
	GnomeObjectClient *object;
	GList *moniker_info, *item;
	
	g_return_val_if_fail (object_desc != NULL, NULL);

	if (strncmp (object_desc, "moniker_url:", 12) != 0)
		return gnome_object_activate_with_goad_id (NULL, object_desc, flags, NULL);

	moniker_info = parse_moniker_string (object_desc + 12);
	if (g_list_length (moniker_info) < 2){
		moniker_info_list_destroy (moniker_info);
		return NULL;
	}

	/*
	 * Bind the file
	 */
	obj = gnome_object_restore_from_url (moniker_info->data, moniker_info->next->data);

	if (obj == CORBA_OBJECT_NIL){
		moniker_info_list_destroy (moniker_info);
		return NULL;
	}

	last = obj;
	CORBA_exception_init (&ev);
	for (item = moniker_info->next->next; item; item = item->next){
		GNOME_Container container;
		GNOME_Unknown new_object;
		const char *item_name = item->data;

		container = (GNOME_Container) GNOME_Unknown_query_interface (
			last, "IDL:GNOME/Container:1.0", &ev);
		if (container == CORBA_OBJECT_NIL){
			moniker_info_list_destroy (moniker_info);
			GNOME_Unknown_unref (obj, &ev);
			CORBA_exception_free (&ev);
			return NULL;
		}

		new_object = GNOME_Container_get_object (
			container, item_name, TRUE, &ev);

		GNOME_Unknown_unref ((GNOME_Unknown) container, &ev);
		if (new_object == CORBA_OBJECT_NIL){
			moniker_info_list_destroy (moniker_info);
			GNOME_Unknown_unref (obj, &ev);
			CORBA_exception_free (&ev);
			return NULL;
		}
		last = new_object;
	}
	moniker_info_list_destroy (moniker_info);
	CORBA_exception_free (&ev);

	object = gtk_type_new (gnome_object_client_get_type ());
	gnome_object_client_construct (object, (CORBA_Object) last);
	
	return object;
}

/**
 * gnome_object_client_from_corba:
 * @corba_object: An existing CORBA object that implements GNOME_Unknown.
 *
 * Wraps the @corba_object CORBA object reference in a GnomeObjectClient
 * object.  This is typically used if you got a CORBA object yourself and not
 * through one of the activation routines and you want to have a GnomeObjectClient
 * handle to use in any of the Bonobo routines. 
 *
 * Returns: A wrapped GnomeObjectClient.
 */
GnomeObjectClient *
gnome_object_client_from_corba (GNOME_Unknown o)
{
	GnomeObjectClient *object;
	
	g_return_val_if_fail (o != CORBA_OBJECT_NIL, NULL);

	object = gtk_type_new (gnome_object_client_get_type ());
	gnome_object_client_construct (object, o);

	return object;
}


/**
 * gnome_object_client_query_interface:
 * @object: object to query interface of
 * @interface_desc: interface description
 * @opt_ev: optional exception environment, or NULL
 * 
 * Queries the object to see if it implements the interface
 * described by @interface_desc. Basicaly a thin
 * GNOME_Unknown::query_interface wrapper.
 * 
 * Return value: A valid GNOME_Unknown reference or
 *               CORBA_OBJECT_NIL if anything untoward happens.
 **/
GNOME_Unknown
gnome_object_client_query_interface (GnomeObjectClient *object,
				     const char *interface_desc,
				     CORBA_Environment *opt_ev)
{
	GNOME_Unknown      interface;
	CORBA_Environment *ev, real_ev;

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (interface_desc != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (GNOME_IS_OBJECT_CLIENT (object), CORBA_OBJECT_NIL);

	if (opt_ev)
		ev = opt_ev;
	else {
		ev = &real_ev;
		CORBA_exception_init (ev);
	}

	interface = GNOME_Unknown_query_interface (
		gnome_object_corba_objref (GNOME_OBJECT (object)),
		interface_desc, ev);
	
        if (ev->_major != CORBA_NO_EXCEPTION) {
		gnome_object_check_env (GNOME_OBJECT (object),
					gnome_object_corba_objref (GNOME_OBJECT (object)),
					ev);
		if (!opt_ev)
			CORBA_exception_free (ev);

                return CORBA_OBJECT_NIL;
	}

	if (!opt_ev)
		CORBA_exception_free (ev);

	return interface;
}

/**
 * gnome_object_client_has_interface:
 * @object: object to query interface of
 * @interface_desc: interface description
 * @opt_ev: optional exception environment, or NULL
 * 
 * Queries the object to see if it implements the interface
 * described by @interface_desc. Basicaly a thin
 * GNOME_Unknown::query_interface wrapper.
 * 
 * Return value: TRUE if the interface is available else FALSE.
 **/
gboolean
gnome_object_client_has_interface (GnomeObjectClient *object,
				   const char *interface_desc,
				   CORBA_Environment *opt_ev)
{
	GNOME_Unknown      interface;

	/* safe type checking in query_interface */
	interface = gnome_object_client_query_interface (object,
							 interface_desc,
							 opt_ev);

	if (interface != CORBA_OBJECT_NIL) {
		CORBA_Environment *ev, real_ev;

		if (opt_ev)
			ev = opt_ev;
		else {
			ev = &real_ev;
			CORBA_exception_init (ev);
		}

		GNOME_Unknown_unref  (interface, ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			gnome_object_check_env (GNOME_OBJECT (object),
						gnome_object_corba_objref (GNOME_OBJECT (object)),
						ev);
			if (!opt_ev)
				CORBA_exception_free (ev);
			return FALSE;
		}

		CORBA_Object_release (interface, ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			gnome_object_check_env (GNOME_OBJECT (object),
						gnome_object_corba_objref (GNOME_OBJECT (object)),
						ev);
			if (!opt_ev)
				CORBA_exception_free (ev);
			return FALSE;
		}

		if (!opt_ev)
			CORBA_exception_free (ev);
		return TRUE;
	} else
		return FALSE;
}

static void
gnome_object_client_destroy (GtkObject *object)
{
	GnomeObject *gnome_object = GNOME_OBJECT (object);
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	GNOME_Unknown_unref (gnome_object_corba_objref (gnome_object), &ev);
	CORBA_exception_free (&ev);

	GTK_OBJECT_CLASS (gnome_object_client_parent_class)->destroy (object);
}

static void
gnome_object_client_class_init (GnomeObjectClientClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_object_client_parent_class = gtk_type_class (gnome_object_get_type ());

	object_class->destroy = gnome_object_client_destroy;
}

/**
 * gnome_object_client_get_type:
 *
 * Returns: the GtkType for the GnomeObjectClient class.
 */
GtkType
gnome_object_client_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"Handle to remote GNOME::Object",
			sizeof (GnomeObjectClient),
			sizeof (GnomeObjectClientClass),
			(GtkClassInitFunc) gnome_object_client_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}

#ifndef gnome_object_restore_from_url_defined_here
#error You might want to remove all the headers included here to get this file here.
#endif
