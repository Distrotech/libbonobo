/*
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#include <config.h>
#include <stdarg.h>
#include <bonobo/bonobo-property-bag-client.h>
#include <bonobo/bonobo-property-types.h>

static BonoboPropertyBagClient *
bonobo_property_bag_client_construct (BonoboPropertyBagClient *pbc,
				     Bonobo_PropertyBag corba_pb)
{
	pbc->corba_pb = corba_pb;

	return pbc;
}

/**
 * bonobo_property_bag_client_new:
 * @corba_property_bag: A CORBA object reference for a remote
 * #Bonobo_PropertyBag.
 *
 * Returns: A new #BonoboPropertyBagClient object which can
 * be used to access @corba_property_bag.  You can, of course,
 * interact with @corba_property_bag directly using CORBA; this
 * object is just provided as a convenience.
 */
BonoboPropertyBagClient *
bonobo_property_bag_client_new (Bonobo_PropertyBag corba_property_bag)
{
	BonoboPropertyBagClient *pbc;

	g_return_val_if_fail (corba_property_bag != CORBA_OBJECT_NIL, NULL);

	pbc = gtk_type_new (bonobo_property_bag_client_get_type ());

	return bonobo_property_bag_client_construct (pbc, corba_property_bag);
}

/**
 * bonobo_property_bag_client_get_properties:
 * @pbc: A #BonoboPropertyBagClient which is bound to a remote
 * #Bonobo_PropertyBag.
 *
 * Returns: A #GList filled with #Bonobo_Property CORBA object
 * references for all of the properties stored in the remote
 * #BonoboPropertyBag.
 */
GList *
bonobo_property_bag_client_get_properties (BonoboPropertyBagClient *pbc)
{
	Bonobo_PropertyList  *props;
	GList		    *prop_list;
	int		     i;
	CORBA_Environment    ev;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), NULL);

	CORBA_exception_init (&ev);

	props = Bonobo_PropertyBag_get_properties (pbc->corba_pb, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		bonobo_object_check_env (BONOBO_OBJECT (pbc), pbc->corba_pb, &ev);
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
 * bonobo_property_bag_client_free_properties:
 * @list: A #GList containing Bonobo_Property corba objrefs (as
 * produced by bonobo_property_bag_client_get_properties(), for
 * example).
 *
 * Releases the CORBA Objrefs stored in @list and frees the list.
 */
void
bonobo_property_bag_client_free_properties (GList *list)
{
	GList *l;

	if (list == NULL)
		return;

	for (l = list; l != NULL; l = l->next) {
		CORBA_Environment ev;
		Bonobo_Property    prop;

		prop = (Bonobo_Property) l->data;

		CORBA_exception_init (&ev);

		CORBA_Object_release (prop, &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			g_warning ("bonobo_property_bag_client_free_properties: Exception releasing objref!");
			CORBA_exception_free (&ev);
			return;
		}

		CORBA_exception_free (&ev);
	}

	g_list_free (list);
}

/**
 * bonobo_property_bag_client_get_property_names:
 * @pbc: A #BonoboPropertyBagClient which is bound to a remote
 * #Bonobo_PropertyBag.
 *
 * This function exists as a convenience, so that you don't have to
 * iterate through all of the #Bonobo_Property objects in order to get
 * a list of their names.  It should be used in place of such an
 * iteration, as it uses fewer resources on the remote
 * #BonoboPropertyBag.
 *
 * Returns: A #GList filled with strings containing the names of all
 * the properties stored in the remote #BonoboPropertyBag.
 */
GList *
bonobo_property_bag_client_get_property_names (BonoboPropertyBagClient *pbc)
{
	Bonobo_PropertyNames  *names;
	GList		     *name_list;
	int		      i;
	CORBA_Environment     ev;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), NULL);

	CORBA_exception_init (&ev);

	names = Bonobo_PropertyBag_get_property_names (pbc->corba_pb, &ev);
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
 * bonobo_property_bag_client_get_property:
 * @pbc: A BonoboPropertyBagClient which is associated with a remote
 * BonoboPropertyBag.
 * @name: A string containing the name of the property which is to
 * be fetched.
 *
 * Returns: A #Bonobo_Property CORBA object reference for the
 *
 */
