/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-object-client.c:
 *   This handles the client-view of a remote Bonobo object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <bonobo/bonobo-object-client.h>

/* These are for the Moniker activation */
#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-moniker-client.h>
#include <bonobo/bonobo-stream-fs.h>
#include "bonobo-object-directory.h"

static BonoboObjectClass *bonobo_object_client_parent_class;

/**
 * bonobo_object_client_construct:
 * @object_client: The BonoboObjectClient object to construct
 * @corba_object: The remote CORBA object this object_client refers to
 *
 * Initializes @object_client with the CORBA object for the
 * Bonobo::Unknown interface provided in @corba_object.
 *
 * Returns: the initialized BonoboObjectClient object.
 */
BonoboObjectClient *
bonobo_object_client_construct (BonoboObjectClient *object_client, CORBA_Object corba_object)
{
	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (object_client), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);
	
	BONOBO_OBJECT (object_client)->corba_objref = corba_object;

	return object_client;
}

static BonoboObjectClient *
bonobo_object_activate_with_either_id(const gchar* iid,
				     gint flags)
{
	CORBA_Object corba_object;
	BonoboObjectClient *object;
	
	corba_object = od_server_activate_with_id (iid, flags, NULL);
	if (corba_object == CORBA_OBJECT_NIL)
		return NULL;
	
	object = gtk_type_new (bonobo_object_client_get_type ());
	bonobo_object_client_construct (object, corba_object);

	return object;
}

/**
 * bonobo_object_activate_with_goad_id:
 * @dummy: obsolete, ignored.
 * @goad_id: GOAD service identification.
 * @flags: Goad activation flags.
 * @params: parameters passed to the factory
 *
 * Activates the service represented by goad_id.
 * The service activated should support the Bonobo::Unknown interface.
 *
 * This routine used goad_server_activate_with_repo_id() routine from
 * libgnorba.
 */
BonoboObjectClient *
bonobo_object_activate_with_goad_id (gpointer dummy,
				    const char *goad_id,
				    gint flags,
				    const char **params)
{
	od_assert_using_goad();

	return bonobo_object_activate_with_either_id(goad_id, flags);
}

/**
 * bonobo_object_activate_with_oaf_id:
 * @list: Preloaded list of servers or NULL.
 * @oaf_id: OAF IID 
 * @flags: OAF activation flags
 * @params: parameters passed to the factory
 *
 * Activates the service represented by @oaf_id.
 * The service activated should support the Bonobo::Unknown interface.
 *
 */
