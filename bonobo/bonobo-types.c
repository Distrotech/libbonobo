#include <config.h>
#include <gobject/gbsearcharray.h>
#include <gobject/gvalue.h>
#include <gobject/gvaluearray.h>
#include <gobject/gvaluecollector.h>
#include <bonobo/bonobo-types.h>
#include <bonobo/bonobo-arg.h>

typedef struct
{
	GType            type;
	CORBA_TypeCode   tc;
	gboolean         is_bonobo_unknown;
} CorbaObjectProxy;

static gint corba_object_proxy_cmp (gconstpointer p1, gconstpointer p2);

static GBSearchArray corba_object_proxy_bsa = G_STATIC_BSEARCH_ARRAY_INIT (sizeof (CorbaObjectProxy), corba_object_proxy_cmp, 0);

static gint
corba_object_proxy_cmp (gconstpointer p1, gconstpointer p2)
{
	const CorbaObjectProxy *node1 = p1, *node2 = p2;

	return G_BSEARCH_ARRAY_CMP (node1->type, node2->type);
}

static void
corba_object_proxy_value_init (GValue *value)
{
	CorbaObjectProxy key, *node;

	key.type = G_VALUE_TYPE (value);
	node = g_bsearch_array_lookup (&corba_object_proxy_bsa, &key);
	/* [FIXME!] value->data[0].v_pointer = node->init ? node->init () : NULL; */
}

static void
corba_object_proxy_value_free (GValue *value)
{
	if (value->data[0].v_pointer) {
		CorbaObjectProxy key, *node;
		CORBA_Environment ev;

		key.type = G_VALUE_TYPE (value);
		node = g_bsearch_array_lookup (&corba_object_proxy_bsa, &key);

		CORBA_exception_init (&ev);
		if (node->is_bonobo_unknown)
			bonobo_object_release_unref (value->data[0].v_pointer, &ev);
		else
			CORBA_Object_release (value->data[0].v_pointer, &ev);
		CORBA_exception_free (&ev);
	}
}

static void
corba_object_proxy_value_copy (const GValue *src_value,
			       GValue       *dest_value)
{
	if (src_value->data[0].v_pointer) {
		CorbaObjectProxy key, *node;
		CORBA_Environment ev;

		key.type = G_VALUE_TYPE (src_value);
		node = g_bsearch_array_lookup (&corba_object_proxy_bsa, &key);

		CORBA_exception_init (&ev);
		if (node->is_bonobo_unknown)
			dest_value->data[0].v_pointer = bonobo_object_dup_ref (
				src_value->data[0].v_pointer, &ev);
		else
			dest_value->data[0].v_pointer = CORBA_Object_duplicate (
				src_value->data[0].v_pointer, &ev);
		CORBA_exception_free (&ev);
	} else
		dest_value->data[0].v_pointer = NULL;
}

static gpointer
corba_object_proxy_value_peek_pointer (const GValue *value)
{
	return value->data[0].v_pointer;
}

static gchar*
corba_object_proxy_collect_value (GValue      *value,
				  guint        n_collect_values,
				  GTypeCValue *collect_values,
				  guint        collect_flags)
{
	CorbaObjectProxy key, *node;

	key.type = G_VALUE_TYPE (value);
	node = g_bsearch_array_lookup (&corba_object_proxy_bsa, &key);

	if (!collect_values[0].v_pointer)
		value->data[0].v_pointer = NULL;
	else {
		CORBA_Environment ev;
		CORBA_Object corba_objref;

		corba_objref = collect_values[0].v_pointer;

		CORBA_exception_init (&ev);
		if (!CORBA_Object_is_a (corba_objref, node->tc->repo_id, &ev))
		    return g_strdup_printf ("CORBA Object %p is not a `%s'.",
					    corba_objref, node->tc->repo_id);

		if (node->is_bonobo_unknown)
			value->data[0].v_pointer = bonobo_object_dup_ref (corba_objref, &ev);
		else
			value->data[0].v_pointer = CORBA_Object_duplicate (corba_objref, &ev);
		CORBA_exception_free (&ev);
	}

	return NULL;
}

