/**
 * bonobo-config-database.c: config database object implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include "config.h"

#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-config-database.h>

#define MAKE_GET_SIMPLE(c_type, default, name, corba_tc, extract_fn)          \
c_type bonobo_config_get_##name  (Bonobo_ConfigDatabase  db,                  \
				 const char            *key,                  \
				 CORBA_Environment     *opt_ev)               \
{                                                                             \
	CORBA_any *value;                                                     \
	c_type retval;                                                        \
	if (!(value = bonobo_config_get_value (db, key, corba_tc, opt_ev)))   \
		return default;                                               \
	retval = extract_fn;                                                  \
	CORBA_free (value);                                                   \
	return retval;                                                        \
}

#define MAKE_GET_WITH_DEFAULT(c_type, name, assign_fn)                        \
c_type bonobo_config_get_##name##_with_default (Bonobo_ConfigDatabase  db,    \
						const char            *key,   \
						c_type                 defval,\
						gboolean              *def)   \
{                                                                             \
	c_type retval;                                                        \
	CORBA_Environment ev;                                                 \
	CORBA_exception_init (&ev);                                           \
        if (def) *def = FALSE;                                                \
	retval = bonobo_config_get_##name (db, key, &ev);                     \
	if (BONOBO_EX (&ev)) {                                                \
		retval = assign_fn (defval);                                  \
                if (def) *def = TRUE;                                         \
        }                                                                     \
	CORBA_exception_free (&ev);                                           \
	return retval;                                                        \
}

/**
 * bonobo_config_get_string:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a string from the configuration database
 *
 * Returns: the value contained in the database, or zero on error.
 */
MAKE_GET_SIMPLE (gchar *, NULL, string, TC_string, 
		 g_strdup (*(char **)value->_value));

MAKE_GET_WITH_DEFAULT (gchar *, string, g_strdup);

/**
 * bonobo_config_get_short:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 16 bit integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gint16, 0, short, TC_short, (*(gint16 *)value->_value));

MAKE_GET_WITH_DEFAULT (gint16, short, );

/**
 * bonobo_config_get_ushort:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 16 bit unsigned integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (guint16, 0, ushort, TC_ushort, (*(guint16 *)value->_value));

MAKE_GET_WITH_DEFAULT (guint16, ushort, );

/**
 * bonobo_config_get_long:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 32 bit integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gint32, 0, long, TC_long, (*(gint32 *)value->_value));

MAKE_GET_WITH_DEFAULT (gint32, long, );

/**
 * bonobo_config_get_ulong:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 32 bit unsigned integer from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (guint32, 0, ulong, TC_ulong, (*(guint32 *)value->_value));

MAKE_GET_WITH_DEFAULT (guint32, ulong, );

/**
 * bonobo_config_get_float:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a single precision floating point value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gfloat, 0.0, float, TC_float, (*(gfloat *)value->_value));

MAKE_GET_WITH_DEFAULT (gfloat, float, );

/**
 * bonobo_config_get_double:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a double precision floating point value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gdouble, 0.0, double, TC_double, (*(gdouble *)value->_value));

MAKE_GET_WITH_DEFAULT (gdouble, double, );

/**
 * bonobo_config_get_char:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a 8 bit character value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gchar, '\0', char, TC_char, (*(gchar *)value->_value));

MAKE_GET_WITH_DEFAULT (gchar, char, );

/**
 * bonobo_config_get_boolean:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a boolean value from the configuration database
 *
 * Returns: the value contained in the database.
 */
MAKE_GET_SIMPLE (gboolean, FALSE, boolean, TC_boolean, 
		 (*(gboolean *)value->_value));

MAKE_GET_WITH_DEFAULT (gboolean, boolean, );

/**
 * bonobo_config_get_value:
 * @db: a reference to the database object
 * @key: key of the value to get
 * @opt_tc: the type of the value, optional
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Get a value from the configuration database
 *
 * Returns: the value contained in the database, or zero on error.
 */