BonoboObjectClient *
bonobo_object_activate_with_oaf_id (const char *oaf_id,
				   gint flags)
{
	od_assert_using_oaf();
	
	return bonobo_object_activate_with_either_id(oaf_id, flags);
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

Bonobo_Unknown
bonobo_object_restore_from_url (const char *iid, const char *url)
{
#define bonobo_object_restore_from_url_defined_here
	CORBA_Object rtn = CORBA_OBJECT_NIL;
	Bonobo_PersistFile persist;
	gchar *name;
	CORBA_Environment ev;
	
	name = g_strdup_printf ("url_moniker!%s", url);
	
	/* 1. Check the naming service to see if we're already available */
	rtn = bonobo_moniker_find_in_naming_service (name, iid);

	if (!rtn) {
		/* 2. fire up that object specified by the iid  */
		CORBA_exception_init (&ev);
		
		rtn = od_server_activate_with_id (
			iid,	/* server to activate */
			0,		/* ActivationFlags */
			&ev);	
		
		g_free (name);



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
	persist = (Bonobo_PersistFile) Bonobo_Unknown_query_interface (
		rtn, "IDL:Bonobo/PersistFile:1.0", &ev);

	if (ev._major != CORBA_NO_EXCEPTION)
		persist = NULL;

	if (persist) {
		gboolean success = FALSE;
		
		Bonobo_PersistFile_load (persist, url, &ev);

		if (ev._major == CORBA_NO_EXCEPTION)
			success = TRUE;
		
		Bonobo_Unknown_unref (persist, &ev);
		CORBA_Object_release (persist, &ev);

		if (!success){
			Bonobo_Unknown_unref (rtn, &ev);
			CORBA_Object_release (rtn, &ev);
			rtn = CORBA_OBJECT_NIL;
		}
	}
	else
	{
		/* doesn't have PersistFile; try with PersistStream */
		BonoboStream *stream;
		
		persist = Bonobo_Unknown_query_interface (
			rtn, "IDL:Bonobo/PersistStream:1.0", &ev);

		if (ev._major != CORBA_NO_EXCEPTION)
			persist = NULL;

		CORBA_exception_free (&ev);
		
		if (!persist)
			rtn = CORBA_OBJECT_NIL;
		else {
			stream = bonobo_stream_fs_open (
				url, Bonobo_Storage_READ);
			if (!stream)
				rtn = CORBA_OBJECT_NIL;
			else {
				Bonobo_PersistStream_load (
					(Bonobo_PersistStream) persist,
					bonobo_object_corba_objref (BONOBO_OBJECT (stream)),
					"", &ev);
				
				if (ev._major != CORBA_NO_EXCEPTION)
					rtn = CORBA_OBJECT_NIL;

				Bonobo_Unknown_unref (persist, &ev);
				CORBA_Object_release (persist, &ev);


			}
		}
	}
	
	CORBA_exception_free (&ev);
	return (Bonobo_Unknown) rtn;
}

static void
moniker_info_list_destroy (GList *moniker_info_list)
{
	g_list_foreach (moniker_info_list, (GFunc) g_free, NULL);
	g_list_free (moniker_info_list);
}

/**
 * bonobo_object_activate:
 * @object_desc: Either a string representation of an object moniker
 * or an object ID (GOAD ID or OAF ID depending on Bonobo compilation).
 * @flags: activation flags
 *
 * Returns: An object created.
 */
BonoboObjectClient *
bonobo_object_activate (const char *object_desc, gint flags)
{
	CORBA_Environment ev;
	Bonobo_Unknown obj, last;
	BonoboObjectClient *object;
	GList *moniker_info, *item;
	
	g_return_val_if_fail (object_desc != NULL, NULL);

	if (strncmp (object_desc, "moniker_url:", 12) != 0)
		return bonobo_object_activate_with_either_id (object_desc, flags);

	moniker_info = parse_moniker_string (object_desc + 12);
	if (g_list_length (moniker_info) < 2){
		moniker_info_list_destroy (moniker_info);
		return NULL;
	}

	/*
	 * Bind the file
	 */
	obj = bonobo_object_restore_from_url (moniker_info->data, moniker_info->next->data);

	if (obj == CORBA_OBJECT_NIL){
		moniker_info_list_destroy (moniker_info);
		return NULL;
	}

	last = obj;
	CORBA_exception_init (&ev);
	for (item = moniker_info->next->next; item; item = item->next){
		Bonobo_Container container;
		Bonobo_Unknown new_object;
		const char *item_name = item->data;

		container = (Bonobo_Container) Bonobo_Unknown_query_interface (
			last, "IDL:Bonobo/Container:1.0", &ev);
		if (container == CORBA_OBJECT_NIL){
			moniker_info_list_destroy (moniker_info);
			Bonobo_Unknown_unref (obj, &ev);
			CORBA_exception_free (&ev);
			return NULL;
		}

		new_object = Bonobo_Container_get_object (
			container, item_name, TRUE, &ev);

		Bonobo_Unknown_unref ((Bonobo_Unknown) container, &ev);
		if (new_object == CORBA_OBJECT_NIL){
			moniker_info_list_destroy (moniker_info);
			Bonobo_Unknown_unref (obj, &ev);
			CORBA_exception_free (&ev);
			return NULL;
		}
		last = new_object;
	}
	moniker_info_list_destroy (moniker_info);
	CORBA_exception_free (&ev);

	object = gtk_type_new (bonobo_object_client_get_type ());
	bonobo_object_client_construct (object, (CORBA_Object) last);
	
	return object;
}

/**
 * bonobo_object_client_from_corba:
 * @corba_object: An existing CORBA object that implements Bonobo_Unknown.
 *
 * Wraps the @corba_object CORBA object reference in a BonoboObjectClient
 * object.  This is typically used if you got a CORBA object yourself and not
 * through one of the activation routines and you want to have a BonoboObjectClient
 * handle to use in any of the Bonobo routines. 
 *
 * Returns: A wrapped BonoboObjectClient.
 */
BonoboObjectClient *
bonobo_object_client_from_corba (Bonobo_Unknown o)
{
	BonoboObjectClient *object;
	
	g_return_val_if_fail (o != CORBA_OBJECT_NIL, NULL);

	object = gtk_type_new (bonobo_object_client_get_type ());
	bonobo_object_client_construct (object, o);

	return object;
}


/**
 * bonobo_object_client_query_interface:
 * @object: object to query interface of
 * @interface_desc: interface description
 * @opt_ev: optional exception environment, or NULL
 * 
 * Queries the object to see if it implements the interface
 * described by @interface_desc. Basicaly a thin
 * Bonobo_Unknown::query_interface wrapper.
 * 
 * Return value: A valid Bonobo_Unknown reference or
 *               CORBA_OBJECT_NIL if anything untoward happens.
 **/
Bonobo_Unknown
bonobo_object_client_query_interface (BonoboObjectClient *object,
				     const char *interface_desc,
				     CORBA_Environment *opt_ev)
{
	Bonobo_Unknown      interface;
	CORBA_Environment *ev, real_ev;

	g_return_val_if_fail (BONOBO_IS_OBJECT_CLIENT (object), CORBA_OBJECT_NIL);
	g_return_val_if_fail (interface_desc != NULL, CORBA_OBJECT_NIL);

	if (opt_ev)
		ev = opt_ev;
	else {
		ev = &real_ev;
		CORBA_exception_init (ev);
	}

	interface = Bonobo_Unknown_query_interface (
						   bonobo_object_corba_objref (BONOBO_OBJECT (object)),
						   interface_desc, ev);
	
        if (ev->_major != CORBA_NO_EXCEPTION) {
		bonobo_object_check_env (BONOBO_OBJECT (object),
					bonobo_object_corba_objref (BONOBO_OBJECT (object)),
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
 * bonobo_object_client_has_interface:
 * @object: object to query interface of
 * @interface_desc: interface description
 * @opt_ev: optional exception environment, or NULL
 * 
 * Queries the object to see if it implements the interface
 * described by @interface_desc. Basicaly a thin
 * Bonobo_Unknown::query_interface wrapper.
 * 
 * Return value: TRUE if the interface is available else FALSE.
 **/
gboolean
bonobo_object_client_has_interface (BonoboObjectClient *object,
				   const char *interface_desc,
				   CORBA_Environment *opt_ev)
{
	Bonobo_Unknown      interface;

	/* safe type checking in query_interface */
	interface = bonobo_object_client_query_interface (object,
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

		Bonobo_Unknown_unref  (interface, ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			bonobo_object_check_env (BONOBO_OBJECT (object),
						bonobo_object_corba_objref (BONOBO_OBJECT (object)),
						ev);
			if (!opt_ev)
				CORBA_exception_free (ev);
			return FALSE;
		}

		CORBA_Object_release (interface, ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			bonobo_object_check_env (BONOBO_OBJECT (object),
						bonobo_object_corba_objref (BONOBO_OBJECT (object)),
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
bonobo_object_client_destroy (GtkObject *object)
{
	BonoboObject *bonobo_object = BONOBO_OBJECT (object);
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	Bonobo_Unknown_unref (bonobo_object_corba_objref (bonobo_object), &ev);
	CORBA_exception_free (&ev);

	GTK_OBJECT_CLASS (bonobo_object_client_parent_class)->destroy (object);
}

static void
bonobo_object_client_class_init (BonoboObjectClientClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_object_client_parent_class = gtk_type_class (bonobo_object_get_type ());

	object_class->destroy = bonobo_object_client_destroy;
}

/**
 * bonobo_object_client_get_type:
 *
 * Returns: the GtkType for the BonoboObjectClient class.
 */
GtkType
bonobo_object_client_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"Handle to remote Bonobo::Unknown",
			sizeof (BonoboObjectClient),
			sizeof (BonoboObjectClientClass),
			(GtkClassInitFunc) bonobo_object_client_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

#ifndef bonobo_object_restore_from_url_defined_here
#error You might want to remove all the headers included here to get this file here.
#endif