Bonobo_Property
bonobo_property_bag_client_get_property (BonoboPropertyBagClient *pbc,
					const char *property_name)
{
	CORBA_Environment ev;
	Bonobo_Property    prop;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), NULL);

	CORBA_exception_init (&ev);

	prop = Bonobo_PropertyBag_get_property (pbc->corba_pb, property_name, &ev);
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
 * bonobo_property_bag_client_persist:
 * @pbc: A #BonoboPropertyBagClient object which is bound to a remote
 * #Bonobo_PropertyBag server.
 * @stream: A #BonoboStream into which the data in @pbc will be written.
 *
 * Reads the property data stored in the #Bonobo_Property_bag to which
 * @pbc is bound and streams it into @stream.  The typical use for
 * this function is to save the property data for a given Bonobo
 * Control into a persistent store to which @stream is attached.
 */
void
bonobo_property_bag_client_persist (BonoboPropertyBagClient *pbc,
				   Bonobo_Stream stream)
{
	Bonobo_PersistStream persist;
	CORBA_Environment   ev;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (stream != NULL);
	g_return_if_fail (BONOBO_IS_STREAM (stream));

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_query_interface (pbc->corba_pb, "IDL:Bonobo/PersistStream:1.0", &ev);

	if (ev._major != CORBA_NO_EXCEPTION ||
	    persist   == CORBA_OBJECT_NIL) {
		g_warning ("BonoboPropertyBagClient: No PersistStream interface "
			   "found on remote PropertyBag!\n");
		CORBA_exception_free (&ev);
		return;
	}

	Bonobo_PersistStream_save (persist, stream, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagClient: Exception caught while persisting "
			   "remote PropertyBag!\n");
		CORBA_exception_free (&ev);
		return;
	}

	Bonobo_Unknown_unref  (persist, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagClient: Exception caught while unrefing PersistStream!\n");
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	CORBA_Object_release (persist, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagClient: Exception caught while releasing "
			   "PersistStream objref!\n");
	}

	CORBA_exception_free (&ev);
}

/**
 * bonobo_property_bag_client_depersist:
 */
void
bonobo_property_bag_client_depersist (BonoboPropertyBagClient *pbc,
				     Bonobo_Stream stream)
{
	Bonobo_PersistStream persist;
	CORBA_Environment   ev;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (stream != NULL);
	g_return_if_fail (BONOBO_IS_STREAM (stream));

	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_query_interface (pbc->corba_pb, "IDL:Bonobo/PersistStream:1.0", &ev);

	if (ev._major != CORBA_NO_EXCEPTION ||
	    persist   == CORBA_OBJECT_NIL) {
		g_warning ("BonoboPropertyBagClient: No PersistStream interface "
			   "found on remote PropertyBag!\n");
		CORBA_exception_free (&ev);
		return;
	}

	Bonobo_PersistStream_load (persist, stream, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagClient: Exception caught while persisting "
			   "remote PropertyBag!\n");
		CORBA_exception_free (&ev);
		return;
	}

	Bonobo_Unknown_unref  (persist, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagClient: Exception caught while unrefing PersistStream!\n");
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	CORBA_Object_release (persist, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagClient: Exception caught while releasing "
			   "PersistStream objref!\n");
	}

	CORBA_exception_free (&ev);
}


/*
 * Property convenience functions.
 */

/**
 * bonobo_property_bag_client_get_property_type:
 */
CORBA_TypeCode
bonobo_property_bag_client_get_property_type (BonoboPropertyBagClient *pbc,
					     const char *propname)
{
	CORBA_Environment ev;
	Bonobo_Property prop;
	CORBA_TypeCode tc;

	g_return_val_if_fail (pbc != NULL, (CORBA_TypeCode) TC_null);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), (CORBA_TypeCode) TC_null);
	g_return_val_if_fail (propname != NULL, (CORBA_TypeCode) TC_null);

	prop = bonobo_property_bag_client_get_property (pbc, propname);
	g_return_val_if_fail (prop != CORBA_OBJECT_NIL, (CORBA_TypeCode) TC_null);

	CORBA_exception_init (&ev);

	tc = Bonobo_Property_get_type (prop, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("bonobo_property_bag_client_get_property_type: Exception getting TypeCode!");

		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);

		CORBA_Object_release (prop, &ev);
		CORBA_exception_free (&ev);

		return (CORBA_TypeCode) TC_null;
	}

	CORBA_Object_release (prop, &ev);
	CORBA_exception_free (&ev);

	return tc;
}

typedef enum {
	FIELD_VALUE,
	FIELD_DEFAULT
} PropUtilFieldType;

