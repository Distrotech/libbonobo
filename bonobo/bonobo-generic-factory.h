/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME GenericFactory object.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _GNOME_GENERIC_FACTORY_H_
#define _GNOME_GENERIC_FACTORY_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>
#include <bonobo/gnome-object.h>

BEGIN_GNOME_DECLS
 
#define GNOME_GENERIC_FACTORY_TYPE        (gnome_generic_factory_get_type ())
#define GNOME_GENERIC_FACTORY(o)          (GTK_CHECK_CAST ((o), GNOME_GENERIC_FACTORY_TYPE, GnomeGenericFactory))
#define GNOME_GENERIC_FACTORY_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_GENERIC_FACTORY_TYPE, GnomeGenericFactoryClass))
#define GNOME_IS_GENERIC_FACTORY(o)       (GTK_CHECK_TYPE ((o), GNOME_GENERIC_FACTORY_TYPE))
#define GNOME_IS_GENERIC_FACTORY_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_GENERIC_FACTORY_TYPE))

struct _GnomeGenericFactory;
typedef struct _GnomeGenericFactory GnomeGenericFactory;
typedef struct _GnomeGenericFactoryPrivate GnomeGenericFactoryPrivate;

typedef GnomeObject * (*GnomeGenericFactoryFn)(GnomeGenericFactory *Factory, void *closure);
typedef GnomeObject * (*GnomeFactoryCallback)(GnomeGenericFactory *factory, const char *goad_id, gpointer closure);
					
struct _GnomeGenericFactory {
	GnomeObject base;

	/*
	 * The function factory
	 */
	GnomeGenericFactoryFn factory; /* compat reasons only */
	GnomeFactoryCallback factory_cb;
	gpointer factory_closure;

	/*
	 * The goad_id for this generic factory
	 */
	char *goad_id;
};

typedef struct {
	GnomeObjectClass parent_class;

	/*
	 * Virtual methods
	 */
	GnomeObject * (*new_generic)(GnomeGenericFactory *c_factory, const char *goad_id);
} GnomeGenericFactoryClass;

GtkType gnome_generic_factory_get_type  (void);

GnomeGenericFactory *gnome_generic_factory_new (
	const char *goad_id,
	GnomeGenericFactoryFn factory,
	void *data);

GnomeGenericFactory *gnome_generic_factory_new_multi (
	const char *goad_id,
	GnomeFactoryCallback factory_cb,
	gpointer data);

GnomeGenericFactory *gnome_generic_factory_construct (
	const char *goad_id,
	GnomeGenericFactory  *c_factory,
	CORBA_Object          corba_factory,
	GnomeGenericFactoryFn factory,
	GnomeFactoryCallback factory_cb,
	void *data);

void gnome_generic_factory_set (
	GnomeGenericFactory *c_factory,
	GnomeGenericFactoryFn factory,
	void *data);

POA_GNOME_GenericFactory__epv *gnome_generic_factory_get_epv (void);

extern POA_GNOME_GenericFactory__vepv gnome_generic_factory_vepv;

END_GNOME_DECLS

#endif
