/*
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/gnome-property-bag-client.h>

static GnomePropertyBagClient *
gnome_property_bag_client_construct (GnomePropertyBagClient *pbc,
				     GNOME_PropertyBag corba_pb)
{
	pbc->corba_pb = corba_pb;

	return pbc;
}

/**
 * gnome_property_bag_client_new:
 * @corba_property_bag: A CORBA object reference for a remote
 * #GNOME_PropertyBag.
 *
 * Returns: A new #GnomePropertyBagClient object which can
 * be used to access @corba_property_bag.  You can, of course,
 * interact with @corba_property_bag directly using CORBA; this
 * object is just provided as a convenience.
 */
GnomePropertyBagClient *
gnome_property_bag_client_new (GNOME_PropertyBag corba_property_bag)
{
	GnomePropertyBagClient *pbc;

	g_return_val_if_fail (corba_property_bag != CORBA_OBJECT_NIL, NULL);

	pbc = gtk_type_new (gnome_property_bag_client_get_type ());

	return gnome_property_bag_client_construct (pbc, corba_property_bag);
}

/**
 * gnome_property_bag_client_get_properties:
 * @pbc: A #GnomePropertyBagClient which is bound to a remote
 * #GNOME_PropertyBag.
 *
 * Returns: A #GList filled with #GNOME_Property CORBA object
 * references for all of the properties stored in the remote
 * #GnomePropertyBag.
 */
GList *
gnome_property_bag_client_get_properties (GnomePropertyBagClient *pbc)
{
	GNOME_PropertyList *props;
	GList		    *prop_list;
	int		     i;
	CORBA_Environment    ev;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG_CLIENT (pbc), NULL);

	CORBA_exception_init (&ev);

	props = GNOME_PropertyBag_get_properties (pbc->corba_pb, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		gnome_object_check_env (GNOME_OBJECT (pbc), pbc->corba_pb, &ev);
		CORBA_exception_free (&ev);
		return NULL;
	}

	prop_list = NULL;
	for (i = 0; i < props->_length; i ++) {

		/*
		 * FIXME: Is it necessary to duplicate these?  I'm
		 * inclined to think that it isn't.
		 */
		prop_list = g_list_prepend (
			prop_list,
			CORBA_Object_duplicate (props->_buffer [i], &ev));
		if (ev._major != CORBA_NO_EXCEPTION) {
			CORBA_Environment ev2;
			GList *curr;

			CORBA_exception_free (&ev);

			for (curr = prop_list; curr != NULL; curr = curr->next) {
				CORBA_exception_init (&ev);
				CORBA_Object_release ((CORBA_Object) curr->data, &ev2);
				CORBA_exception_free (&ev);
			}

			g_list_free (prop_list);

			return NULL;
		}
	}

	CORBA_exception_free (&ev);

	CORBA_free (props);

	return prop_list;
}

/**
 * gnome_property_bag_client_get_property_names:
 * @pbc: A #GnomePropertyBagClient which is bound to a remote
 * #GNOME_PropertyBag.
 *
 * This function exists as a convenience, so that you don't have to
 * iterate through all of the #GNOME_Property objects in order to get
 * a list of their names.  It should be used in place of such an
 * iteration, as it uses fewer resources on the remote
 * #GnomePropertyBag.
 *
 * Returns: A #GList filled with strings containing the names of all
 * the properties stored in the remote #GnomePropertyBag.
 */
GList *
gnome_property_bag_client_get_property_names (GnomePropertyBagClient *pbc)
{
	GNOME_PropertyNames *names;
	GList		     *name_list;
	int		      i;
	CORBA_Environment     ev;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG_CLIENT (pbc), NULL);

	CORBA_exception_init (&ev);

	names = GNOME_PropertyBag_get_property_names (pbc->corba_pb, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		CORBA_exception_free (&ev);
		return NULL;
	}
	
	CORBA_exception_free (&ev);

	name_list = NULL;
	for (i = 0; i < names->_length; i ++) {
		 char *name;

		 name = g_strdup (names->_buffer [i]);
		 name_list = g_list_prepend (name_list, name);
	}

	CORBA_free (names);

	return name_list;
}

/**
 * gnome_property_bag_client_get_property:
 * @pbc: A GnomePropertyBagClient which is associated with a remote
 * GnomePropertyBag.
 * @name: A string containing the name of the property which is to
 * be fetched.
 *
 * Returns: A #GNOME_Property CORBA object reference for the
 *
 */
GNOME_Property
gnome_property_bag_client_get_property (GnomePropertyBagClient *pbc,
					 char *property_name)
{
	CORBA_Environment ev;
	GNOME_Property   prop;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG_CLIENT (pbc), NULL);

	CORBA_exception_init (&ev);

	prop = GNOME_PropertyBag_get_property (pbc->corba_pb, property_name, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return prop;
}


/*
 * Bonobo Property streaming functions.
 */

/**
 * gnome_property_bag_client_write_to_stream:
 * @pbc: A #GnomePropertyBagClient object which is bound to a remote
 * #GNOME_PropertyBag server.
 * @stream: A #GnomeStream into which the data in @pbc will be written.
 *
 * Reads the property data stored in the #GNOME_Property_bag to which
 * @pbc is bound and streams it into @stream.  The typical use for
 * this function is to save the property data for a given Bonobo
 * Control into a persistent store to which @stream is attached.
 */
void
gnome_property_bag_client_write_to_stream (GnomePropertyBagClient *pbc,
					    GnomeStream *stream)
{
	g_return_if_fail (pbc != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (stream != NULL);
	g_return_if_fail (GNOME_IS_STREAM (stream));

	g_warning ("gnome_property_bag_client_write_to_stream: Unimplemented.\n");
}

/**
 * gnome_property_bag_client_read_from_stream:
 */
void
gnome_property_bag_client_read_from_stream (GnomePropertyBagClient *pbc,
					     GnomeStream *stream)
{
	g_return_if_fail (pbc != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (stream != NULL);
	g_return_if_fail (GNOME_IS_STREAM (stream));

	g_warning ("gnome_property_bag_client_read_from_stream: Unimplemented.\n");
}


/*
 * GtkObject crap.
 */


static void
gnome_property_bag_client_class_init (GnomePropertyBagClientClass *class)
{
/*	GtkObjectClass *object_class = (GtkObjectClass *) class; */
}

static void
gnome_property_bag_client_init (GnomePropertyBagClient *pbc)
{
}

/**
 * gnome_property_bag_client_get_type:
 *
 * Returns: The GtkType corresponding to the GnomePropertyBagClient
 * class.
 */
GtkType
gnome_property_bag_client_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomePropertyBagClient",
			sizeof (GnomePropertyBagClient),
			sizeof (GnomePropertyBagClientClass),
			(GtkClassInitFunc) gnome_property_bag_client_class_init,
			(GtkObjectInitFunc) gnome_property_bag_client_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}
