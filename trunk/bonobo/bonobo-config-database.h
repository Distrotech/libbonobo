/**
 * bonobo-config-database.h: config database client methods.
 *
 * Author:
 *   Dietmar Maurer  (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef __BONOBO_CONFIG_DATABASE_H__
#define __BONOBO_CONFIG_DATABASE_H__

#include <bonobo/Bonobo.h>

G_BEGIN_DECLS

gchar *
bonobo_config_get_string               (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gchar *
bonobo_config_get_string_with_default  (Bonobo_ConfigDatabase  db,
				        const char            *key,
				        gchar                 *defval,
				        gboolean              *def);
gint16 
bonobo_config_get_short                (Bonobo_ConfigDatabase  db,
			                const char            *key,
			                CORBA_Environment     *opt_ev);
gint16 
bonobo_config_get_short_with_default   (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint16                 defval,
					gboolean              *def);
guint16 
bonobo_config_get_ushort               (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
guint16 
bonobo_config_get_ushort_with_default  (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint16                defval,
					gboolean              *def);
gint32 
bonobo_config_get_long                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gint32 
bonobo_config_get_long_with_default    (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint32                 defval,
					gboolean              *def);
guint32 
bonobo_config_get_ulong                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
guint32 
bonobo_config_get_ulong_with_default   (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint32                defval,
					gboolean              *def);
gfloat 
bonobo_config_get_float                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gfloat 
bonobo_config_get_float_with_default   (Bonobo_ConfigDatabase  db,
					const char            *key,
					gfloat                 defval,
					gboolean              *def);
gdouble 
bonobo_config_get_double               (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gdouble 
bonobo_config_get_double_with_default  (Bonobo_ConfigDatabase  db,
					const char            *key,
					gdouble                defval,
					gboolean              *def);
gboolean
bonobo_config_get_boolean              (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gboolean 
bonobo_config_get_boolean_with_default (Bonobo_ConfigDatabase  db,
					const char            *key,
					gboolean               defval,
					gboolean              *def);
gchar
bonobo_config_get_char                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_Environment     *opt_ev);
gchar 
bonobo_config_get_char_with_default    (Bonobo_ConfigDatabase  db,
					const char            *key,
					gchar                  defval,
					gboolean              *def);
CORBA_any *
bonobo_config_get_value                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_TypeCode         opt_tc,
					CORBA_Environment     *opt_ev);

void
bonobo_config_set_string               (Bonobo_ConfigDatabase  db,
					const char            *key,
					const char            *value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_short                (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint16                 value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_ushort               (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint16                value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_long                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					gint32                 value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_ulong                (Bonobo_ConfigDatabase  db,
					const char            *key,
					guint32                value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_float                (Bonobo_ConfigDatabase  db,
					const char            *key,
					gfloat                 value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_double               (Bonobo_ConfigDatabase  db,
					const char            *key,
					gdouble                value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_boolean              (Bonobo_ConfigDatabase  db,
					const char            *key,
					gboolean               value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_char                 (Bonobo_ConfigDatabase  db,
					const char            *key,
					gchar                  value,
					CORBA_Environment     *opt_ev);
void
bonobo_config_set_value                (Bonobo_ConfigDatabase  db,
					const char            *key,
					CORBA_any             *value,
					CORBA_Environment     *opt_ev);

G_END_DECLS

#endif /* ! __BONOBO_CONFIG_DATABASE_H__ */