CORBA_any *
bonobo_config_get_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_TypeCode         opt_tc,
			  CORBA_Environment     *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_any *retval;
	const char *locale;

	bonobo_return_val_if_fail (db != CORBA_OBJECT_NIL, NULL, opt_ev);
	bonobo_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	if (!(locale = g_getenv ("LANG")))
		locale = "";

	retval = Bonobo_ConfigDatabase_getValue (db, key, locale, my_ev);

	if (BONOBO_EX (my_ev)) {
		if (!opt_ev) {
			g_warning ("Cannot get value: %s\n", 
				   bonobo_exception_get_text (my_ev));
			
			CORBA_exception_free (&ev);
		}
		return NULL;
	}


	if (retval && opt_tc != CORBA_OBJECT_NIL) {

		/* fixme: we can also try to do automatic type conversions */

		if (!CORBA_TypeCode_equal (opt_tc, retval->_type, my_ev)) {
			CORBA_free (retval);
			if (!opt_ev)
				CORBA_exception_free (&ev);
			bonobo_exception_set (opt_ev, 
			        ex_Bonobo_ConfigDatabase_InvalidType);
			return NULL;
		}

	}

	if (!opt_ev)
		CORBA_exception_free (&ev);

	return retval;
}

#define MAKE_SET_SIMPLE(c_type, name, corba_tc)                               \
void bonobo_config_set_##name (Bonobo_ConfigDatabase  db,                     \
			       const char            *key,                    \
			       const c_type           value,                  \
			       CORBA_Environment     *opt_ev)                 \
{                                                                             \
	CORBA_any *any;                                                       \
	bonobo_return_if_fail (db != CORBA_OBJECT_NIL, opt_ev);               \
	bonobo_return_if_fail (key != NULL, opt_ev);                          \
	any = bonobo_arg_new (corba_tc);                                      \
	*((c_type *)(any->_value)) = value;                                   \
	bonobo_config_set_value (db, key, any, opt_ev);                       \
	bonobo_arg_release (any);                                             \
}

/**
 * bonobo_config_set_short:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 16 bit integer value in the configuration database.
 */
MAKE_SET_SIMPLE (gint16, short, TC_short)
/**
 * bonobo_config_set_ushort:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 16 bit unsigned integer value in the configuration database.
 */
MAKE_SET_SIMPLE (guint16, ushort, TC_ushort)
/**
 * bonobo_config_set_long:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 32 bit integer value in the configuration database.
 */
MAKE_SET_SIMPLE (gint32, long, TC_long)
/**
 * bonobo_config_set_ulong:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 32 bit unsigned integer value in the configuration database.
 */
MAKE_SET_SIMPLE (guint32, ulong, TC_ulong)
/**
 * bonobo_config_set_float:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a single precision floating point value in the configuration database.
 */
MAKE_SET_SIMPLE (gfloat, float, TC_float)
/**
 * bonobo_config_set_double:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a double precision floating point value in the configuration database.
 */
MAKE_SET_SIMPLE (gdouble, double, TC_double)
/**
 * bonobo_config_set_boolean:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a boolean value in the configuration database.
 */
MAKE_SET_SIMPLE (gboolean, boolean, TC_boolean)
/**
 * bonobo_config_set_char:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a 8 bit characte value in the configuration database.
 */
MAKE_SET_SIMPLE (gchar, char, TC_char)
/**
 * bonobo_config_set_string:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a string value in the configuration database.
 */
void
bonobo_config_set_string (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  const char            *value,
			  CORBA_Environment     *opt_ev)
{
	CORBA_any *any;

	bonobo_return_if_fail (db != CORBA_OBJECT_NIL, opt_ev);
	bonobo_return_if_fail (key != NULL, opt_ev);
	bonobo_return_if_fail (value != NULL, opt_ev);

	any = bonobo_arg_new (TC_string);

	BONOBO_ARG_SET_STRING (any, value);

	bonobo_config_set_value (db, key, any, opt_ev);

	bonobo_arg_release (any);
}

/**
 * bonobo_config_set_value:
 * @db: a reference to the database object
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a value in the configuration database.
 */
void
bonobo_config_set_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_any             *value,
			  CORBA_Environment     *opt_ev)
{
	CORBA_Environment ev, *my_ev;

	bonobo_return_if_fail (db != CORBA_OBJECT_NIL, opt_ev);
	bonobo_return_if_fail (key != NULL, opt_ev);
	bonobo_return_if_fail (value != NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;
	
	Bonobo_ConfigDatabase_setValue (db, key, value, my_ev);
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}

