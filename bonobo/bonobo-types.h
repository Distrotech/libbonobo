/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GRuntime types for CORBA Objects.
 *
 * Authors:
 *   Martin Baulig (baulig@suse.de)
 *
 * Copyright 2001 SuSE Linux AG.
 */
#ifndef _BONOBO_TYPES_H_
#define _BONOBO_TYPES_H_

#include <stdarg.h>
#include <glib/gmacros.h>
#include <gobject/gobject.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-arg.h>

G_BEGIN_DECLS

GType bonobo_corba_object_type_register_static      (const gchar           *name,
                                                     const CORBA_TypeCode   tc,
						     gboolean               is_bonobo_unknown) G_GNUC_CONST;

GType bonobo_unknown_get_type                       (void) G_GNUC_CONST;
GType bonobo_corba_any_get_type                     (void) G_GNUC_CONST;
GType bonobo_corba_object_get_type                  (void) G_GNUC_CONST;
GType bonobo_corba_exception_get_type               (void) G_GNUC_CONST;

#define BONOBO_TYPE_UNKNOWN                         (bonobo_unknown_get_type ())
#define BONOBO_TYPE_CORBA_ANY                       (bonobo_corba_any_get_type ())
#define BONOBO_TYPE_CORBA_OBJECT                    (bonobo_corba_object_get_type ())
#define BONOBO_TYPE_CORBA_EXCEPTION                 (bonobo_corba_exception_get_type ())

#define BONOBO_VALUE_HOLDS_UNKNOWN(value)           (G_TYPE_CHECK_VALUE_TYPE ((value), BONOBO_TYPE_UNKNOWN))
#define BONOBO_VALUE_HOLDS_CORBA_ANY(value)         (G_TYPE_CHECK_VALUE_TYPE ((value), BONOBO_TYPE_CORBA_ANY))
#define BONOBO_VALUE_HOLDS_CORBA_OBJECT(value)      (G_TYPE_CHECK_VALUE_TYPE ((value), BONOBO_TYPE_CORBA_OBJECT))
#define BONOBO_VALUE_HOLDS_CORBA_EXCEPTION(value)   (G_TYPE_CHECK_VALUE_TYPE ((value), BONOBO_TYPE_CORBA_EXCEPTION))

Bonobo_Unknown     bonobo_value_get_unknown         (GValue *value);
BonoboArg         *bonobo_value_get_corba_any       (GValue *value);
CORBA_Object       bonobo_value_get_corba_object    (GValue *value);
CORBA_Environment *bonobo_value_get_corba_exception (GValue *value);

void       bonobo_closure_invoke_va_list            (GClosure            *closure,
						     GValue              *retval,
						     GType                first_type,
						     va_list              var_args);

void       bonobo_closure_invoke		    (GClosure            *closure,
						     GValue              *retval,
						     GType                first_type,
						     ...);

GClosure * bonobo_closure_store                     (GClosure            *closure,
						     GClosureMarshal      default_marshal);
						     
G_END_DECLS

#endif
