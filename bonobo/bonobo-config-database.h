/**
 * bonobo-config-database.h: config database object implementation.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_DATABASE_H__
#define __BONOBO_CONFIG_DATABASE_H__

#include <bonobo/bonobo-xobject.h>
#include "Bonobo_Config.h"

BEGIN_GNOME_DECLS

#define BONOBO_CONFIG_DATABASE_TYPE        (bonobo_config_database_get_type ())
#define BONOBO_CONFIG_DATABASE(o)	   (GTK_CHECK_CAST ((o), BONOBO_CONFIG_DATABASE_TYPE, BonoboConfigDatabase))
#define BONOBO_CONFIG_DATABASE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONFIG_DATABASE_TYPE, BonoboConfigDatabaseClass))
#define BONOBO_IS_CONFIG_DATABASE(o)	   (GTK_CHECK_TYPE ((o), BONOBO_CONFIG_DATABASE_TYPE))
#define BONOBO_IS_CONFIG_DATABASE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONFIG_DATABASE_TYPE))

typedef struct _BonoboConfigDatabase        BonoboConfigDatabase;

struct _BonoboConfigDatabase {
	BonoboXObject       base;
};

typedef struct {
	BonoboXObjectClass  parent_class;

	POA_Bonobo_ConfigDatabase__epv epv;

        /*
         * virtual methods
         */

	CORBA_any      *(*get_value)    (BonoboConfigDatabase *db,
					 const CORBA_char     *key, 
					 CORBA_Environment    *ev);

	void            (*set_value)    (BonoboConfigDatabase *db,
					 const CORBA_char     *key, 
					 const CORBA_any      *value,
					 CORBA_Environment    *ev);

	Bonobo_KeyList *(*list_dirs)    (BonoboConfigDatabase *db,
					 const CORBA_char     *dir,
					 CORBA_Environment    *ev);

	Bonobo_KeyList *(*list_keys)    (BonoboConfigDatabase *db,
					 const CORBA_char     *dir,
					 CORBA_Environment    *ev);

	CORBA_boolean   (*dir_exists)   (BonoboConfigDatabase *db,
					 const CORBA_char     *dir,
					 CORBA_Environment    *ev);

	void            (*remove)       (BonoboConfigDatabase *db,
					 const CORBA_char     *path, 
					 CORBA_Environment    *ev);

	void            (*add_database) (BonoboConfigDatabase *db,
					 const Bonobo_ConfigDatabase ddb,
					 const CORBA_char     *filter_path,
					 const CORBA_char     *append_path,
					 CORBA_Environment    *ev);

	void            (*sync)         (BonoboConfigDatabase *db, 
					 CORBA_Environment    *ev);

} BonoboConfigDatabaseClass;


GtkType		      
bonobo_config_database_get_type  (void);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_DATABASE_H__ */
