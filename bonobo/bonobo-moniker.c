/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * GNOME Moniker
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-moniker.h>
#include <string.h>

GtkObject *bonobo_moniker_parent_class;
/**
 * bonobo_moniker_set_server:
 * @moniker: the moniker on which we act.
 * @goadid: a GOAD ID for the server
 * @filename: the url on which this
 */
void
bonobo_moniker_set_server (BonoboMoniker *moniker, const char *goadid, const char *url)
{
	g_return_if_fail (moniker != NULL);
	g_return_if_fail (BONOBO_IS_MONIKER (moniker));
	g_return_if_fail (goadid != NULL);
	g_return_if_fail (url != NULL);

	if (moniker->goadid)
		g_free (moniker->goadid);
	moniker->goadid = g_strdup (goadid);

	if (moniker->url)
		g_free (moniker->url);
	moniker->url = g_strdup (url);
}
	
/**
 * bonobo_moniker_append_item_name:
 * @moniker: the moniker on which we act.
 * @item_name: a string describing the item to append.
 */
void
bonobo_moniker_append_item_name (BonoboMoniker *moniker, const char *item_name)
{
	g_return_if_fail (moniker != NULL);
	g_return_if_fail (BONOBO_IS_MONIKER (moniker));
	g_return_if_fail (item_name != NULL);

	moniker->items = g_list_prepend (moniker->items, g_strdup (item_name));
}

/*
 * Escapes a strings for use in a moniker
 */
static char *
escape (const char *str)
{
	const char *p;
	char *q, *res;
	int len = 0;

	for (p = str; *p; p++){
		if (*p == ',' || *p == '\\')
			len++;
		len++;
	}
	len++;
	
	res = q = g_malloc (len);

	if (!res)
		return NULL;
		
	for (p = str; *p; p++){
		if (*p == ',' || *p == '\\')
			*q++ = '\\';
		*q++ = *p;
	}
	*q = 0;

	return res;
}

/**
 * bonobo_moniker_get_as_string:
 * @moniker: the moniker object we operate on.
 *
 * Returns a textual representation of the moniker @moniker.  A %NULL
 * is returned on any errors encountered.
 */
char *
bonobo_moniker_get_as_string (BonoboMoniker *moniker)
{
	int n, i;
	GList *l;
	char *res;
	char **array;
	int len;
	
	g_return_val_if_fail (moniker != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_MONIKER (moniker), NULL);

	if (moniker->goadid == NULL)
		return NULL;

	if (moniker->url == NULL)
		return NULL;
	
	n = 2 + g_list_length (moniker->items);

	array = g_new (char *, n);
	if (array == NULL)
		return NULL;

	array [0] = escape (moniker->goadid);
	array [1] = escape (moniker->url);

	for (i = 0, l = moniker->items; l; l = l->next)
		array [i+2] = escape (l->data);

	len = sizeof ("moniker_url:");
	for (i = 0; i < n; i++)
		len += strlen (array [i]) + 1;

	res = g_malloc (len);
	if (res != NULL){
		strcpy (res, "moniker_url:");
		for (i = 0; i < n; i++){
			strcat (res, array [i]);
			strcat (res, ",");
		}
	}
	for (i = 0; i < n; i++)
		g_free (array [i]);
	g_free (array);

	return res;
}

/**
 * bonobo_moniker_new:
 *
 * Creates a new BonoboMoniker object
 *
 * Returns: The newly-constructed BonoboMoniker object.
 */
BonoboMoniker *
bonobo_moniker_new (void)
{
	BonoboMoniker *moniker;

	moniker = gtk_type_new (bonobo_moniker_get_type ());

	return moniker;
}

static void
bonobo_moniker_destroy (GtkObject *object)
{
	BonoboMoniker *moniker = (BonoboMoniker *) object;
	GList *l;
	
	if (moniker->goadid)
		g_free (moniker->goadid);
	if (moniker->url)
		g_free (moniker->url);

	for (l = moniker->items; l; l = l->next){
		char *string = l->data;

		g_free (string);
	}
	g_list_free (moniker->items);
	GTK_OBJECT_CLASS (bonobo_moniker_parent_class)->destroy(object);
}

static void
bonobo_moniker_class_init (GtkObjectClass *object_class)
{
	bonobo_moniker_parent_class = gtk_type_class (gtk_object_get_type ());
	
	object_class->destroy = bonobo_moniker_destroy;
}

/**
 * bonobo_moniker_get_type:
 *
 * Returns: The GtkType of the BonoboMoniker class.
 */
GtkType
bonobo_moniker_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboMoniker",
			sizeof (BonoboMoniker),
			sizeof (BonoboMonikerClass),
			(GtkClassInitFunc) bonobo_moniker_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}


