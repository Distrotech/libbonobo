/*
 * Some XML-based BonoboPropertyBag persistence helpers.
 *
 * Authors:
 *   Michael Meeks (michael@helixcode.com)
 *   Nat Friedman  (nat@nat.org)
 *
 * Copyright 1999,2000 Helix Code, Inc.
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
#include <bonobo/bonobo-ui-node.h>

/*
  A rough tag outline:

  Ignore Fixed, Unions for now; they suck badly.

  <any>
	<type name="%s" repo_id="%s" tckind="%d" length="%d"
	sub_parts="%d" recurse_depth="%d" default_index="%d">
		<subnames>
			<name>MyName</name>
			<name>Fish</name>
		</subnames>
		<subtypes>
			<type.../>
		</subtypes>
	</type>

	<value length="3">
		<value>2.5</value>
		<value>1</value>
		<value>hello</value>
	</value>
  </any>
 */

static void
encode_type (BonoboUINode      *type_parent,
	     CORBA_TypeCode     tc,
	     CORBA_Environment *ev)
{
	BonoboUINode *node;
	char scratch [128];
	int  i;

	node = bonobo_ui_node_new_child (type_parent, "type");

	if (tc->name)
		bonobo_ui_node_set_attr (node, "name", tc->name);
	
	if (tc->repo_id)
		bonobo_ui_node_set_attr (node, "repo_id", tc->repo_id);

	snprintf (scratch, 127, "%d", tc->kind);
	bonobo_ui_node_set_attr (node, "tckind", scratch);

	snprintf (scratch, 127, "%u", tc->length);
	bonobo_ui_node_set_attr (node, "length", scratch);

	snprintf (scratch, 127, "%u", tc->sub_parts);
	bonobo_ui_node_set_attr (node, "sub_parts", scratch);

	switch (tc->kind) {
 	case CORBA_tk_struct:
	case CORBA_tk_union:
	case CORBA_tk_enum:
	case CORBA_tk_except: { /* subnames */
		BonoboUINode *subnames;

		subnames = bonobo_ui_node_new_child (type_parent, "subnames");

		for (i = 0; i < tc->sub_parts; i++) {
			BonoboUINode *subname;
			subname = bonobo_ui_node_new_child (subnames, "name");
			bonobo_ui_node_set_content (subname, tc->subnames [i]);
		}
	}

	case CORBA_tk_alias:
	case CORBA_tk_array:
	case CORBA_tk_sequence: { /* subtypes */
		BonoboUINode *subtypes;

		subtypes = bonobo_ui_node_new_child (type_parent, "subtypes");

		for (i = 0; i < tc->sub_parts; i++)
			encode_type (subtypes, tc->subtypes [i], ev);
	}
	default:
		break;
	}
}

#define DO_ENCODE(tckind,format,corbatype,value)			\
	case tckind:							\
		snprintf (scratch, 127, format,				\
			  * (corbatype *) *value);			\
		break;

static void
encode_value (BonoboUINode      *parent,
	      CORBA_TypeCode     tc,
	      gpointer          *value,
	      CORBA_Environment *ev)
{
	BonoboUINode *node;
	char scratch [256] = "";

	node = bonobo_ui_node_new_child (parent, "value");

	switch (tc->kind) {
	case CORBA_tk_null:
	case CORBA_tk_void:
		break;

		DO_ENCODE (CORBA_tk_short,  "%d", CORBA_short, value);
		DO_ENCODE (CORBA_tk_ushort, "%u", CORBA_unsigned_short, value);
		DO_ENCODE (CORBA_tk_long,   "%d", CORBA_long, value);
		DO_ENCODE (CORBA_tk_enum,   "%d", CORBA_enum, value);
		DO_ENCODE (CORBA_tk_ulong,  "%u", CORBA_unsigned_long, value);

		DO_ENCODE (CORBA_tk_float,   "%g", CORBA_float,   value);
		DO_ENCODE (CORBA_tk_double,  "%g", CORBA_double,  value);
		DO_ENCODE (CORBA_tk_boolean, "%d", CORBA_boolean, value);

		DO_ENCODE (CORBA_tk_char,  "%d", CORBA_char, value);
		DO_ENCODE (CORBA_tk_octet, "%d", CORBA_octet, value);

		DO_ENCODE (CORBA_tk_wchar, "%d", CORBA_wchar, value);

/*
#ifdef G_HAVE_GINT64
		DO_ENCODE (CORBA_tk_longlong,   "%ll",  CORBA_long_long, value);
		DO_ENCODE (CORBA_tk_ulonglong,  "%ull", CORBA_unsigned_long_long, value);
#endif
		DO_ENCODE (CORBA_tk_longdouble, "%L", CORBA_long_double, value);
*/

	case CORBA_tk_string:
	case CORBA_tk_wstring:
		bonobo_ui_node_set_content (node, *(CORBA_char **) *value);
		break;

	case CORBA_tk_objref:
		g_warning ("Cannot serialize an objref");
		break;

	case CORBA_tk_TypeCode:
		encode_type (node, * (CORBA_TypeCode *) *value, ev);
		break;

	case CORBA_tk_any:
		bonobo_property_bag_xml_encode_any (node, (CORBA_any *) *value, ev);
		break;

 	case CORBA_tk_struct:
	case CORBA_tk_except:

	case CORBA_tk_sequence:
	case CORBA_tk_array:

	case CORBA_tk_alias:
	case CORBA_tk_union:
	case CORBA_tk_Principal:
	case CORBA_tk_fixed:
	case CORBA_tk_recursive:
	default:
		g_warning ("Unhandled");
		break;
	}

	if (scratch [0])
		bonobo_ui_node_set_content (node, scratch);
}

void 
bonobo_property_bag_xml_encode_any (BonoboUINode      *parent,
				    CORBA_any         *any,
				    CORBA_Environment *ev)
{
	BonoboUINode *node;
	gpointer      value;

	g_return_if_fail (any != NULL);
	g_return_if_fail (parent != NULL);

	node = bonobo_ui_node_new_child (parent, "any");
	value = any->_value;

	encode_type  (node, any->_type, ev);
	encode_value (node, any->_type, &value, ev);
}

CORBA_any *
bonobo_property_bag_xml_decode_any (BonoboUINode      *node,
				    CORBA_Environment *ev)
{
	return NULL;
}
