/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-generic-factory.h: a GenericFactory object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *   ÉRDI Gergõ (cactus@cactus.rulez.org): cleanup
 *
 * Copyright 1999 Helix Code, Inc., 2001 Gergõ Érdi
 */
#ifndef _BONOBO_GENERIC_FACTORY_H_
#define _BONOBO_GENERIC_FACTORY_H_


#include <gobject/gobject.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-i18n.h>
#include <liboaf/oaf.h>
#include <liboaf/liboaf.h>

G_BEGIN_DECLS
 
#define BONOBO_GENERIC_FACTORY_TYPE        (bonobo_generic_factory_get_type ())
#define BONOBO_GENERIC_FACTORY(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_GENERIC_FACTORY_TYPE, BonoboGenericFactory))
#define BONOBO_GENERIC_FACTORY_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_GENERIC_FACTORY_TYPE, BonoboGenericFactoryClass))
#define BONOBO_IS_GENERIC_FACTORY(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_GENERIC_FACTORY_TYPE))
#define BONOBO_IS_GENERIC_FACTORY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_GENERIC_FACTORY_TYPE))

typedef struct _BonoboGenericFactoryPrivate BonoboGenericFactoryPrivate;
typedef struct _BonoboGenericFactory        BonoboGenericFactory;

typedef BonoboObject * (*BonoboFactoryCallback) (BonoboGenericFactory *factory, const char *component_id, gpointer closure);
					
struct _BonoboGenericFactory {
	GObject                      base;

	BonoboGenericFactoryPrivate *priv;
};

typedef struct {
	GObjectClass parent_class;

	/* Virtual methods */
	BonoboObject *(*new_generic) (BonoboGenericFactory *factory,
				      const char           *component_id);
} BonoboGenericFactoryClass;

GType                 bonobo_generic_factory_get_type  (void);

GNOME_ObjectFactory   bonobo_generic_factory_corba_objref (BonoboGenericFactory *object);

GNOME_ObjectFactory   bonobo_generic_factory_corba_object_create (BonoboGenericFactory *object, 
								  gpointer              shlib_id);

BonoboGenericFactory *bonobo_generic_factory_new (const char            *oaf_iid,
						  BonoboFactoryCallback  factory_cb,
						  gpointer               user_data);

BonoboGenericFactory *bonobo_generic_factory_construct (BonoboGenericFactory  *factory,
							GNOME_ObjectFactory    corba_factory,
							const char            *oaf_iid,
							BonoboFactoryCallback  factory_cb,
							gpointer               user_data);

POA_GNOME_ObjectFactory__epv *bonobo_generic_factory_get_epv (void);

#ifdef __BONOBO_UI_MAIN_H__
#define BONOBO_FACTORY_INIT(descr, version, argcp, argv)                      \
	if (!bonobo_ui_init (descr, version, argcp, argv))                    \
		g_error (_("Could not initialize Bonobo"));
#else
#define BONOBO_FACTORY_INIT(desc, version, argcp, argv)                       \
	if (!bonobo_init (argcp, argv))                                       \
		g_error (_("Could not initialize Bonobo"));
#endif

#define BONOBO_OAF_FACTORY(oafiid, descr, version, callback, data)            \
int main (int argc, char *argv [])                                            \
{                                                                             \
	BonoboGenericFactory *factory;                                        \
                                                                              \
	BONOBO_FACTORY_INIT (descr, version, &argc, argv);                    \
	factory = bonobo_generic_factory_new (oafiid, callback, data);        \
	bonobo_main ();                                                       \
	return 0;                                                             \
}                                                                             

#define BONOBO_OAF_FACTORY_MULTI(oafiid, descr, version, callback, data)      \
	BONOBO_OAF_FACTORY(oafiid, descr, version, callback, data)

G_END_DECLS

#endif
