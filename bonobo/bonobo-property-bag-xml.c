/*
 * An XML-based BonoboPropertyBag persistence implementation.
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#include <config.h>
#include <stdlib.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-stream-client.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-property-bag-xml.h>

static GHashTable *bonobo_property_bag_xml_global_types = NULL;

#define PROPERTY_NAME_TOKEN  "name"
#define PROPERTY_VALUE_TOKEN "value"

/*
 * Internal data structures.
 */
typedef struct {
	GHashTable *types;
} BonoboPropertyBagXMLPersister;

typedef struct {
	BonoboPropertyBagValueToXMLStringFn   to_string;
	BonoboPropertyBagValueFromXMLStringFn from_string;
	gpointer			     user_data;
} BonoboPropertyValueXMLConverter;

/*
 * Static prototypes.
 */
static void  bonobo_property_bag_xml_setup_global_types (BonoboPropertyBagXMLPersister *persister,
							BonoboPropertyBag *pb);

static BonoboPropertyValueXMLConverter *
bonobo_property_bag_xml_get_converter (BonoboPropertyBagXMLPersister *persister,
				      const char *type)
{
	BonoboPropertyValueXMLConverter *converter;

	g_assert (persister != NULL);
	g_assert (type != NULL);

	converter = g_hash_table_lookup (persister->types, type);

	if (converter != NULL)
		return converter;

	return g_hash_table_lookup (bonobo_property_bag_xml_global_types, type);
}

static char *
bonobo_property_bag_xml_value_to_string (BonoboPropertyBagXMLPersister *persister,
					const char *type, gpointer value)
{
	BonoboPropertyValueXMLConverter *converter;

	g_assert (bonobo_property_bag_xml_global_types != NULL);
	g_assert (persister != NULL);
	g_assert (persister->types != NULL);
	g_assert (type != NULL);

	converter = bonobo_property_bag_xml_get_converter (persister, type);

	if (converter == NULL) {
		g_warning ("BonoboPropertyBagXML: Could not find converter for type \"%s\".  "
			   "Maybe you forgot to add it?", type);
		return g_strdup ("Unknown");
	}

	return (converter->to_string) (type, value, converter->user_data);
}

static gpointer
bonobo_property_bag_xml_string_to_value (BonoboPropertyBagXMLPersister *persister,
					const char *type, const char *str)
{
	BonoboPropertyValueXMLConverter *converter;

	g_assert (bonobo_property_bag_xml_global_types != NULL);
	g_assert (persister != NULL);
	g_assert (persister->types != NULL);
	g_assert (type != NULL);
	g_assert (str != NULL);

	converter = bonobo_property_bag_xml_get_converter (persister, type);

	if (converter == NULL) {
		g_warning ("BonoboPropertyBagXML: Could not find converter for type \"%s\".  "
			   "Maybe you forgot to add it?", type);
		return NULL;
	}

	return (converter->from_string) (type, str, converter->user_data);
}

static gboolean
bonobo_property_bag_xml_stream_buffer (xmlChar *docbuff, size_t docsz,
				       const Bonobo_Stream stream)
{
	CORBA_Environment  ev;
	size_t		   pos;

	/*
	 * Write the size of the buffer out at the beginning.
	 */
	CORBA_exception_init (&ev);

	bonobo_stream_client_printf (stream, TRUE, &ev, "%d", docsz);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBagXML: Exception while writing size header to stream!");
		return FALSE;
	}

	/*
	 * Now write the XML buffer out.
	 */
	pos = 0;
	while (pos < docsz) {
		CORBA_long bytes_written;

		bytes_written = bonobo_stream_client_write (stream, docbuff + pos, docsz - pos, &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			g_warning ("BonoboPropertyBagXML: Exception while writing properties to stream");
			CORBA_exception_free (&ev);
			return FALSE;
		}

		pos += bytes_written;
	}

	CORBA_exception_free (&ev);

	return TRUE;
}


/**
 * bonobo_property_bag_xml_persist:
 */