static CORBA_any *
bonobo_property_bag_client_get_field_any (BonoboPropertyBagClient *pbc,
					 const char *propname,
					 PropUtilFieldType field)

{
	Bonobo_Property     prop;
	CORBA_Environment  ev;
	CORBA_any         *any;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), NULL);
	g_return_val_if_fail (propname != NULL, NULL);

	prop = bonobo_property_bag_client_get_property (pbc, propname);
	g_return_val_if_fail (prop != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (&ev);

	if (field == FIELD_VALUE)
		any = Bonobo_Property_get_value (prop, &ev);
	else
		any = Bonobo_Property_get_default (prop, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("bonobo_property_bag_client_get_field_any: Exception getting property value!");

		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);

		CORBA_Object_release (prop, &ev);
		CORBA_exception_free (&ev);

		return NULL;
	}

	CORBA_Object_release (prop, &ev);
	CORBA_exception_free (&ev);

	return any;
}

static gboolean
bonobo_property_bag_client_get_field_boolean (BonoboPropertyBagClient *pbc,
					     const char *propname,
					     PropUtilFieldType field)
{
	CORBA_any *any;
	gboolean   b;

	g_return_val_if_fail (pbc != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), FALSE);
	g_return_val_if_fail (propname != NULL, FALSE);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return FALSE;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_boolean, 0);

	b = *(gboolean *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return b;
}

static gshort
bonobo_property_bag_client_get_field_short (BonoboPropertyBagClient *pbc,
					   const char *propname,
					   PropUtilFieldType field)
{
	CORBA_any *any;
	gshort     s;

	g_return_val_if_fail (pbc != NULL, 0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0);
	g_return_val_if_fail (propname != NULL, 0);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return 0;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_short, 0);

	s = *(gshort *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return s;
}

static gushort
bonobo_property_bag_client_get_field_ushort (BonoboPropertyBagClient *pbc,
					    const char *propname,
					    PropUtilFieldType field)
{
	CORBA_any *any;
	gushort    s;

	g_return_val_if_fail (pbc != NULL, 0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0);
	g_return_val_if_fail (propname != NULL, 0);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return 0.0;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_ushort, 0);

	s = *(gushort *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return s;
}

static glong
bonobo_property_bag_client_get_field_long (BonoboPropertyBagClient *pbc,
					  const char *propname,
					  PropUtilFieldType field)
{
	CORBA_any *any;
	glong      l;

	g_return_val_if_fail (pbc != NULL, 0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0);
	g_return_val_if_fail (propname != NULL, 0);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return 0.0;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_long, 0);

	l = *(glong *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return l;
}

static gulong
bonobo_property_bag_client_get_field_ulong (BonoboPropertyBagClient *pbc,
					   const char *propname,
					   PropUtilFieldType field)
{
	CORBA_any *any;
	gulong     l;

	g_return_val_if_fail (pbc != NULL, 0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0);
	g_return_val_if_fail (propname != NULL, 0);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return 0.0;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_ulong, 0);

	l = *(gulong *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return l;
}

static gfloat
bonobo_property_bag_client_get_field_float (BonoboPropertyBagClient *pbc,
					   const char *propname,
					   PropUtilFieldType field)
{
	CORBA_any *any;
	gfloat     f;

	g_return_val_if_fail (pbc != NULL, 0.0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0.0);
	g_return_val_if_fail (propname != NULL, 0.0);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return 0.0;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_float, 0.0);

	f = *(gfloat *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return f;
}

static gdouble
bonobo_property_bag_client_get_field_double (BonoboPropertyBagClient *pbc,
					    const char *propname,
					    PropUtilFieldType field)
{
	CORBA_any *any;
	gdouble    d;

	g_return_val_if_fail (pbc != NULL, 0.0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0.0);
	g_return_val_if_fail (propname != NULL, 0.0);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return 0.0;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_double, 0.0);

	d = *(gdouble *) any->_value;

	CORBA_any__free (any, NULL, TRUE);

	return d;
}

static char *
bonobo_property_bag_client_get_field_string (BonoboPropertyBagClient *pbc,
					    const char *propname,
					    PropUtilFieldType field)
{
	CORBA_any *any;
	char      *str;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), NULL);
	g_return_val_if_fail (propname != NULL, NULL);

	any = bonobo_property_bag_client_get_field_any (pbc, propname, field);

	if (any == NULL)
		return NULL;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_string, NULL);

	str = g_strdup (*(char **) any->_value);

	CORBA_any__free (any, NULL, TRUE);

	return str;
}