static gchar*
corba_object_proxy_lcopy_value (const GValue *value,
				guint         n_collect_values,
				GTypeCValue  *collect_values,
				guint         collect_flags)
{
	gpointer *corba_p = collect_values[0].v_pointer;

	if (!corba_p)
		return g_strdup_printf ("value location for `%s' passed as NULL", G_VALUE_TYPE_NAME (value));

	if (!value->data[0].v_pointer)
		*corba_p = NULL;
	else {
		CorbaObjectProxy key, *node;
		CORBA_Environment ev;

		key.type = G_VALUE_TYPE (value);
		node = g_bsearch_array_lookup (&corba_object_proxy_bsa, &key);

		CORBA_exception_init (&ev);
		if (node->is_bonobo_unknown)
			*corba_p = bonobo_object_dup_ref (value->data[0].v_pointer, &ev);
		else
			*corba_p = CORBA_Object_duplicate (value->data[0].v_pointer, &ev);
		CORBA_exception_free (&ev);
	}

  return NULL;
}

GType
bonobo_corba_object_type_register_static (const gchar *name, const CORBA_TypeCode tc,
					  gboolean is_bonobo_unknown)
{
	static const GTypeValueTable vtable = {
		corba_object_proxy_value_init,
		corba_object_proxy_value_free,
		corba_object_proxy_value_copy,
		corba_object_proxy_value_peek_pointer,
		"p",
		corba_object_proxy_collect_value,
		"p",
		corba_object_proxy_lcopy_value,
	};
	static const GTypeInfo type_info = {
		0,		/* class_size */
		NULL,		/* base_init */
		NULL,		/* base_finalize */
		NULL,		/* class_init */
		NULL,		/* class_finalize */
		NULL,		/* class_data */
		0,		/* instance_size */
		0,		/* n_preallocs */
		NULL,		/* instance_init */
		&vtable,	/* value_table */
	};
	GType type;

	g_return_val_if_fail (tc != NULL, 0);
	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (g_type_from_name (name) == 0, 0);

	type = g_type_register_static (G_TYPE_BOXED, name, &type_info, 0);

	/* install proxy functions upon successfull registration */
	if (type) {
		CorbaObjectProxy key;
		CORBA_Environment ev;

		key.type = type;
		CORBA_exception_init (&ev);
		key.tc = (CORBA_TypeCode) CORBA_Object_duplicate ((CORBA_Object) tc, &ev);
		CORBA_exception_init (&ev);
		key.is_bonobo_unknown = is_bonobo_unknown;
		g_bsearch_array_insert (&corba_object_proxy_bsa, &key, TRUE);
		CORBA_exception_free (&ev);
	}
	
	return type;
}

#define BONOBO_TYPE_CORBA_OBJECT_IMPL(name,typename,tc,is_bonobo_unknown)	\
GType										\
bonobo_ ## name ## _get_type (void)						\
{										\
	static GType type = 0;							\
	if (!type)								\
		type = bonobo_corba_object_type_register_static (		\
			## typename, ## tc, is_bonobo_unknown);			\
	return type;								\
}

BONOBO_TYPE_CORBA_OBJECT_IMPL (corba_object, "CorbaObject", TC_CORBA_Object, FALSE);
BONOBO_TYPE_CORBA_OBJECT_IMPL (corba_exception, "CorbaException", TC_CORBA_exception_type, FALSE);
BONOBO_TYPE_CORBA_OBJECT_IMPL (unknown, "BonoboUnknown", TC_Bonobo_Unknown, TRUE);

static gpointer
corba_any_init (void)
{
	return CORBA_any__alloc ();
}

static gpointer
corba_any_copy (gpointer any)
{
	return bonobo_arg_copy (any);
}

static void
corba_any_free (gpointer any)
{
	bonobo_arg_release (any);
}

GType
bonobo_corba_any_get_type (void)
{
	static GType type = 0;
	if (!type)
		type = g_boxed_type_register_static ("BonoboCorbaAny", corba_any_init,
						     corba_any_copy, corba_any_free, TRUE);
	return type;
}