gboolean
bonobo_property_bag_xml_persist (BonoboPropertyBag *pb, const Bonobo_Stream stream,
				gpointer user_data)
{
	BonoboPropertyBagXMLPersister *persister = user_data;
	xmlDocPtr		      doc;
	xmlNsPtr		      ns;
	GList			     *props;
	GList			     *l;
	xmlChar			     *docbuff;
	size_t			      docsz;
	int			      temp_sz;

	/*
	 * Create the document and top-level node.
	 */
	doc = xmlNewDoc ("1.0");
	ns = xmlNewGlobalNs (doc, "http://www.helixcode.com/bonobo/property-persist-xml", NULL);
	doc->xmlRootNode = xmlNewDocNode (doc, ns, "properties", NULL);

	/*
	 * Now create each property.
	 */
	props = bonobo_property_bag_get_prop_list (pb);

	for (l = props; l != NULL; l = l->next) {
		BonoboProperty *prop = l->data;
		xmlNodePtr propnode;
		char *value_str;
		char *default_str;

		if (prop->flags & BONOBO_PROPERTY_UNSTORED)
			continue;

		if (! (prop->flags & BONOBO_PROPERTY_READABLE))
			continue;

		if (prop->flags & BONOBO_PROPERTY_USE_DEFAULT_OPT) {
			if (bonobo_property_bag_compare_values (pb, prop->type,
							       prop->value, prop->default_value))
				continue;
		}

		value_str   = bonobo_property_bag_xml_value_to_string (persister, prop->type, prop->value);
		default_str = bonobo_property_bag_xml_value_to_string (persister, prop->type, prop->default_value);

		propnode = xmlNewChild (doc->xmlRootNode, ns, "property", NULL);
		xmlNewChild (propnode, ns, PROPERTY_NAME_TOKEN, prop->name);
		xmlNewChild (propnode, ns, PROPERTY_VALUE_TOKEN, value_str);

		g_free (value_str);
		g_free (default_str);
	}

	g_list_free (props);

	/*
	 * Store the entire XML document in a buffer.
	 */
	xmlDocDumpMemory (doc, &docbuff, &temp_sz);
	docsz = (size_t) temp_sz;
	xmlFreeDoc (doc);

	/*
	 * Synchronously (vomit!) stream the document into @stream.
	 */
	if (! bonobo_property_bag_xml_stream_buffer (docbuff, docsz, stream))
		g_warning ("BonoboPropertyBagXML: Unable to stream property XML buffer!");

	g_free (docbuff);

	return TRUE;
}

static gboolean
bonobo_property_bag_xml_unstream_buffer (const Bonobo_Stream stream,
					 xmlChar **docbuff, size_t *docsize)
{
	CORBA_Environment  ev;
	char		  *szstr;
	size_t		   pos;

	/*
	 * Read the size from the beginning of the stream.
	 */
	CORBA_exception_init (&ev);

	if ( bonobo_stream_client_read_string (stream, &szstr, &ev) == -1) {
		g_warning ("BonoboPropertyBagXML: Exception reading buffer size from stream!");
		CORBA_exception_free (&ev);
		return FALSE;
	}

	*docsize = strtol (szstr, NULL, 10);
	g_free (szstr);

	/*
	 * Now read the XML buffer into memory.
	 */
	(*docbuff) = g_new (xmlChar, *docsize);
	pos = 0;
	while (pos < *docsize) {
		Bonobo_Stream_iobuf *buffer;

#define READ_CHUNK_SIZE 65536
		Bonobo_Stream_read (stream,
				   MIN (READ_CHUNK_SIZE, (*docsize) - pos),
				   &buffer,
				   &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			g_warning ("BonoboPropertyBagXML: Exception while reading XML buffer from stream!");
			g_free (*docbuff);
			CORBA_exception_free (&ev);
			return FALSE;
		}

		memcpy ((*docbuff) + pos, buffer->_buffer, buffer->_length);
		pos += buffer->_length;

		CORBA_free (buffer);
	}

	return TRUE;
}

/*
 * Get a value for a node either carried as an attibute or as
 * the content of a child.
 */
