/*
 * gnome-object-client.c:
 *   This handles the client-view of a remote GNOME object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <bonobo/gnome-object-client.h>

static GnomeObjectClass *gnome_object_client_parent_class;

/**
 * gnome_object_client_construct:
 * @object_client: The GnomeObjectClient object to construct
 * @corba_object: The remote CORBA object this object_client refers to
 *
 * Constructs @object_client
 */
GnomeObjectClient *
gnome_object_client_construct (GnomeObjectClient *object_client, CORBA_Object corba_object)
{
	g_return_val_if_fail (object_client != NULL, NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (GNOME_IS_OBJECT_CLIENT (object_client), NULL);
	
	GNOME_OBJECT (object_client)->object = corba_object;

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

static void
gnome_object_client_destroy (GtkObject *object)
{
	GnomeObject *gnome_object = GNOME_OBJECT (object);
	GnomeObjectClient *gnome_object_client = GNOME_OBJECT_CLIENT (object);

	GNOME_obj_unref (gnome_object->object, &gnome_object->ev);
	GTK_OBJECT_CLASS (gnome_object_client_parent_class)->destroy (object);
}

static void
gnome_object_client_class_init (GnomeObjectClientClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_object_client_parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = gnome_object_client_destroy;
}

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
