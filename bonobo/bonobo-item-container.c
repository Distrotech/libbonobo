/**
 * GNOME container object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtkwidget.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-object.h>
#include <bonobo/gnome-container.h>

static GnomeObjectClass *gnome_container_parent_class;

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

/**
 * gnome_container_get_moniker:
 * @container: A GnomeContainer object whose moniker is being requested.
 *
 * Returns: The GnomeMoniker associated with @container.
 */
GnomeMoniker *
gnome_container_get_moniker (GnomeContainer *container)
{
	g_error ("Implement me");
	return NULL;
}

static void
gnome_container_destroy (GtkObject *object)
{
	GnomeContainer *container = GNOME_CONTAINER (object);

	GTK_OBJECT_CLASS (gnome_container_parent_class)->destroy (object);
}

static void
corba_container_class_init (void)
{
	/* Init the epv */
	    /* nothing just yet */
	
	/* Init the vepv */
	gnome_container_vepv._base_epv     = &gnome_object_base_epv;
	gnome_container_vepv.GNOME_Unknown_epv = &gnome_object_epv;
	gnome_container_vepv.GNOME_Container_epv = &gnome_container_epv;
}

static void
gnome_container_class_init (GnomeContainerClass *container_class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) container_class;

	gnome_container_parent_class = gtk_type_class (gnome_object_get_type ());

	object_class->destroy = gnome_container_destroy;

	corba_container_class_init ();
}

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
GnomeObject *
gnome_container_construct (GnomeContainer  *container,
			   GNOME_Container corba_container)
{
	g_return_val_if_fail (container != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CONTAINER (container), NULL);
	g_return_val_if_fail (corba_container != CORBA_OBJECT_NIL, NULL);
	
	gnome_object_construct (GNOME_OBJECT (container), (CORBA_Object) corba_container);

	return GNOME_OBJECT (container);
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
 * Returns: The new created GnomeContainer object
 */
GnomeObject *
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

