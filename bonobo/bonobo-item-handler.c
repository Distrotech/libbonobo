/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-item-handler.c: a generic Item Container resolver (implements ItemContainer)
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 2000 Miguel de Icaza.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkwidget.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>
#include "bonobo-item-handler.h"

static POA_Bonobo_ItemContainer__vepv item_handler_vepv;

static CORBA_Object
create_bonobo_item_handler (BonoboObject *object)
{
	POA_Bonobo_ItemContainer *servant;
	CORBA_Environment ev;
	CORBA_Object o;

	CORBA_exception_init (&ev);

	servant = (POA_Bonobo_ItemContainer *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &item_handler_vepv;

	POA_Bonobo_ItemContainer__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &ev);

	if (o) {
		bonobo_object_bind_to_servant (object, servant);
		CORBA_exception_free (&ev);
		return o;
	} else {
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}
}

/*
 * Returns a list of the objects in this container
 */
static Bonobo_ItemContainer_ObjectNames *
impl_enum_objects (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboItemHandler *handler = BONOBO_ITEM_HANDLER (object);

	if (handler->enum_objects)
		return handler->enum_objects (handler, handler->user_data, ev);
	else
		return Bonobo_ItemContainer_ObjectNames__alloc ();
}

static Bonobo_Unknown
impl_get_object (PortableServer_Servant servant,
		 const CORBA_char      *item_name,
		 CORBA_boolean          only_if_exists,
		 CORBA_Environment     *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboItemHandler *handler = BONOBO_ITEM_HANDLER (object);

	if (handler->get_object)
		return handler->get_object (handler, item_name,
					    only_if_exists,
					    handler->user_data, ev);
	else
		return CORBA_OBJECT_NIL;
}

/*
 * BonoboItemHandler CORBA vector-class initialization routine
 */

/**
 * bonobo_item_handler_get_epv:
 *
 * Returns: The EPV for the default BonoboItemHandler implementation.  
 */
POA_Bonobo_ItemContainer__epv *
bonobo_item_handler_get_epv (void)
{
	POA_Bonobo_ItemContainer__epv *epv;

	epv = g_new0 (POA_Bonobo_ItemContainer__epv, 1);

	epv->enumObjects     = impl_enum_objects;
	epv->getObjectByName = impl_get_object;

	return epv;
}

static void
corba_item_handler_class_init (void)
{
	/* Init the vepv */
	item_handler_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	item_handler_vepv.Bonobo_ItemContainer_epv = bonobo_item_handler_get_epv ();
}

/*
 * BonoboItemContainer class initialization routine
 */
static void
bonobo_item_handler_class_init (BonoboItemHandlerClass *container_class)
{
	corba_item_handler_class_init ();
}

/**
 * bonobo_item_handler_construct:
 * @container: The handler object to construct
 * @corba_container: The CORBA object that implements Bonobo::ItemContainer
 *
 * Constructs the @container Gtk object using the provided CORBA
 * object.
 *
 * Returns: The constructed BonoboItemContainer object.
 */
BonoboItemHandler *
bonobo_item_handler_construct (BonoboItemHandler  *handler,
			       Bonobo_ItemContainer corba_handler,
			       BonoboItemHandlerEnumObjectsFn enum_objects,
			       BonoboItemHandlerGetObjectFn get_object,
			       gpointer user_data)
{
	g_return_val_if_fail (handler != NULL, NULL);
	g_return_val_if_fail (corba_handler != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (BONOBO_IS_ITEM_HANDLER (handler), NULL);
	
	bonobo_object_construct (BONOBO_OBJECT (handler), (CORBA_Object) corba_handler);

	handler->get_object   = get_object;
	handler->enum_objects = enum_objects;
	handler->user_data    = user_data;
	
	return handler;
}

/**
 * bonobo_item_handler_get_type:
 *
 * Returns: The GtkType for the BonoboItemContainer class.
 */
GtkType
bonobo_item_handler_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboItemHandler",
			sizeof (BonoboItemHandler),
			sizeof (BonoboItemHandlerClass),
			(GtkClassInitFunc) bonobo_item_handler_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_item_handler_new:
 *
 * Creates a new BonoboItemHandler object.  These are used to hold
 * client sites.
 *
 * Returns: The newly created BonoboItemHandler object
 */
BonoboItemHandler *
bonobo_item_handler_new (BonoboItemHandlerEnumObjectsFn enum_objects,
			 BonoboItemHandlerGetObjectFn   get_object,
			 gpointer                       user_data)

{
	BonoboItemHandler *handler;
	Bonobo_ItemContainer corba_handler;

	handler = gtk_type_new (bonobo_item_handler_get_type ());
	corba_handler = create_bonobo_item_handler (BONOBO_OBJECT (handler));

	if (corba_handler == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (handler));
		return NULL;
	}
	
	return bonobo_item_handler_construct (
		handler, corba_handler,
		enum_objects, get_object, user_data);
}

/**
 * bonobo_parse_item_options:
 * @option_string: a string with a list of options
 *
 * The bonobo_parse_item_options() routine parses the
 * @option_string which is a semi-colon separated list
 * of arguments.
 *
 * Each argument is of the form value[=key].  The entire
 * option string is defined by:
 *
 * option_string := keydef
 *                | keydef ; option_string
 *
 * keydef := value [=key]
 *
 * The key can be literal values, values with spaces, and the
 * \ character is used as an escape sequence.  To include a
 * literal ";" in a value you can use \;.
 *
 * Returns: A GSList that contains structures of type BonoboItemOption
 * each BonoboItemOption
 */
GSList *
bonobo_item_option_parse (const char *option_string)
{
	GSList *list = NULL;
	GString *key = NULL;
	BonoboItemOption *option = NULL;
	const char *p;
	
	for (p = option_string; *p; p++) {
		if (*p == '=' ) {
			GString *value = NULL;
			if (!key)
				return list;
			
			option = g_new0 (BonoboItemOption, 1);
			option->key = key->str;
			g_string_free (key, FALSE);
			key = NULL;

			for (p++; *p; p++) {
				if (*p == ';')
					goto next;
				if (!value)
					value = g_string_new ("");

				if (*p == '\\') {
					p++;
					if (*p == 0)
						break;
					g_string_append_c (value, *p);
					continue;
				}
				g_string_append_c (value, *p);
			}
		next:
			if (value) {
				option->value = value->str;
				g_string_free (value, FALSE);
			}
			list = g_slist_append (list, option);
			if (*p == 0)
				break;
		} else {
			if (key == NULL)
				key = g_string_new ("");
			g_string_append_c (key, *p);
		}
	}

	if (key) {
		BonoboItemOption *option = g_new (BonoboItemOption, 1);
		
		option->key = key->str;
		g_string_free (key, FALSE);
		
		list = g_slist_append (list, option);
	}
	
	return list;
}

/** 
 * bonobo_item_options_free:
 * @options: a GSList of BonoboItemOption structures that was returned by bonobo_item_option_parse()
 *
 * Use this to release a list returned by bonobo_item_option_parse()
 */
void
bonobo_item_options_free (GSList *options)
{
	GSList *l;

	for (l = options; l; l = l->next) {
		BonoboItemOption *option = l->data;

		g_free (option->key);
		if (option->value)
			g_free (option->value);
	}

	g_slist_free (options);
}