static const char *
xml_value_get (xmlNodePtr node, const char *name)
{
	char       *ret;
	xmlNodePtr  child;

	ret = (char *) xmlGetProp (node, name);
	if (ret != NULL)
		return ret;
	child = node->xmlChildrenNode;

	while (child != NULL) {
		if (!strcmp (child->name, name)) {
		        /*
			 * !!! Inefficient, but ...
			 */
			ret = xmlNodeGetContent(child);
			if (ret != NULL)
			    return (ret);
		}
		child = child->next;
	}

	return NULL;
}

static void
bonobo_property_bag_xml_load_property (BonoboPropertyBag *pb,
				      BonoboPropertyBagXMLPersister *persister,
				      xmlNodePtr prop_node)
{
	const char *name_str;
	const char *value_str;
	const char *type;
	gpointer    value;
	
	if (strcmp (prop_node->name, "property")) {
		g_warning ("BonoboPropertyBagXML: Top-level property node name is not \"property\" -- "
			   "malformatted XML buffer!");
		return;
	}

	/*
	 * Grab the property values into strings.
	 */
	name_str      = xml_value_get (prop_node, "name");
	value_str     = xml_value_get (prop_node, "value");

	type = bonobo_property_bag_get_prop_type (pb, name_str);

	value = bonobo_property_bag_xml_string_to_value (persister, type, value_str);

	bonobo_property_bag_set_value (pb, name_str, value);
}

/**
 * bonobo_property_bag_xml_depersist:
 */
gboolean
bonobo_property_bag_xml_depersist (BonoboPropertyBag *pb, const Bonobo_Stream stream,
				  gpointer user_data)
{
	BonoboPropertyBagXMLPersister *persister = user_data;
	xmlChar			     *docbuff;
	xmlDocPtr		      doc;
	xmlNodePtr		      curr;
	size_t			      docsz;
	
	/*
	 * Read in the PropertyBag XML document.
	 */
	if (! bonobo_property_bag_xml_unstream_buffer (stream, &docbuff, &docsz))
		return FALSE;

	/*
	 * Parse the buffer into an XML document.
	 */
	doc = xmlParseMemory (docbuff, docsz);
	if (doc == NULL) {
		g_warning ("BonoboPropertyBagXML: Could not parse XML buffer!");
		g_free (docbuff);
		return FALSE;
	}

	g_free (docbuff);

	/*
	 * Read all of the properties out of the XML document and
	 * modify our internal properties accordingly.
	 */
	for (curr = doc->xmlRootNode->xmlChildrenNode; curr != NULL; curr = curr->next)
		bonobo_property_bag_xml_load_property (pb, persister, curr);

	xmlFreeDoc (doc);

	return TRUE;
}

static gboolean
bonobo_property_bag_xml_foreach_remove_type (gpointer key, gpointer value,
					    gpointer user_data)
{
	g_free (key);
	g_free (value);

	return TRUE;
}

/* This function is called when the PropertyBag is destroyed. */
static void
bonobo_property_bag_xml_destroy_cb (BonoboPropertyBag *pb, gpointer data)
{
	BonoboPropertyBagXMLPersister *persister = data;

	/*
	 * Destroy the local types table.
	 */
	g_hash_table_foreach_remove (persister->types,
				     bonobo_property_bag_xml_foreach_remove_type,
				     NULL);

	g_hash_table_destroy (persister->types);

	g_free (persister);
}

/**
 * bonobo_property_bag_xml_register:
 * @pb: A BonoboPropertyBag.
 *
 * Sets @pb to use the XML persistence implementation for
 * saving/loading properties.
 */
void
bonobo_property_bag_xml_register (BonoboPropertyBag *pb)
{
	BonoboPropertyBagXMLPersister *persister;

	/*
	 * We will pass 'persister' around as the closure to all the
	 * BonoboPropertyBag persistence functions.
	 */
	persister = g_new0 (BonoboPropertyBagXMLPersister, 1);
	persister->types = g_hash_table_new (g_str_hash, g_str_equal);

	gtk_signal_connect (GTK_OBJECT (pb), "destroy",
			    GTK_SIGNAL_FUNC (bonobo_property_bag_xml_destroy_cb),
			    persister);

	/*
	 * Create the default XML types.
	 */
	bonobo_property_bag_xml_setup_global_types (persister, pb);

	/*
	 * Register this persister with the PropertyBag.
	 */
	bonobo_property_bag_set_persister (pb,
					  bonobo_property_bag_xml_persist,
					  bonobo_property_bag_xml_depersist,
					  persister);

}


