/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME container object.
 *
 * The GnomeContainer object represents a document which may have one
 * or more embedded document objects.  To embed an object in the
 * container, create a GnomeClientSite, add it to the container, and
 * then create an object supporting GNOME::Embeddable and bind it to
 * the client site.  The GnomeContainer maintains a list of the client
 * sites which correspond to the objects embedded in the container.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999 International GNOME Support (http://www.gnome-support.com)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkwidget.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-object.h>
#include <bonobo/gnome-container.h>
#include <bonobo/gnome-client-site.h>

enum {
	GET_OBJECT,
	LAST_SIGNAL
};

static guint signals [LAST_SIGNAL] = { 0, };

static GnomeObjectClass *gnome_container_parent_class;

POA_GNOME_ParseDisplayName__epv gnome_parse_display_name_epv = { NULL, };
POA_GNOME_Container__epv gnome_container_epv = { NULL, };
POA_GNOME_Container__vepv gnome_container_vepv;

static CORBA_Object
create_gnome_container (GnomeObject *object)
{
	POA_GNOME_Container *servant;
	CORBA_Object o;
	
	servant = (POA_GNOME_Container *) g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_container_vepv;

	POA_GNOME_Container__init ((PortableServer_Servant) servant, &object->ev);
	if (object->ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &object->ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &object->ev);

	if (o){
		gnome_object_bind_to_servant (object, servant);
		return o;
	} else
		return CORBA_OBJECT_NIL;
}

static void
gnome_container_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (gnome_container_parent_class)->destroy (object);
}

/*
 * Returns a list of the objects in this container
 */
static GNOME_Container_ObjectList *
impl_enum_objects (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeContainer *container = GNOME_CONTAINER (object);
	GNOME_Container_ObjectList *return_list;
	int items;
	GList *l;
	int i;
	
	return_list = GNOME_Container_ObjectList__alloc ();
	if (return_list == NULL)
		return NULL;

	items = g_list_length (container->client_sites);
	
	return_list->_buffer = CORBA_sequence_GNOME_Unknown_allocbuf (items);
	if (return_list->_buffer == NULL){
		CORBA_free (return_list);
		return NULL;
	}
	
	return_list->_length = items;
	return_list->_maximum = items;

	/*
	 * Assemble the list of objects
	 */
	for (i = 0, l = container->client_sites; l; l = l->next, i++){
		GnomeClientSite *client_site = GNOME_CLIENT_SITE (l->data);
		GnomeObject *bound_object = GNOME_OBJECT (client_site->bound_object);

		return_list->_buffer [i] = CORBA_Object_duplicate (
			gnome_object_corba_objref (bound_object), ev);
	}

	return return_list;
}

static GNOME_Unknown
impl_get_object (PortableServer_Servant servant,
		 CORBA_char *item_name,
		 CORBA_boolean only_if_exists,
		 CORBA_Environment * ev)
{
	GNOME_Unknown ret;
	
	gtk_signal_emit (
		GTK_OBJECT (gnome_object_from_servant (servant)),
		signals [GET_OBJECT], item_name, only_if_exists, ev, &ret);

	return ret;
}

/*
 * GnomeContainer CORBA vector-class initialization routine
 */
static void
corba_container_class_init (void)
{
	/* Init the epv */
	gnome_container_epv.enum_objects = impl_enum_objects;
	gnome_container_epv.get_object = impl_get_object;

	/* Init the vepv */
	gnome_container_vepv._base_epv     = &gnome_object_base_epv;
	gnome_container_vepv.GNOME_Unknown_epv = &gnome_object_epv;
	gnome_container_vepv.GNOME_ParseDisplayName_epv = &gnome_parse_display_name_epv;
	gnome_container_vepv.GNOME_Container_epv = &gnome_container_epv;
}

typedef GNOME_Unknown (*GnomeSignal_POINTER__POINTER_BOOL_POINTER) (
	GnomeContainer *item_container,
	CORBA_char *item_name,
	CORBA_boolean only_if_exists,
	CORBA_Environment *ev,
	gpointer func_data);

