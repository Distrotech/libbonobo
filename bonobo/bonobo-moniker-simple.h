/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-moniker-simple: Simplified object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#ifndef _BONOBO_MONIKER_SIMPLE_SIMPLE_H_
#define _BONOBO_MONIKER_SIMPLE_SIMPLE_H_

#include <bonobo/bonobo-types.h>
#include <bonobo/bonobo-moniker.h>

G_BEGIN_DECLS

#define BONOBO_MONIKER_SIMPLE_TYPE        (bonobo_moniker_simple_get_type ())
#define BONOBO_MONIKER_SIMPLE(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_MONIKER_SIMPLE_TYPE, BonoboMonikerSimple))
#define BONOBO_MONIKER_SIMPLE_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_MONIKER_SIMPLE_TYPE, BonoboMonikerSimpleClass))
#define BONOBO_IS_MONIKER_SIMPLE(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_MONIKER_SIMPLE_TYPE))
#define BONOBO_IS_MONIKER_SIMPLE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_MONIKER_SIMPLE_TYPE))

typedef struct _BonoboMonikerSimple        BonoboMonikerSimple;
typedef struct _BonoboMonikerSimplePrivate BonoboMonikerSimplePrivate;

typedef Bonobo_Unknown (*BonoboMonikerSimpleResolveFn) (BonoboMoniker               *moniker,
							const Bonobo_ResolveOptions *options,
							const CORBA_char            *requested_interface,
							CORBA_Environment           *ev);

struct _BonoboMonikerSimple {
        BonoboMoniker                moniker;

	BonoboMonikerSimplePrivate  *priv;
};

typedef struct {
	BonoboMonikerClass parent_class;
} BonoboMonikerSimpleClass;

GType          bonobo_moniker_simple_get_type    (void) G_GNUC_CONST;

BonoboMoniker *bonobo_moniker_simple_construct   (BonoboMonikerSimple         *moniker,
						  const char                  *name,
						  GClosure                    *resolve_closure);

BonoboMoniker *bonobo_moniker_simple_new         (const char                  *name,
						  BonoboMonikerSimpleResolveFn resolve_fn);

BonoboMoniker *bonobo_moniker_simple_new_closure (const char                  *name,
						  GClosure                    *resolve_closure);

G_END_DECLS

#endif /* _BONOBO_MONIKER_SIMPLE_H_ */