/*
 * The XML persistence type system.
 */
static void
bonobo_property_bag_xml_create_global_type (const char				*type_name,
					   BonoboPropertyBagValueToXMLStringFn    to_string,
					   BonoboPropertyBagValueFromXMLStringFn  from_string)
{
	BonoboPropertyValueXMLConverter *converter;

	converter = g_hash_table_lookup (bonobo_property_bag_xml_global_types, type_name);

	if (converter == NULL) {
		converter = g_new0 (BonoboPropertyValueXMLConverter, 1);

		g_hash_table_insert (bonobo_property_bag_xml_global_types, g_strdup (type_name),
				     converter);
	}

	converter->to_string   = to_string;
	converter->from_string = from_string;
}

static void
bonobo_property_bag_xml_create_local_type (BonoboPropertyBagXMLPersister		*persister,
					  const char				*type_name,
					  BonoboPropertyBagValueToXMLStringFn     to_string,
					  BonoboPropertyBagValueFromXMLStringFn   from_string,
					  gpointer				 user_data)
{
	BonoboPropertyValueXMLConverter *converter;

	converter = g_hash_table_lookup (persister->types, type_name);

	if (converter == NULL) {
		converter = g_new0 (BonoboPropertyValueXMLConverter, 1);

		g_hash_table_insert (persister->types, g_strdup (type_name),
				     converter);
	}

	converter->to_string   = to_string;
	converter->from_string = from_string;
	converter->user_data   = user_data;
}

/**
 * bonobo_property_bag_xml_add_type:
 */
void
bonobo_property_bag_xml_add_type (BonoboPropertyBag		      *pb,
				 const char			      *type_name,
				 BonoboPropertyBagValueToXMLStringFn    to_string,
				 BonoboPropertyBagValueFromXMLStringFn  from_string,
				 gpointer			       user_data)
{
	BonoboPropertyBagXMLPersister   *persister;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb));
	g_return_if_fail (type_name != NULL);
	g_return_if_fail (to_string != NULL);
	g_return_if_fail (from_string != NULL);

	persister = bonobo_property_bag_get_persist_data (pb);
	g_return_if_fail (persister != NULL);

	bonobo_property_bag_xml_create_local_type (persister, type_name,
						  to_string, from_string,
						  user_data);
}

/*
 * Default XML types.
 */

/* Value -> XML String */
static char *
bonobo_property_bag_boolean_to_xml_string (const char *type, gpointer value,
					  gpointer user_data)
{
	gboolean b = *(gboolean *) value;

	return g_strdup_printf ("%s", b ? "True" : "False");
}

static char *
bonobo_property_bag_short_to_xml_string (const char *type, gpointer value,
					gpointer user_data)
{
	gshort s = *(gshort *) value;

	return g_strdup_printf ("%d", s);
}

static char *
bonobo_property_bag_ushort_to_xml_string (const char *type, gpointer value,
					 gpointer user_data)
{
	gushort s = *(gushort *) value;

	return g_strdup_printf ("%d", s);
}

static char *
bonobo_property_bag_long_to_xml_string (const char *type, gpointer value,
				       gpointer user_data)
{
	glong l = *(glong *) value;

	return g_strdup_printf ("%ld", l);
}

static char *
bonobo_property_bag_ulong_to_xml_string (const char *type, gpointer value,
					gpointer user_data)
{
	gulong l = *(gulong *) value;

	return g_strdup_printf ("%ld", l);
}

static char *
bonobo_property_bag_float_to_xml_string (const char *type, gpointer value,
					gpointer user_data)
{
	gfloat f = *(gfloat *) value;

	return g_strdup_printf ("%f", f);
}

