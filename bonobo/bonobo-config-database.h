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

typedef struct _BonoboConfigDatabasePrivate BonoboConfigDatabasePrivate;
typedef struct _BonoboConfigDatabase        BonoboConfigDatabase;

struct _BonoboConfigDatabase {
	BonoboXObject base;

	gboolean writeable;

	BonoboConfigDatabasePrivate *priv;
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

	void            (*remove_value) (BonoboConfigDatabase *db,
					 const CORBA_char     *key, 
					 CORBA_Environment    *ev);

	void            (*remove_dir)   (BonoboConfigDatabase *db,
					 const CORBA_char     *dir, 
					 CORBA_Environment    *ev);

	void            (*sync)         (BonoboConfigDatabase *db, 
					 CORBA_Environment    *ev);

} BonoboConfigDatabaseClass;


GtkType		      
bonobo_config_database_get_type  (void);


char *
bonobo_config_get_string (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_Environment     *opt_ev);
gint32 
bonobo_config_get_long   (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_Environment     *opt_ev);

gfloat 
bonobo_config_get_float  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_Environment     *opt_ev);

gdouble 
bonobo_config_get_double (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_Environment     *opt_ev);

gboolean
bonobo_config_get_bool   (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_Environment     *opt_ev);

CORBA_any *
bonobo_config_get_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_TypeCode         opt_tc,
			  CORBA_Environment     *opt_ev);

void
bonobo_config_set_string (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  const char            *value,
			  CORBA_Environment     *opt_ev);

void
bonobo_config_set_long   (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  gint32                 value,
			  CORBA_Environment     *opt_ev);

void
bonobo_config_set_float  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  gfloat                 value,
			  CORBA_Environment     *opt_ev);

void
bonobo_config_set_double (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  gdouble                value,
			  CORBA_Environment     *opt_ev);

void
bonobo_config_set_bool   (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  gboolean               value,
			  CORBA_Environment     *opt_ev);

void
bonobo_config_set_value  (Bonobo_ConfigDatabase  db,
			  const char            *key,
			  CORBA_any             *value,
			  CORBA_Environment     *opt_ev);

END_GNOME_DECLS

#endif /* ! __BONOBO_CONFIG_DATABASE_H__ */
