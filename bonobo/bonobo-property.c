/*
 * Welcome to my world.
 *
 * Author:
 *    Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */

#include <config.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-property-bag.h>
#include <bonobo/gnome-property.h>

typedef struct {
	POA_GNOME_Property		 prop;
	GnomePropertyBag		*pb;
	char				*property_name;
} GnomePropertyServant;

static CORBA_char *
impl_GNOME_Property_get_name (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;

	return CORBA_string_dup (pservant->property_name);
}

static CORBA_TypeCode
impl_GNOME_Property_get_type (PortableServer_Servant servant,
			       CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;
	const char *type;
	CORBA_any *any;
	gpointer value;

	g_warning ("Improperly implemented GnomeProperty method: get_type\n");

	type = gnome_property_bag_get_prop_type (pservant->pb, pservant->property_name);
	value = gnome_property_bag_get_value (pservant->pb, pservant->property_name);

	any = gnome_property_bag_value_to_any (pservant->pb, type, value);

	return any->_type;
}


static CORBA_any *
impl_GNOME_Property_get_value (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;
	const char *type;
	gpointer value;

	type = gnome_property_bag_get_prop_type (pservant->pb, pservant->property_name);
	value = gnome_property_bag_get_value (pservant->pb, pservant->property_name);

	return gnome_property_bag_value_to_any (pservant->pb, type, value);
}

static void
impl_GNOME_Property_set_value (PortableServer_Servant servant,
				CORBA_any *any,
				CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;
	gpointer new_value;
	const char *type;

	type = gnome_property_bag_get_prop_type (pservant->pb, pservant->property_name);
	new_value = gnome_property_bag_any_to_value (pservant->pb, type, any);

	gnome_property_bag_set_value (pservant->pb, pservant->property_name, new_value);
}

static CORBA_any *
impl_GNOME_Property_get_default (PortableServer_Servant servant,
				  CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;
	gpointer default_value;
	const char *type;

	type = gnome_property_bag_get_prop_type (pservant->pb, pservant->property_name);
	default_value = gnome_property_bag_get_default (pservant->pb, pservant->property_name);

	return gnome_property_bag_value_to_any (pservant->pb, type, default_value);
}

static CORBA_char *
impl_GNOME_Property_get_doc_string (PortableServer_Servant servant,
				     CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;

	return CORBA_string_dup (gnome_property_bag_get_docstring (pservant->pb, pservant->property_name));
}


static CORBA_boolean
impl_GNOME_Property_is_stored (PortableServer_Servant servant,
				CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;
	GnomePropertyFlags flags;

	flags = gnome_property_bag_get_flags (pservant->pb, pservant->property_name);

	return (CORBA_boolean) ((flags & GNOME_PROPERTY_STORED) != 0);
}

static CORBA_boolean
impl_GNOME_Property_is_read_only (PortableServer_Servant servant,
				   CORBA_Environment *ev)
{
	GnomePropertyServant *pservant = (GnomePropertyServant *) servant;
	GnomePropertyFlags flags;

	flags = gnome_property_bag_get_flags (pservant->pb, pservant->property_name);

	return (CORBA_boolean) ((flags & GNOME_PROPERTY_READ_ONLY) != 0);

}

static POA_GNOME_Property__epv *
gnome_property_get_epv (void)
{
	static POA_GNOME_Property__epv *epv = NULL;

	if (epv != NULL)
		return epv;

	epv = g_new0 (POA_GNOME_Property__epv, 1);

	epv->get_name       = impl_GNOME_Property_get_name;
	epv->get_type       = impl_GNOME_Property_get_type;
	epv->get_value      = impl_GNOME_Property_get_value;
	epv->set_value      = impl_GNOME_Property_set_value;
	epv->get_default    = impl_GNOME_Property_get_default;
	epv->get_doc_string = impl_GNOME_Property_get_doc_string;
	epv->is_stored	    = impl_GNOME_Property_is_stored;
	epv->is_read_only   = impl_GNOME_Property_is_read_only;

	return epv;
}

static POA_GNOME_Property__vepv *
gnome_property_get_vepv (void)
{
	static POA_GNOME_Property__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_GNOME_Property__vepv, 1);

	vepv->GNOME_Property_epv = gnome_property_get_epv ();

	return vepv;
}

PortableServer_Servant
gnome_property_servant_new (PortableServer_POA adapter, GnomePropertyBag *pb,
			     char *property_name)
{
	GnomePropertyServant	*servant;
	CORBA_Environment        ev;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (property_name != NULL, NULL);

	/*
	 * Verify that the specified property exists.
	 */
	if (! gnome_property_bag_has_property (pb, property_name))
		return NULL;


	CORBA_exception_init (&ev);

	/*
	 * Create a transient servant for the property.
	 */
	servant = g_new0 (GnomePropertyServant, 1);

	servant->property_name = g_strdup (property_name);
	servant->pb = pb;

	((POA_GNOME_Property *) servant)->vepv = gnome_property_get_vepv ();
	
	POA_GNOME_Property__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomeProperty: Could not initialize Property servant");
		g_free (servant->property_name);
		g_free (servant);
		CORBA_exception_free (&ev);
		return NULL;
	}

	CORBA_exception_free (&ev);

	return servant;
}

void
gnome_property_servant_destroy (PortableServer_Servant servant)
{
	CORBA_Environment ev;

	g_return_if_fail (servant != NULL);

	CORBA_exception_init (&ev);

	POA_GNOME_Property__fini ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomeProperty: Could not deconstruct Property servant");
		CORBA_exception_free (&ev);
		return;
	}

	CORBA_exception_free (&ev);

	g_free (((GnomePropertyServant *) servant)->property_name);
	g_free (servant);
}
