/*
 * An XML-based GnomePropertyBag persistence implementation.
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
#include <bonobo/gnome-stream-client.h>
#include <gnome-xml/tree.h>
#include <gnome-xml/parser.h>
#include <bonobo/gnome-property-bag.h>
#include <bonobo/gnome-property-bag-xml.h>

static GHashTable *gnome_property_bag_xml_global_types = NULL;

#define PROPERTY_NAME_TOKEN  "name"
#define PROPERTY_VALUE_TOKEN "value"

/*
 * Internal data structures.
 */
typedef struct {
	GHashTable *types;
} GnomePropertyBagXMLPersister;

typedef struct {
	GnomePropertyBagValueToXMLStringFn   to_string;
	GnomePropertyBagValueFromXMLStringFn from_string;
	gpointer			     user_data;
} GnomePropertyValueXMLConverter;

/*
 * Static prototypes.
 */
static void  gnome_property_bag_xml_setup_global_types (GnomePropertyBagXMLPersister *persister,
							GnomePropertyBag *pb);

static GnomePropertyValueXMLConverter *
gnome_property_bag_xml_get_converter (GnomePropertyBagXMLPersister *persister,
				      const char *type)
{
	GnomePropertyValueXMLConverter *converter;

	g_assert (persister != NULL);
	g_assert (type != NULL);

	converter = g_hash_table_lookup (persister->types, type);

	if (converter != NULL)
		return converter;

	return g_hash_table_lookup (gnome_property_bag_xml_global_types, type);
}

static char *
gnome_property_bag_xml_value_to_string (GnomePropertyBagXMLPersister *persister,
					const char *type, gpointer value)
{
	GnomePropertyValueXMLConverter *converter;

	g_assert (gnome_property_bag_xml_global_types != NULL);
	g_assert (persister != NULL);
	g_assert (persister->types != NULL);
	g_assert (type != NULL);

	converter = gnome_property_bag_xml_get_converter (persister, type);

	if (converter == NULL) {
		g_warning ("GnomePropertyBagXML: Could not find converter for type \"%s\".  "
			   "Maybe you forgot to add it?", type);
		return g_strdup ("Unknown");
	}

	return (converter->to_string) (type, value, converter->user_data);
}

static gpointer
gnome_property_bag_xml_string_to_value (GnomePropertyBagXMLPersister *persister,
					const char *type, const char *str)
{
	GnomePropertyValueXMLConverter *converter;

	g_assert (gnome_property_bag_xml_global_types != NULL);
	g_assert (persister != NULL);
	g_assert (persister->types != NULL);
	g_assert (type != NULL);
	g_assert (str != NULL);

	converter = gnome_property_bag_xml_get_converter (persister, type);

	if (converter == NULL) {
		g_warning ("GnomePropertyBagXML: Could not find converter for type \"%s\".  "
			   "Maybe you forgot to add it?", type);
		return NULL;
	}

	return (converter->from_string) (type, str, converter->user_data);
}

static gboolean
gnome_property_bag_xml_stream_buffer (xmlChar *docbuff, size_t docsz,
				      const GNOME_Stream stream)
{
	CORBA_Environment  ev;
	size_t		   pos;

	/*
	 * Write the size of the buffer out at the beginning.
	 */
	CORBA_exception_init (&ev);

	gnome_stream_client_printf (stream, TRUE, &ev, "%d", docsz);

	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomePropertyBagXML: Exception while writing size header to stream!\n");
		return FALSE;
	}

	/*
	 * Now write the XML buffer out.
	 */
	pos = 0;
	while (pos < docsz) {
		CORBA_long bytes_written;

		bytes_written = gnome_stream_client_write (stream, docbuff + pos, docsz - pos, &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			g_warning ("GnomePropertyBagXML: Exception while writing properties to stream\n");
			CORBA_exception_free (&ev);
			return FALSE;
		}

		pos += bytes_written;
	}

	CORBA_exception_free (&ev);

	return TRUE;
}


/**
 * gnome_property_bag_xml_persist:
 */
