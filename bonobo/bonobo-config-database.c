/**
 * bonobo-config-database.c: config database object implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-arg.h>

#include "bonobo-config-database.h"


static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_DATABASE_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE (BONOBO_X_OBJECT_TYPE)

#define DATABASE_FROM_SERVANT(servant) (BONOBO_CONFIG_DATABASE (bonobo_object_from_servant (servant)))

typedef struct {
	Bonobo_ConfigDatabase db;
	char *filter_path;
	char *append_path;
} DataBaseInfo;

struct _BonoboConfigDatabasePrivate {
	GList *db_list;
};

static void
insert_key_name (gpointer	key,
		 gpointer	value,
		 gpointer	user_data)
{
	Bonobo_KeyList *key_list = (Bonobo_KeyList *)user_data;

	key_list->_buffer [key_list->_length++] = CORBA_string_dup (key);
}

Bonobo_KeyList *
merge_keylists (Bonobo_KeyList *cur_list, 
		Bonobo_KeyList *def_list)
{
	Bonobo_KeyList *key_list;
	GHashTable     *ht;
	int             i, len;

	ht =  g_hash_table_new (g_str_hash, g_str_equal);

	for (i = 0; i < cur_list->_length; i++) 
		g_hash_table_insert (ht, cur_list->_buffer [i], NULL);

	for (i = 0; i < def_list->_length; i++)
		g_hash_table_insert (ht, def_list->_buffer [i], NULL);

	len =  g_hash_table_size (ht);

	key_list = Bonobo_KeyList__alloc ();
	key_list->_length = 0;
	key_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);
	CORBA_sequence_set_release (key_list, TRUE); 

	g_hash_table_foreach (ht, insert_key_name, key_list);

	g_hash_table_destroy (ht);

	return key_list;
}

static CORBA_any *
get_default (BonoboConfigDatabase   *cd,
	     const CORBA_char       *key, 
	     CORBA_Environment      *ev)
{
	CORBA_any *value = NULL;
	DataBaseInfo *info;
	GList *l;

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		value = Bonobo_ConfigDatabase_getValue (info->db, key, ev);
		if (BONOBO_EX (ev))
			return NULL;

		if (value)
			return value;

	}

	return NULL;
}

static CORBA_any *
impl_Bonobo_ConfigDatabase_getValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_any *value = NULL;

	if (CLASS (cd)->get_value)
		value = CLASS (cd)->get_value (cd, key, ev);

	if (value)
		return value;

	return get_default (cd, key, ev);
}

static void 
impl_Bonobo_ConfigDatabase_setValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     const CORBA_any        *value,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->set_value)
		CLASS (cd)->set_value (cd, key, value, ev);
}

static CORBA_any *
impl_Bonobo_ConfigDatabase_getDefault (PortableServer_Servant  servant,
				       const CORBA_char       *key, 
				       CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	return get_default (cd, key, ev);
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listDirs (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_Environment nev;
	DataBaseInfo *info;
	GList *l;
	Bonobo_KeyList *cur_list, *def_list, *tmp_list;

	cur_list = NULL;

	if (CLASS (cd)->list_keys)
		cur_list = CLASS (cd)->list_dirs (cd, dir, ev);
	
	if (BONOBO_EX (ev))
		return NULL;
	
	CORBA_exception_init (&nev);

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		CORBA_exception_init (&nev);

		def_list = Bonobo_ConfigDatabase_listDirs (info->db, dir,&nev);
		if (!BONOBO_EX (&nev) && def_list) {

			if (!def_list->_length) {
				CORBA_free (def_list);
				continue;
			}

			if (!cur_list || !cur_list->_length) {
				if (cur_list)
					CORBA_free (cur_list);
				cur_list = def_list;
				continue;
			}

			tmp_list = merge_keylists (cur_list, def_list);
			
			CORBA_free (cur_list);
			CORBA_free (def_list);

			cur_list = tmp_list;
		}
	}

	CORBA_exception_free (&nev);

	return cur_list;
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listKeys (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_Environment nev;
	DataBaseInfo *info;
	GList *l;
	Bonobo_KeyList *cur_list, *def_list, *tmp_list;

	cur_list = NULL;

	if (CLASS (cd)->list_keys)
		cur_list = CLASS (cd)->list_keys (cd, dir, ev);
	
	if (BONOBO_EX (ev))
		return NULL;

	CORBA_exception_init (&nev);

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		CORBA_exception_init (&nev);

		def_list = Bonobo_ConfigDatabase_listKeys (info->db, dir,&nev);
		if (!BONOBO_EX (&nev) && def_list) {

			if (!def_list->_length) {
				CORBA_free (def_list);
				continue;
			}

			if (!cur_list || !cur_list->_length) {
				if (cur_list)
					CORBA_free (cur_list);
				cur_list = def_list;
				continue;
			}

			tmp_list = merge_keylists (cur_list, def_list);
			
			CORBA_free (cur_list);
			CORBA_free (def_list);

			cur_list = tmp_list;
		}
	}

	CORBA_exception_free (&nev);

	return cur_list;
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase_dirExists (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	CORBA_Environment nev;
	DataBaseInfo *info;
	CORBA_boolean res;
	GList *l;

	if (CLASS (cd)->dir_exists &&
	    CLASS (cd)->dir_exists (cd, dir, ev))
		return TRUE;

	CORBA_exception_init (&nev);

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		CORBA_exception_init (&nev);
		
		res = Bonobo_ConfigDatabase_dirExists (info->db, dir, &nev);
		
		CORBA_exception_free (&nev);
		
		if (!BONOBO_EX (&nev) && res)
			return TRUE;
	}

	CORBA_exception_free (&nev);

	return CORBA_FALSE;
}

static void 
impl_Bonobo_ConfigDatabase_removeValue (PortableServer_Servant  servant,
					const CORBA_char       *key,
					CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->remove_value)
		CLASS (cd)->remove_value (cd, key, ev);
}

static void 
impl_Bonobo_ConfigDatabase_removeDir (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->remove_dir)
		CLASS (cd)->remove_dir (cd, dir, ev);
}

static void 
impl_Bonobo_ConfigDatabase_addDatabase (PortableServer_Servant servant,
					const Bonobo_ConfigDatabase ddb,
					const CORBA_char  *filter_path,
					const CORBA_char  *append_path,
					CORBA_Environment *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);
	DataBaseInfo *info;
	GList *l;

	/* we cant add ourselves */
	if (CORBA_Object_is_equivalent (BONOBO_OBJREF (cd), ddb, NULL))
		return;

	/* check if the database was already added */
	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;
		if (CORBA_Object_is_equivalent (info->db, ddb, NULL))
			return;
	}

	info = g_new0 (DataBaseInfo , 1);

	info->db = bonobo_object_dup_ref (ddb, ev);

	if (BONOBO_EX (ev)) {
		g_free (info);
		return;
	}
	
	info->filter_path = g_strdup (filter_path);
	info->append_path = g_strdup (append_path);

	cd->priv->db_list = g_list_append (cd->priv->db_list, info);
}

