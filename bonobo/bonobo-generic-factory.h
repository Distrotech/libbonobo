/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-generic-factory.h: a GenericFactory object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   �RDI Gerg� (cactus@cactus.rulez.org): cleanup
 *
 * Copyright 1999 Helix Code, Inc., 2001 Gerg� �rdi
 */
#ifndef _BONOBO_GENERIC_FACTORY_H_
#define _BONOBO_GENERIC_FACTORY_H_


#include <gobject/gobject.h>
#include <gobject/gclosure.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-i18n.h>

G_BEGIN_DECLS
 
#define BONOBO_GENERIC_FACTORY_TYPE        (bonobo_generic_factory_get_type ())
#define BONOBO_GENERIC_FACTORY(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_GENERIC_FACTORY_TYPE, BonoboGenericFactory))
#define BONOBO_GENERIC_FACTORY_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_GENERIC_FACTORY_TYPE, BonoboGenericFactoryClass))
#define BONOBO_IS_GENERIC_FACTORY(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_GENERIC_FACTORY_TYPE))
#define BONOBO_IS_GENERIC_FACTORY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_GENERIC_FACTORY_TYPE))

typedef struct _BonoboGenericFactoryPrivate BonoboGenericFactoryPrivate;
typedef struct _BonoboGenericFactory        BonoboGenericFactory;

typedef BonoboObject * (*BonoboFactoryCallback) (BonoboGenericFactory *factory,
						 const char           *component_id,
						 gpointer              closure);
					
struct _BonoboGenericFactory {
	BonoboObject                 base;

	BonoboGenericFactoryPrivate *priv;
};

typedef struct {
	BonoboObjectClass            parent_class;

	POA_Bonobo_GenericFactory__epv epv;

	BonoboObject *(*new_generic) (BonoboGenericFactory *factory,
				      const char           *oaf_iid);

} BonoboGenericFactoryClass;

GType                 bonobo_generic_factory_get_type  (void) G_GNUC_CONST;

BonoboGenericFactory *bonobo_generic_factory_new	 (const char            *oaf_iid,
							  BonoboFactoryCallback  factory_cb,
							  gpointer               user_data);

BonoboGenericFactory *bonobo_generic_factory_new_closure (const char            *oaf_iid,
							  GClosure              *factory_closure);

BonoboGenericFactory *bonobo_generic_factory_construct	 (BonoboGenericFactory  *factory,
							  const char            *oaf_iid,
							  GClosure              *factory_cb);

#ifdef __BONOBO_UI_MAIN_H__
#define BONOBO_FACTORY_INIT(descr, version, argcp, argv)                      \
	if (!bonobo_ui_init (descr, version, argcp, argv))                    \
		g_error (_("Could not initialize Bonobo"));
#else
#define BONOBO_FACTORY_INIT(desc, version, argcp, argv)                       \
	if (!bonobo_init (argcp, argv))                                       \
		g_error (_("Could not initialize Bonobo"));
#endif

#define BONOBO_OAF_FACTORY(oafiid, descr, version, callback, data)		\
int main (int argc, char *argv [])						\
{										\
	BonoboGenericFactory *factory;						\
										\
	BONOBO_FACTORY_INIT (descr, version, &argc, argv);			\
										\
	factory = bonobo_generic_factory_new (oafiid, callback, data);		\
	bonobo_main ();								\
	return 0;								\
}                                                                             

#define BONOBO_OAF_FACTORY_MULTI(oafiid, descr, version, callback, data)      \
	BONOBO_OAF_FACTORY(oafiid, descr, version, callback, data)

G_END_DECLS

#endif