static void 
gnome_marshal_POINTER__POINTER_BOOL_POINTER (GtkObject * object,
					     GtkSignalFunc func,
					     gpointer func_data,
					     GtkArg * args)
{
	GnomeSignal_POINTER__POINTER_BOOL_POINTER rfunc;
	void **return_val;
	return_val = GTK_RETLOC_POINTER (args[3]);
	rfunc = (GnomeSignal_POINTER__POINTER_BOOL_POINTER) func;
	*return_val = (*rfunc) (GNOME_CONTAINER (object),
				GTK_VALUE_POINTER (args[0]),
				GTK_VALUE_BOOL (args[1]),
				GTK_VALUE_POINTER (args[2]),
				func_data);
}
/*
 * GnomeContainer class initialization routine
 */
static void
gnome_container_class_init (GnomeContainerClass *container_class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) container_class;

	gnome_container_parent_class = gtk_type_class (gnome_object_get_type ());

	object_class->destroy = gnome_container_destroy;

	signals [GET_OBJECT] =
		gtk_signal_new  (
			"get_object",
			GTK_RUN_LAST,
			object_class->type,
			0,
			gnome_marshal_POINTER__POINTER_BOOL_POINTER,
			GTK_TYPE_POINTER,
			3,
			GTK_TYPE_POINTER, GTK_TYPE_BOOL, GTK_TYPE_POINTER);
	gtk_object_class_add_signals (object_class, signals, LAST_SIGNAL);

	corba_container_class_init ();
}

/*
 * GnomeContainer instance initialization routine
 */
static void
gnome_container_init (GnomeContainer *container)
{
}

/**
 * gnome_container_construct:
 * @container: The container object to construct
 * @corba_container: The CORBA object that implements GNOME::Container
 *
 * Constructs the @container Gtk object using the provided CORBA
 * object.
 *
 * Returns: The constructed GnomeContainer object.
 */
GnomeContainer *
gnome_container_construct (GnomeContainer  *container,
			   GNOME_Container corba_container)
{
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CONTAINER (container), NULL);
	g_return_val_if_fail (corba_container != CORBA_OBJECT_NIL, NULL);
	
	gnome_object_construct (GNOME_OBJECT (container), (CORBA_Object) corba_container);

	return container;
}

/**
 * gnome_container_get_type:
 *
 * Returns: The GtkType for the GnomeContainer class.
 */
GtkType
gnome_container_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Container:1.0",
			sizeof (GnomeContainer),
			sizeof (GnomeContainerClass),
			(GtkClassInitFunc) gnome_container_class_init,
			(GtkObjectInitFunc) gnome_container_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}

/**
 * gnome_container_new:
 *
 * Creates a new GnomeContainer object.  These are used to hold
 * client sites.
 *
 * Returns: The newly created GnomeContainer object
 */
GnomeContainer *
gnome_container_new (void)
{
	GnomeContainer *container;
	GNOME_Container corba_container;

	container = gtk_type_new (gnome_container_get_type ());
	corba_container = create_gnome_container (GNOME_OBJECT (container));

	if (corba_container == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (container));
		return NULL;
	}
	
	return gnome_container_construct (container, corba_container);
}

/**
 * gnome_container_get_moniker:
 * @container: A GnomeContainer object.
 */
GnomeMoniker *
gnome_container_get_moniker (GnomeContainer *container)
{
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CONTAINER (container), NULL);

	return container->moniker;
}

/**
 * gnome_container_add:
 * @container: The object to operate on.
 * @client_site: The client site to add to the container
 *
 * Adds the @client_site to the list of client-sites managed by this
 * container
 */
void
gnome_container_add (GnomeContainer *container, GnomeObject *client_site)
{
	g_return_if_fail (container != NULL);
	g_return_if_fail (client_site != NULL);
	g_return_if_fail (GNOME_IS_CONTAINER (container));
	g_return_if_fail (GNOME_IS_OBJECT (client_site));

	gtk_object_ref (GTK_OBJECT (client_site));
	container->client_sites = g_list_prepend (container->client_sites, client_site);
}

/**
 * gnome_container_remove:
 * @container: The object to operate on.
 * @client_site: The client site to remove from the container
 *
 * Removes the @client_site from the @container
 */
void
gnome_container_remove (GnomeContainer *container, GnomeObject *client_site)
{
	g_return_if_fail (container != NULL);
	g_return_if_fail (client_site != NULL);
	g_return_if_fail (GNOME_IS_CONTAINER (container));
	g_return_if_fail (GNOME_IS_OBJECT (client_site));

	container->client_sites = g_list_remove (container->client_sites, client_site);
	gtk_object_unref (GTK_OBJECT (client_site));
}