static void 
impl_Bonobo_ConfigDatabase_sync (PortableServer_Servant  servant, 
				 CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->sync)
		CLASS (cd)->sync (cd, ev);
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase__get_writeable (PortableServer_Servant  servant,
					   CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	return cd->writeable;
}

static void
bonobo_config_database_destroy (GtkObject *object)
{
	BonoboConfigDatabase *cd = BONOBO_CONFIG_DATABASE (object);
	DataBaseInfo *info;
	GList *l;

	for (l = cd->priv->db_list; l != NULL; l = l->next) {
		info = (DataBaseInfo *)l->data;

		bonobo_object_release_unref (info->db, NULL);

		if (info->filter_path)
			g_free (info->filter_path);
		
		if (info->append_path)
			g_free (info->append_path);

		g_free (info);
	}
	
	if (cd->priv->db_list)
		g_list_free (cd->priv->db_list);

	g_free (cd->priv);
	
	parent_class->destroy (object);
}

static void
bonobo_config_database_class_init (BonoboConfigDatabaseClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	POA_Bonobo_ConfigDatabase__epv *epv;
		
	parent_class = gtk_type_class (PARENT_TYPE);

	object_class->destroy = bonobo_config_database_destroy;

	epv = &class->epv;

	epv->getValue       = impl_Bonobo_ConfigDatabase_getValue;
	epv->setValue       = impl_Bonobo_ConfigDatabase_setValue;
	epv->getDefault     = impl_Bonobo_ConfigDatabase_getDefault;
	epv->listDirs       = impl_Bonobo_ConfigDatabase_listDirs;
	epv->listKeys       = impl_Bonobo_ConfigDatabase_listKeys;
	epv->dirExists      = impl_Bonobo_ConfigDatabase_dirExists;
	epv->removeValue    = impl_Bonobo_ConfigDatabase_removeValue;
	epv->removeDir      = impl_Bonobo_ConfigDatabase_removeDir;
	epv->addDatabase    = impl_Bonobo_ConfigDatabase_addDatabase;
	epv->sync           = impl_Bonobo_ConfigDatabase_sync;


	epv->_get_writeable = impl_Bonobo_ConfigDatabase__get_writeable;
}