static char *
bonobo_property_bag_double_to_xml_string (const char *type, gpointer value,
					 gpointer user_data)
{
	gdouble d = *(gdouble *) value;

	return g_strdup_printf ("%f", d);
}

static char *
bonobo_property_bag_string_to_xml_string (const char *type, gpointer value,
					 gpointer user_data)
{
	return g_strdup ((char *) value);
}

/* XML String -> Value */
static gpointer
bonobo_property_bag_xml_string_to_boolean (const char *type, const char *str,
					  gpointer user_data)
{
	gboolean *b;

	b = g_new0 (gboolean, 1);

	if (! g_strcasecmp (str, "True"))
		*b = TRUE;
	else
		*b = FALSE;

	return (gpointer) b;
}

static gpointer
bonobo_property_bag_xml_string_to_short (const char *type, const char *str,
					gpointer user_data)
{
	gshort *s;

	s = g_new0 (gshort, 1);

	*s = atoi (str);

	return (gpointer) s;
}

static gpointer
bonobo_property_bag_xml_string_to_ushort (const char *type, const char *str,
					 gpointer user_data)
{
	gushort *s;

	s = g_new0 (gushort, 1);

	*s = atoi (str);

	return (gpointer) s;
}

static gpointer
bonobo_property_bag_xml_string_to_long (const char *type, const char *str,
				       gpointer user_data)
{
	glong *l;

	l = g_new0 (glong, 1);

	*l = strtol (str, NULL, 10);

	return (gpointer) l;
}

static gpointer
bonobo_property_bag_xml_string_to_ulong (const char *type, const char *str,
					gpointer user_data)
{
	gulong *l;

	l = g_new0 (gulong, 1);

	*l = strtol (str, NULL, 10);

	return (gpointer) l;
}

static gpointer
bonobo_property_bag_xml_string_to_float (const char *type, const char *str,
					gpointer user_data)
{
	gfloat *f;

	f = g_new0 (gfloat, 1);

	*f = strtod (str, NULL);

	return (gpointer) f;
}

static gpointer
bonobo_property_bag_xml_string_to_double (const char *type, const char *str,
					 gpointer user_data)
{
	gdouble *d;

	d = g_new0 (gdouble, 1);

	*d = strtod (str, NULL);

	return (gpointer) d;
}

static gpointer
bonobo_property_bag_xml_string_to_string (const char *type, const char *str,
					 gpointer user_data)
{
	return g_strdup ((char *) str);
}


static void
bonobo_property_bag_xml_setup_global_types (BonoboPropertyBagXMLPersister *persister,
					   BonoboPropertyBag *pb)
{
	bonobo_property_bag_xml_create_global_type ("boolean",
						   bonobo_property_bag_boolean_to_xml_string,
						   bonobo_property_bag_xml_string_to_boolean);
	bonobo_property_bag_xml_create_global_type ("short",
						   bonobo_property_bag_short_to_xml_string,
						   bonobo_property_bag_xml_string_to_short);
	bonobo_property_bag_xml_create_global_type ("ushort",
						   bonobo_property_bag_ushort_to_xml_string,
						   bonobo_property_bag_xml_string_to_ushort);
	bonobo_property_bag_xml_create_global_type ("long",
						   bonobo_property_bag_long_to_xml_string,
						   bonobo_property_bag_xml_string_to_long);
	bonobo_property_bag_xml_create_global_type ("ulong",
						   bonobo_property_bag_ulong_to_xml_string,
						   bonobo_property_bag_xml_string_to_ulong);
	bonobo_property_bag_xml_create_global_type ("float",
						   bonobo_property_bag_float_to_xml_string,
						   bonobo_property_bag_xml_string_to_float);
	bonobo_property_bag_xml_create_global_type ("double",
						   bonobo_property_bag_double_to_xml_string,
						   bonobo_property_bag_xml_string_to_double);
	bonobo_property_bag_xml_create_global_type ("string",
						   bonobo_property_bag_string_to_xml_string,
						   bonobo_property_bag_xml_string_to_string);
}