gboolean
gnome_property_bag_xml_persist (GnomePropertyBag *pb, const GNOME_Stream stream,
				gpointer user_data)
{
	GnomePropertyBagXMLPersister *persister = user_data;
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
	doc->root = xmlNewDocNode (doc, ns, "properties", NULL);

	/*
	 * Now create each property.
	 */
	props = gnome_property_bag_get_prop_list (pb);

	for (l = props; l != NULL; l = l->next) {
		GnomeProperty *prop = l->data;
		xmlNodePtr propnode;
		char *value_str;
		char *default_str;

		if (prop->flags & GNOME_PROPERTY_UNSTORED)
			continue;

		if (! (prop->flags & GNOME_PROPERTY_READ_ONLY))
			continue;

		if (prop->flags & GNOME_PROPERTY_USE_DEFAULT_OPT) {
			if (gnome_property_bag_compare_values (pb, prop->type,
							       prop->value, prop->default_value))
				continue;
		}

		value_str   = gnome_property_bag_xml_value_to_string (persister, prop->type, prop->value);
		default_str = gnome_property_bag_xml_value_to_string (persister, prop->type, prop->default_value);

		propnode = xmlNewChild (doc->root, ns, "property", NULL);
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
	if (! gnome_property_bag_xml_stream_buffer (docbuff, docsz, stream))
		g_warning ("GnomePropertyBagXML: Unable to stream property XML buffer!");

	g_free (docbuff);

	return TRUE;
}

static gboolean
gnome_property_bag_xml_unstream_buffer (const GNOME_Stream stream,
					xmlChar **docbuff, size_t *docsize)
{
	CORBA_Environment  ev;
	char		  *szstr;
	size_t		   pos;

	/*
	 * Read the size from the beginning of the stream.
	 */
	CORBA_exception_init (&ev);

	if ( gnome_stream_client_read_string (stream, &szstr, &ev) == -1) {
		g_warning ("GnomePropertyBagXML: Exception reading buffer size from stream!\n");
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
		GNOME_Stream_iobuf *buffer;

#define READ_CHUNK_SIZE 65536
		GNOME_Stream_read (stream,
				   MIN (READ_CHUNK_SIZE, (*docsize) - pos),
				   &buffer,
				   &ev);

		if (ev._major != CORBA_NO_EXCEPTION) {
			g_warning ("GnomePropertyBagXML: Exception while reading XML buffer from stream!\n");
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
	child = node->childs;

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
gnome_property_bag_xml_load_property (GnomePropertyBag *pb,
				      GnomePropertyBagXMLPersister *persister,
				      xmlNodePtr prop_node)
{
	const char *name_str;
	const char *value_str;
	const char *type;
	gpointer    value;
	
	if (strcmp (prop_node->name, "property")) {
		g_warning ("GnomePropertyBagXML: Top-level property node name is not \"property\" -- "
			   "malformatted XML buffer!\n");
		return;
	}

	/*
	 * Grab the property values into strings.
	 */
	name_str      = xml_value_get (prop_node, "name");
	value_str     = xml_value_get (prop_node, "value");

	type = gnome_property_bag_get_prop_type (pb, name_str);

	value = gnome_property_bag_xml_string_to_value (persister, type, value_str);

	gnome_property_bag_set_value (pb, name_str, value);
}

/**
 * gnome_property_bag_xml_depersist:
 */
gboolean
gnome_property_bag_xml_depersist (GnomePropertyBag *pb, const GNOME_Stream stream,
				  gpointer user_data)
{
	GnomePropertyBagXMLPersister *persister = user_data;
	xmlChar			     *docbuff;
	xmlDocPtr		      doc;
	xmlNodePtr		      curr;
	size_t			      docsz;
	
	/*
	 * Read in the PropertyBag XML document.
	 */
	if (! gnome_property_bag_xml_unstream_buffer (stream, &docbuff, &docsz))
		return FALSE;

	/*
	 * Parse the buffer into an XML document.
	 */
	doc = xmlParseMemory (docbuff, docsz);
	if (doc == NULL) {
		g_warning ("GnomePropertyBagXML: Could not parse XML buffer!\n");
		g_free (docbuff);
		return FALSE;
	}

	g_free (docbuff);

	/*
	 * Read all of the properties out of the XML document and
	 * modify our internal properties accordingly.
	 */
	for (curr = doc->root->childs; curr != NULL; curr = curr->next)
		gnome_property_bag_xml_load_property (pb, persister, curr);

	xmlFreeDoc (doc);

	return TRUE;
}

static gboolean
gnome_property_bag_xml_foreach_remove_type (gpointer key, gpointer value,
					    gpointer user_data)
{
	g_free (key);
	g_free (value);

	return TRUE;
}

/* This function is called when the PropertyBag is destroyed. */
static void
gnome_property_bag_xml_destroy_cb (GnomePropertyBag *pb, gpointer data)
{
	GnomePropertyBagXMLPersister *persister = data;

	/*
	 * Destroy the local types table.
	 */
	g_hash_table_foreach_remove (persister->types,
				     gnome_property_bag_xml_foreach_remove_type,
				     NULL);

	g_hash_table_destroy (persister->types);

	g_free (persister);
}

/**
 * gnome_property_bag_xml_register:
 * @pb: A GnomePropertyBag.
 *
 * Sets @pb to use the XML persistence implementation for
 * saving/loading properties.
 */
void
gnome_property_bag_xml_register (GnomePropertyBag *pb)
{
	GnomePropertyBagXMLPersister *persister;

	/*
	 * We will pass 'persister' around as the closure to all the
	 * GnomePropertyBag persistence functions.
	 */
	persister = g_new0 (GnomePropertyBagXMLPersister, 1);
	persister->types = g_hash_table_new (g_str_hash, g_str_equal);

	gtk_signal_connect (GTK_OBJECT (pb), "destroy",
			    GTK_SIGNAL_FUNC (gnome_property_bag_xml_destroy_cb),
			    persister);

	/*
	 * Create the default XML types.
	 */
	gnome_property_bag_xml_setup_global_types (persister, pb);

	/*
	 * Register this persister with the PropertyBag.
	 */
	gnome_property_bag_set_persister (pb,
					  gnome_property_bag_xml_persist,
					  gnome_property_bag_xml_depersist,
					  persister);

}


/*
 * The XML persistence type system.
 */
static void
gnome_property_bag_xml_create_global_type (const char				*type_name,
					   GnomePropertyBagValueToXMLStringFn    to_string,
					   GnomePropertyBagValueFromXMLStringFn  from_string)
{
	GnomePropertyValueXMLConverter *converter;

	converter = g_hash_table_lookup (gnome_property_bag_xml_global_types, type_name);

	if (converter == NULL) {
		converter = g_new0 (GnomePropertyValueXMLConverter, 1);

		g_hash_table_insert (gnome_property_bag_xml_global_types, g_strdup (type_name),
				     converter);
	}

	converter->to_string   = to_string;
	converter->from_string = from_string;
}

static void
gnome_property_bag_xml_create_local_type (GnomePropertyBagXMLPersister		*persister,
					  const char				*type_name,
					  GnomePropertyBagValueToXMLStringFn     to_string,
					  GnomePropertyBagValueFromXMLStringFn   from_string,
					  gpointer				 user_data)
{
	GnomePropertyValueXMLConverter *converter;

	converter = g_hash_table_lookup (persister->types, type_name);

	if (converter == NULL) {
		converter = g_new0 (GnomePropertyValueXMLConverter, 1);

		g_hash_table_insert (persister->types, g_strdup (type_name),
				     converter);
	}

	converter->to_string   = to_string;
	converter->from_string = from_string;
	converter->user_data   = user_data;
}

/**
 * gnome_property_bag_xml_add_type:
 */
void
gnome_property_bag_xml_add_type (GnomePropertyBag		      *pb,
				 const char			      *type_name,
				 GnomePropertyBagValueToXMLStringFn    to_string,
				 GnomePropertyBagValueFromXMLStringFn  from_string,
				 gpointer			       user_data)
{
	GnomePropertyBagXMLPersister   *persister;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG (pb));
	g_return_if_fail (type_name != NULL);
	g_return_if_fail (to_string != NULL);
	g_return_if_fail (from_string != NULL);

	persister = gnome_property_bag_get_persist_data (pb);
	g_return_if_fail (persister != NULL);

	gnome_property_bag_xml_create_local_type (persister, type_name,
						  to_string, from_string,
						  user_data);
}

/*
 * Default XML types.
 */

/* Value -> XML String */
static char *
gnome_property_bag_boolean_to_xml_string (const char *type, gpointer value,
					  gpointer user_data)
{
	gboolean b = *(gboolean *) value;

	return g_strdup_printf ("%s", b ? "True" : "False");
}

static char *
gnome_property_bag_short_to_xml_string (const char *type, gpointer value,
					gpointer user_data)
{
	gshort s = *(gshort *) value;

	return g_strdup_printf ("%d", s);
}

static char *
gnome_property_bag_ushort_to_xml_string (const char *type, gpointer value,
					 gpointer user_data)
{
	gushort s = *(gushort *) value;

	return g_strdup_printf ("%d", s);
}

static char *
gnome_property_bag_long_to_xml_string (const char *type, gpointer value,
				       gpointer user_data)
{
	glong l = *(glong *) value;

	return g_strdup_printf ("%ld", l);
}

static char *
gnome_property_bag_ulong_to_xml_string (const char *type, gpointer value,
					gpointer user_data)
{
	gulong l = *(gulong *) value;

	return g_strdup_printf ("%ld", l);
}

static char *
gnome_property_bag_float_to_xml_string (const char *type, gpointer value,
					gpointer user_data)
{
	gfloat f = *(gfloat *) value;

	return g_strdup_printf ("%f", f);
}

static char *
gnome_property_bag_double_to_xml_string (const char *type, gpointer value,
					 gpointer user_data)
{
	gdouble d = *(gdouble *) value;

	return g_strdup_printf ("%f", d);
}

static char *
gnome_property_bag_string_to_xml_string (const char *type, gpointer value,
					 gpointer user_data)
{
	return g_strdup ((char *) value);
}

/* XML String -> Value */
static gpointer
gnome_property_bag_xml_string_to_boolean (const char *type, const char *str,
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
gnome_property_bag_xml_string_to_short (const char *type, const char *str,
					gpointer user_data)
{
	gshort *s;

	s = g_new0 (gshort, 1);

	*s = atoi (str);

	return (gpointer) s;
}

static gpointer
gnome_property_bag_xml_string_to_ushort (const char *type, const char *str,
					 gpointer user_data)
{
	gushort *s;

	s = g_new0 (gushort, 1);

	*s = atoi (str);

	return (gpointer) s;
}

static gpointer
gnome_property_bag_xml_string_to_long (const char *type, const char *str,
				       gpointer user_data)
{
	glong *l;

	l = g_new0 (glong, 1);

	*l = strtol (str, NULL, 10);

	return (gpointer) l;
}

static gpointer
gnome_property_bag_xml_string_to_ulong (const char *type, const char *str,
					gpointer user_data)
{
	gulong *l;

	l = g_new0 (gulong, 1);

	*l = strtol (str, NULL, 10);

	return (gpointer) l;
}

static gpointer
gnome_property_bag_xml_string_to_float (const char *type, const char *str,
					gpointer user_data)
{
	gfloat *f;

	f = g_new0 (gfloat, 1);

	*f = strtod (str, NULL);

	return (gpointer) f;
}

static gpointer
gnome_property_bag_xml_string_to_double (const char *type, const char *str,
					 gpointer user_data)
{
	gdouble *d;

	d = g_new0 (gdouble, 1);

	*d = strtod (str, NULL);

	return (gpointer) d;
}

static gpointer
gnome_property_bag_xml_string_to_string (const char *type, const char *str,
					 gpointer user_data)
{
	return g_strdup ((char *) str);
}


static void
gnome_property_bag_xml_setup_global_types (GnomePropertyBagXMLPersister *persister,
					   GnomePropertyBag *pb)
{
	gnome_property_bag_xml_create_global_type ("boolean",
						   gnome_property_bag_boolean_to_xml_string,
						   gnome_property_bag_xml_string_to_boolean);
	gnome_property_bag_xml_create_global_type ("short",
						   gnome_property_bag_short_to_xml_string,
						   gnome_property_bag_xml_string_to_short);
	gnome_property_bag_xml_create_global_type ("ushort",
						   gnome_property_bag_ushort_to_xml_string,
						   gnome_property_bag_xml_string_to_ushort);
	gnome_property_bag_xml_create_global_type ("long",
						   gnome_property_bag_long_to_xml_string,
						   gnome_property_bag_xml_string_to_long);
	gnome_property_bag_xml_create_global_type ("ulong",
						   gnome_property_bag_ulong_to_xml_string,
						   gnome_property_bag_xml_string_to_ulong);
	gnome_property_bag_xml_create_global_type ("float",
						   gnome_property_bag_float_to_xml_string,
						   gnome_property_bag_xml_string_to_float);
	gnome_property_bag_xml_create_global_type ("double",
						   gnome_property_bag_double_to_xml_string,
						   gnome_property_bag_xml_string_to_double);
	gnome_property_bag_xml_create_global_type ("string",
						   gnome_property_bag_string_to_xml_string,
						   gnome_property_bag_xml_string_to_string);
}