/*
 * Querying property values.
 */
gboolean
bonobo_property_bag_client_get_value_boolean (BonoboPropertyBagClient *pbc,
					     const char *propname)
{
	return bonobo_property_bag_client_get_field_boolean (pbc, propname, FIELD_VALUE);
}

gshort
bonobo_property_bag_client_get_value_short (BonoboPropertyBagClient *pbc,
					   const char *propname)
{
	return bonobo_property_bag_client_get_field_short (pbc, propname, FIELD_VALUE);
}

gushort
bonobo_property_bag_client_get_value_ushort (BonoboPropertyBagClient *pbc,
					    const char *propname)
{
	return bonobo_property_bag_client_get_field_ushort (pbc, propname, FIELD_VALUE);
}

glong
bonobo_property_bag_client_get_value_long (BonoboPropertyBagClient *pbc,
					  const char *propname)
{
	return bonobo_property_bag_client_get_field_long (pbc, propname, FIELD_VALUE);
}

gulong
bonobo_property_bag_client_get_value_ulong (BonoboPropertyBagClient *pbc,
					   const char *propname)
{
	return bonobo_property_bag_client_get_field_ulong (pbc, propname, FIELD_VALUE);
}

gfloat
bonobo_property_bag_client_get_value_float (BonoboPropertyBagClient *pbc,
					   const char *propname)
{
	return bonobo_property_bag_client_get_field_float (pbc, propname, FIELD_VALUE);
}

gdouble
bonobo_property_bag_client_get_value_double (BonoboPropertyBagClient *pbc,
					    const char *propname)
{
	return bonobo_property_bag_client_get_field_double (pbc, propname, FIELD_VALUE);
}

char *
bonobo_property_bag_client_get_value_string (BonoboPropertyBagClient *pbc,
					    const char *propname)
{
	return bonobo_property_bag_client_get_field_string (pbc, propname, FIELD_VALUE);
}

CORBA_any *
bonobo_property_bag_client_get_value_any (BonoboPropertyBagClient *pbc,
					 const char *propname)
{
	return bonobo_property_bag_client_get_field_any (pbc, propname, FIELD_VALUE);
}


/*
 * Querying property default values.
 */
gboolean
bonobo_property_bag_client_get_default_boolean (BonoboPropertyBagClient *pbc,
					       const char *propname)
{
	return bonobo_property_bag_client_get_field_boolean (pbc, propname, FIELD_DEFAULT);
}

gshort
bonobo_property_bag_client_get_default_short (BonoboPropertyBagClient *pbc,
					     const char *propname)
{
	return bonobo_property_bag_client_get_field_short (pbc, propname, FIELD_DEFAULT);
}

gushort
bonobo_property_bag_client_get_default_ushort (BonoboPropertyBagClient *pbc,
					      const char *propname)
{
	return bonobo_property_bag_client_get_field_ushort (pbc, propname, FIELD_DEFAULT);
}


glong
bonobo_property_bag_client_get_default_long (BonoboPropertyBagClient *pbc,
					    const char *propname)
{
	return bonobo_property_bag_client_get_field_long (pbc, propname, FIELD_DEFAULT);
}

gulong
bonobo_property_bag_client_get_default_ulong (BonoboPropertyBagClient *pbc,
					     const char *propname)
{
	return bonobo_property_bag_client_get_field_ulong (pbc, propname, FIELD_DEFAULT);
}

gfloat
bonobo_property_bag_client_get_default_float (BonoboPropertyBagClient *pbc,
					     const char *propname)
{
	return bonobo_property_bag_client_get_field_float (pbc, propname, FIELD_DEFAULT);
}

gdouble
bonobo_property_bag_client_get_default_double (BonoboPropertyBagClient *pbc,
					      const char *propname)
{
	return bonobo_property_bag_client_get_field_double (pbc, propname, FIELD_DEFAULT);
}

char *
bonobo_property_bag_client_get_default_string (BonoboPropertyBagClient *pbc,
					      const char *propname)
{
	return bonobo_property_bag_client_get_field_string (pbc, propname, FIELD_DEFAULT);
}

CORBA_any *
bonobo_property_bag_client_get_default_any (BonoboPropertyBagClient *pbc,
					   const char *propname)
{
	return bonobo_property_bag_client_get_field_any (pbc, propname, FIELD_DEFAULT);
}

/* Setting property values. */

