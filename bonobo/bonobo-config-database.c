/**
 * bonobo-config-database.c: config database object implementation.
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#include <config.h>

#include "bonobo-config-database.h"


static GtkObjectClass *parent_class = NULL;

#define CLASS(o) BONOBO_CONFIG_DATABASE_CLASS (GTK_OBJECT(o)->klass)

#define PARENT_TYPE (BONOBO_X_OBJECT_TYPE)

#define DATABASE_FROM_SERVANT(servant) (BONOBO_CONFIG_DATABASE (bonobo_object_from_servant (servant)))

static CORBA_any *
impl_Bonobo_ConfigDatabase_getValue (PortableServer_Servant  servant,
				     const CORBA_char       *key, 
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->get_value)
		return CLASS (cd)->get_value (cd, key, ev);

	return NULL;
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

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listDirs (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->list_dirs)
		return CLASS (cd)->list_dirs (cd, dir, ev);

	return NULL;
}

static Bonobo_KeyList *
impl_Bonobo_ConfigDatabase_listKeys (PortableServer_Servant  servant,
				     const CORBA_char       *dir,
				     CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->list_keys)
		return CLASS (cd)->list_keys (cd, dir, ev);

	return NULL;
}

static CORBA_boolean
impl_Bonobo_ConfigDatabase_dirExists (PortableServer_Servant  servant,
				      const CORBA_char       *dir,
				      CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->dir_exists)
		return CLASS (cd)->dir_exists (cd, dir, ev);

	return CORBA_FALSE;
}

static void 
impl_Bonobo_ConfigDatabase_remove (PortableServer_Servant  servant,
				   const CORBA_char       *path,
				   CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->remove)
		CLASS (cd)->remove (cd, path, ev);
}

static void 
impl_Bonobo_ConfigDatabase_addDatabase (PortableServer_Servant servant,
					const Bonobo_ConfigDatabase ddb,
					const CORBA_char  *filter_path,
					const CORBA_char  *append_path,
					CORBA_Environment *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->add_database)
		CLASS (cd)->add_database (cd, ddb, filter_path, 
					  append_path, ev);
}

static void 
impl_Bonobo_ConfigDatabase_sync (PortableServer_Servant  servant, 
				 CORBA_Environment      *ev)
{
	BonoboConfigDatabase *cd = DATABASE_FROM_SERVANT (servant);

	if (CLASS (cd)->sync)
		CLASS (cd)->sync (cd, ev);
}

static void
bonobo_config_database_class_init (BonoboConfigDatabaseClass *class)
{
	POA_Bonobo_ConfigDatabase__epv *epv;
		
	parent_class = gtk_type_class (PARENT_TYPE);

	epv = &class->epv;

	epv->getValue    = impl_Bonobo_ConfigDatabase_getValue;
	epv->setValue    = impl_Bonobo_ConfigDatabase_setValue;
	epv->listDirs    = impl_Bonobo_ConfigDatabase_listDirs;
	epv->listKeys    = impl_Bonobo_ConfigDatabase_listKeys;
	epv->dirExists   = impl_Bonobo_ConfigDatabase_dirExists;
	epv->remove      = impl_Bonobo_ConfigDatabase_remove;
	epv->addDatabase = impl_Bonobo_ConfigDatabase_addDatabase;
	epv->sync        = impl_Bonobo_ConfigDatabase_sync;
}

static void
bonobo_config_database_init (BonoboConfigDatabase *cd)
{
	/* nothing to do */
}

BONOBO_X_TYPE_FUNC_FULL (BonoboConfigDatabase, 
			 Bonobo_ConfigDatabase,
			 PARENT_TYPE,
			 bonobo_config_database);