static void
bonobo_config_database_init (BonoboConfigDatabase *cd)
{
	cd->priv = g_new0 (BonoboConfigDatabasePrivate, 1);
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigDatabase, 
			 Bonobo_ConfigDatabase,
			 PARENT_TYPE,
			 bonobo_config_database);

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

MAKE_GET_SIMPLE (gchar *, NULL, string, TC_string, 
		 g_strdup (*(char **)value->_value));
MAKE_GET_SIMPLE (gint16, 0, short, TC_short, (*(gint16 *)value->_value));
MAKE_GET_SIMPLE (guint16, 0, ushort, TC_ushort, (*(guint16 *)value->_value));
MAKE_GET_SIMPLE (gint32, 0, long, TC_long, (*(gint32 *)value->_value));
MAKE_GET_SIMPLE (guint32, 0, ulong, TC_ulong, (*(guint32 *)value->_value));
MAKE_GET_SIMPLE (gfloat, 0.0, float, TC_float, (*(gfloat *)value->_value));
MAKE_GET_SIMPLE (gdouble, 0.0, double, TC_double, (*(gdouble *)value->_value));
MAKE_GET_SIMPLE (gchar, '\0', char, TC_char, (*(gchar *)value->_value));
MAKE_GET_SIMPLE (gboolean, FALSE, boolean, TC_boolean, 
		 (*(gboolean *)value->_value));

CORBA_any *
bonobo_config_get_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_TypeCode         opt_tc,
			  CORBA_Environment     *opt_ev)
{
	CORBA_Environment ev, *my_ev;
	CORBA_any *retval;

	bonobo_return_val_if_fail (db != CORBA_OBJECT_NIL, NULL, opt_ev);
	bonobo_return_val_if_fail (key != NULL, NULL, opt_ev);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	retval = Bonobo_ConfigDatabase_getValue (db, key, my_ev);
	
	if (BONOBO_EX (my_ev) && !opt_ev)
		g_warning ("Cannot get value: %s\n", 
			   bonobo_exception_get_text (my_ev));

	if (retval && opt_tc != CORBA_OBJECT_NIL) {

		/* fixme: we can also try to do automatic type conversions */

		if (!CORBA_TypeCode_equal (opt_tc, retval->_type, my_ev)) {
			CORBA_free (retval);
			if (!opt_ev)
				CORBA_exception_free (&ev);
			/* fixme: we need an InvalidType exception */
			bonobo_exception_set (opt_ev, ex_Bonobo_BadArg);
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

MAKE_SET_SIMPLE (gint16, short, TC_short)
MAKE_SET_SIMPLE (guint16, ushort, TC_ushort)
MAKE_SET_SIMPLE (gint32, long, TC_long)
MAKE_SET_SIMPLE (guint32, ulong, TC_ulong)
MAKE_SET_SIMPLE (gfloat, float, TC_float)
MAKE_SET_SIMPLE (gdouble, double, TC_double)
MAKE_SET_SIMPLE (gboolean, boolean, TC_boolean)
MAKE_SET_SIMPLE (gchar, char, TC_char)

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
	
	if (BONOBO_EX (my_ev) && !opt_ev)
		g_warning ("Cannot set value: %s\n", 
			   bonobo_exception_get_text (my_ev));

	if (!opt_ev)
		CORBA_exception_free (&ev);
}

