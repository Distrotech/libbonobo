/*
 * gnome-object-client.c:
 *   This handles the client-view of a remote GNOME object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <bonobo/gnome-object-client.h>

static GnomeUnknownClass *gnome_unknown_client_parent_class;

/**
 * gnome_unknown_client_construct:
 * @object_client: The GnomeUnknownClient object to construct
 * @corba_object: The remote CORBA object this object_client refers to
 *
 * Constructs @object_client
 */
GnomeUnknownClient *
gnome_unknown_client_construct (GnomeUnknownClient *object_client, CORBA_Object corba_object)
{
	g_return_val_if_fail (object_client != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_UNKNOWN_CLIENT (object_client), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);
	
	GNOME_UNKNOWN (object_client)->object = corba_object;

	return object_client;
}

/**
 * gnome_unknown_activate_with_repo_id:
 * @list: Preloaded list of servers or NULL.
 * @repo_id: CORBA interface repository id that we want to activate.
 * @flags: Goad activation flags.
 * @params: parameters passed to the factory
 *
 * Activates a service and wraps it on the GnomeUnknownClient object.
 * The service activated should support the GNOME::object interface.
 *
 * This routine used goad_server_activate_with_repo_id() routine from
 * libgnorba.
 */
GnomeUnknownClient *
gnome_unknown_activate_with_repo_id (GoadServerList *list,
				    const char *repo_id,
				    GoadActivationFlags flags,
				    const char **params)
{
	CORBA_Object corba_object;
	GnomeUnknownClient *object;

	corba_object = goad_server_activate_with_repo_id (NULL, repo_id, 0, NULL);
	if (corba_object == CORBA_OBJECT_NIL)
		return NULL;
	
	object = gtk_type_new (gnome_unknown_client_get_type ());
	gnome_unknown_client_construct (object, corba_object);

	return object;
}

/**
 * gnome_unknown_activate_with_goad_id:
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
GnomeUnknownClient *
gnome_unknown_activate_with_goad_id (GoadServerList *list,
				    const char *goad_id,
				    GoadActivationFlags flags,
				    const char **params)
{
	CORBA_Object corba_object;
	GnomeUnknownClient *object;

	corba_object = goad_server_activate_with_id (NULL, goad_id, 0, NULL);
	if (corba_object == CORBA_OBJECT_NIL)
		return NULL;
	
	object = gtk_type_new (gnome_unknown_client_get_type ());
	gnome_unknown_client_construct (object, corba_object);

	return object;
}

static void
gnome_unknown_client_destroy (GtkObject *object)
{
	GnomeUnknown *gnome_unknown = GNOME_UNKNOWN (object);
	GnomeUnknownClient *gnome_unknown_client = GNOME_UNKNOWN_CLIENT (object);

	GNOME_Unknown_unref (gnome_unknown->object, &gnome_unknown->ev);
	GTK_OBJECT_CLASS (gnome_unknown_client_parent_class)->destroy (object);
}

static void
gnome_unknown_client_class_init (GnomeUnknownClientClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_unknown_client_parent_class = gtk_type_class (gnome_unknown_get_type ());

	object_class->destroy = gnome_unknown_client_destroy;
}

GtkType
gnome_unknown_client_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"Handle to remote GNOME::Object",
			sizeof (GnomeUnknownClient),
			sizeof (GnomeUnknownClientClass),
			(GtkClassInitFunc) gnome_unknown_client_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_unknown_get_type (), &info);
	}

	return type;
}