void
bonobo_property_bag_client_set_value_any (BonoboPropertyBagClient *pbc,
					 const char *propname,
					 CORBA_any *value)
{
	Bonobo_Property    prop;
	CORBA_Environment ev;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);
	g_return_if_fail (value != NULL);

	prop = bonobo_property_bag_client_get_property (pbc, propname);
	g_return_if_fail (prop != CORBA_OBJECT_NIL);

	CORBA_exception_init (&ev);

	Bonobo_Property_set_value (prop, value, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("bonobo_property_bag_client_set_value_any: Exception setting property!");
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	CORBA_Object_release (prop, &ev);
	CORBA_exception_free (&ev);

	return;
}

void
bonobo_property_bag_client_set_value_boolean (BonoboPropertyBagClient *pbc,
					     const char *propname,
					     gboolean value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_boolean ("boolean", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_short (BonoboPropertyBagClient *pbc,
					   const char *propname,
					   gshort value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_short ("short", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_ushort (BonoboPropertyBagClient *pbc,
					    const char *propname,
					    gushort value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_ushort ("ushort", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_long (BonoboPropertyBagClient *pbc,
					  const char *propname,
					  glong value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_long ("long", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_ulong (BonoboPropertyBagClient *pbc,
					   const char *propname,
					   gulong value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_ulong ("ulong", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_float (BonoboPropertyBagClient *pbc,
					   const char *propname,
					   gfloat value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_float ("float", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_double (BonoboPropertyBagClient *pbc,
					    const char *propname,
					    gdouble value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_double ("double", (gconstpointer) &value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

void
bonobo_property_bag_client_set_value_string (BonoboPropertyBagClient *pbc,
					     const char *propname,
					     const char *value)
{
	CORBA_any      *any;

	g_return_if_fail (pbc != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc));
	g_return_if_fail (propname != NULL);

	any = bonobo_property_marshal_string ("string", value, NULL);
	g_return_if_fail (any != NULL);

	bonobo_property_bag_client_set_value_any (pbc, propname, any);

	CORBA_any__free (any, NULL, TRUE);
}

/*
 * Querying other fields and flags.
 */
char *
bonobo_property_bag_client_get_docstring (BonoboPropertyBagClient *pbc,
					 const char *propname)
{
	CORBA_Environment  ev;
	Bonobo_Property     prop;
	CORBA_char        *docstr;

	g_return_val_if_fail (pbc != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), NULL);
	g_return_val_if_fail (propname != NULL, NULL);

	prop = bonobo_property_bag_client_get_property (pbc, propname);
	g_return_val_if_fail (prop != CORBA_OBJECT_NIL, NULL);

	CORBA_exception_init (&ev);

	docstr = Bonobo_Property_get_doc_string (prop, &ev);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("bonobo_property_bag_client_get_doc_string: Exception getting doc string!");

		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);

		CORBA_Object_release (prop, &ev);
		CORBA_exception_free (&ev);

		return NULL;
	}

	CORBA_exception_free (&ev);

	return (char *) docstr;
}

BonoboPropertyFlags
bonobo_property_bag_client_get_flags (BonoboPropertyBagClient *pbc,
				     const char *propname)
{
	BonoboPropertyFlags flags;
	Bonobo_Property     prop;
	CORBA_Environment  ev;
	gboolean           is_read_only;

	g_return_val_if_fail (pbc != NULL, 0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG_CLIENT (pbc), 0);
	g_return_val_if_fail (propname != NULL, 0);

	prop = bonobo_property_bag_client_get_property (pbc, propname);
	g_return_val_if_fail (prop != CORBA_OBJECT_NIL, 0);

	CORBA_exception_init (&ev);

	is_read_only = Bonobo_Property_is_read_only (prop, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		goto flags_error;

	flags = (is_read_only    ? BONOBO_PROPERTY_READ_ONLY       : 0);

	return flags;

 flags_error:
	CORBA_exception_free (&ev);
	CORBA_exception_init (&ev);

	CORBA_Object_release (prop, &ev);
	CORBA_exception_free (&ev);

	return 0;
}



/*
 * GtkObject crap.
 */

static void
bonobo_property_bag_client_class_init (BonoboPropertyBagClientClass *class)
{
/*	GtkObjectClass *object_class = (GtkObjectClass *) class; */
}

static void
bonobo_property_bag_client_init (BonoboPropertyBagClient *pbc)
{
}

/**
 * bonobo_property_bag_client_get_type:
 *
 * Returns: The GtkType corresponding to the BonoboPropertyBagClient
 * class.
 */
GtkType
bonobo_property_bag_client_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboPropertyBagClient",
			sizeof (BonoboPropertyBagClient),
			sizeof (BonoboPropertyBagClientClass),
			(GtkClassInitFunc) bonobo_property_bag_client_class_init,
			(GtkObjectInitFunc) bonobo_property_bag_client_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

#define SEND_OBTUSE(pbc,name,args,t,odd) \
	case CORBA_tk##t: \
		bonobo_property_bag_client_set_value##t (pbc, name, va_arg (args, CORBA##odd)); \
		break;

#define SEND(pbc,name,args,t) SEND_OBTUSE(pbc,name,args,t,t)

char *
bonobo_property_bag_client_setv (BonoboPropertyBagClient *pbc,
				 const char              *first_arg,
				 va_list                  var_args)
{
	const char *arg_name;

	g_return_val_if_fail (pbc != NULL, g_strdup ("No property bag"));
	g_return_val_if_fail (first_arg != NULL, g_strdup ("No arg"));

	arg_name = first_arg;
	while (arg_name) {
		CORBA_TypeCode type;

		type = bonobo_property_bag_client_get_property_type (pbc, arg_name);

		if (type == TC_null)
			return g_strdup_printf ("No such arg '%s'", arg_name);

		switch (type->kind) {
		case CORBA_tk_string:
			bonobo_property_bag_client_set_value_string (pbc, arg_name,
								     va_arg (var_args, CORBA_char *));
			break;

			SEND        (pbc, arg_name, var_args, _boolean);
			SEND_OBTUSE (pbc, arg_name, var_args, _ushort, _unsigned_short);
			SEND        (pbc, arg_name, var_args, _short);
			SEND        (pbc, arg_name, var_args, _float);
			SEND        (pbc, arg_name, var_args, _double);
			SEND_OBTUSE (pbc, arg_name, var_args, _ulong, _unsigned_long);
			SEND        (pbc, arg_name, var_args, _long);

		default:
			return g_strdup_printf ("Unhandled setv arg '%s' type %d",
						arg_name, type->kind);
		}

		arg_name = va_arg (var_args, char *);
	}

	return NULL;
}
#undef SEND
#undef SEND_OBTUSE

#define RECIEVE_OBTUSE(pbc,name,args,t,odd) \
	case CORBA_tk##t: \
		*((CORBA##odd *)va_arg (args, CORBA##odd *)) = \
		    bonobo_property_bag_client_get_value##t (pbc, name); \
		break;

#define RECIEVE(pbc,name,args,t) RECIEVE_OBTUSE(pbc,name,args,t,t)


char *
bonobo_property_bag_client_getv (BonoboPropertyBagClient *pbc,
				 const char              *first_arg,
				 va_list                  var_args)
{
	const char *arg_name;

	g_return_val_if_fail (pbc != NULL, g_strdup ("No property bag"));
	g_return_val_if_fail (first_arg != NULL, g_strdup ("No arg"));

	arg_name = first_arg;
	while (arg_name) {
		CORBA_TypeCode type;

		type = bonobo_property_bag_client_get_property_type (pbc, arg_name);

		if (type == TC_null)
			return g_strdup_printf ("No such arg '%s'", arg_name);

		switch (type->kind) {
		case CORBA_tk_string:
		{
			char *txt = bonobo_property_bag_client_get_value_string (pbc, arg_name);
			*((CORBA_char **)(va_arg (var_args, CORBA_char **))) =
				CORBA_string_dup (txt);
			g_free (txt);
			break;
		}

			RECIEVE        (pbc, arg_name, var_args, _boolean);
			RECIEVE_OBTUSE (pbc, arg_name, var_args, _ushort, _unsigned_short);
			RECIEVE        (pbc, arg_name, var_args, _short);
			RECIEVE        (pbc, arg_name, var_args, _float);
			RECIEVE        (pbc, arg_name, var_args, _double);
			RECIEVE_OBTUSE (pbc, arg_name, var_args, _ulong, _unsigned_long);
			RECIEVE        (pbc, arg_name, var_args, _long);

		default:
			return g_strdup_printf ("Unhandled getv arg '%s' type %d",
						arg_name, type->kind);
		}

		arg_name = va_arg (var_args, char *);
	}

	return NULL;
}
#undef RECIEVE
#undef RECIEVE_OBTUSE
