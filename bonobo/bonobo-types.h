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

#include <glib/gmacros.h>
#include <gobject/gobject.h>
#include <bonobo/bonobo-object.h>

G_BEGIN_DECLS

GType  bonobo_corba_object_type_register_static     (const gchar           *name,
                                                     const CORBA_TypeCode   tc,
						     gboolean               is_bonobo_unknown);

#define BONOBO_TYPE_CORBA_OBJECT                    (bonobo_corba_object_get_type ())
GType bonobo_corba_object_get_type                  (void);

#define BONOBO_TYPE_CORBA_EXCEPTION                 (bonobo_corba_exception_get_type ())
GType bonobo_corba_exception_get_type               (void);

#define BONOBO_TYPE_UNKNOWN                         (bonobo_unknown_get_type ())
GType bonobo_unknown_get_type                       (void);

#define BONOBO_TYPE_CORBA_ANY                       (bonobo_corba_any_get_type ())
GType bonobo_corba_any_get_type                     (void);

G_END_DECLS

#endif
