/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */

#ifndef _GNOME_CONTAINER_H_
#define _GNOME_CONTAINER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/gnome-object.h>
#include <bonobo/gnome-moniker.h>
#include <bonobo/gnome-container.h>

BEGIN_GNOME_DECLS
 
#define GNOME_CONTAINER_TYPE        (gnome_container_get_type ())
#define GNOME_CONTAINER(o)          (GTK_CHECK_CAST ((o), GNOME_CONTAINER_TYPE, GnomeContainer))
#define GNOME_CONTAINER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_CONTAINER_TYPE, GnomeContainerClass))
#define GNOME_IS_CONTAINER(o)       (GTK_CHECK_TYPE ((o), GNOME_CONTAINER_TYPE))
#define GNOME_IS_CONTAINER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_CONTAINER_TYPE))

typedef GList GnomeClientSiteList;

typedef struct _GnomeContainerPrivate GnomeContainerPrivate;

typedef struct {
	GnomeObject base;

	GnomeClientSiteList *client_sites;
	
	GnomeMoniker *moniker;

	GnomeContainerPrivate *priv;
} GnomeContainer;

typedef struct {
	GnomeObjectClass parent_class;

	GNOME_Unknown (*get_object) (GnomeContainer *item_container,
				     CORBA_char *item_name,
				     CORBA_boolean *only_if_exists,
				     CORBA_Environment *ev);
} GnomeContainerClass;

GtkType          gnome_container_get_type    (void);
GnomeContainer  *gnome_container_new         (void);
GnomeContainer  *gnome_container_construct   (GnomeContainer *container,
					      GNOME_Container container_corba);
GnomeMoniker    *gnome_container_get_moniker (GnomeContainer *container);

void             gnome_container_add         (GnomeContainer *container,
					      GnomeObject    *object);

void             gnome_container_remove       (GnomeContainer *container,
					       GnomeObject    *object);

POA_GNOME_Container__epv *gnome_container_get_epv (gboolean duplicate);

/*
 * Exported vectors
 */
extern POA_GNOME_Container__epv gnome_container_epv;
extern POA_GNOME_Container__vepv gnome_container_vepv;

END_GNOME_DECLS

#endif

