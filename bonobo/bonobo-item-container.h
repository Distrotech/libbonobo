/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */

#ifndef _BONOBO_CONTAINER_H_
#define _BONOBO_CONTAINER_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-container.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_CONTAINER_TYPE        (bonobo_container_get_type ())
#define BONOBO_CONTAINER(o)          (GTK_CHECK_CAST ((o), BONOBO_CONTAINER_TYPE, BonoboContainer))
#define BONOBO_CONTAINER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_CONTAINER_TYPE, BonoboContainerClass))
#define BONOBO_IS_CONTAINER(o)       (GTK_CHECK_TYPE ((o), BONOBO_CONTAINER_TYPE))
#define BONOBO_IS_CONTAINER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_CONTAINER_TYPE))

typedef GList BonoboClientSiteList;

typedef struct _BonoboContainerPrivate BonoboContainerPrivate;

typedef struct {
	BonoboObject base;

	BonoboClientSiteList *client_sites;
	
	BonoboMoniker *moniker;

	BonoboContainerPrivate *priv;
} BonoboContainer;

typedef struct {
	BonoboObjectClass parent_class;

	Bonobo_Unknown (*get_object) (BonoboContainer   *item_container,
				      CORBA_char        *item_name,
				      CORBA_boolean      only_if_exists,
				      CORBA_Environment *ev);
} BonoboContainerClass;

GtkType          bonobo_container_get_type    (void);
BonoboContainer *bonobo_container_new         (void);
BonoboContainer *bonobo_container_construct   (BonoboContainer *container,
					       Bonobo_Container container_corba);
BonoboMoniker   *bonobo_container_get_moniker (BonoboContainer *container);

void             bonobo_container_add         (BonoboContainer *container,
					       BonoboObject    *object);
void             bonobo_container_remove      (BonoboContainer *container,
					       BonoboObject    *object);

POA_Bonobo_Container__epv *bonobo_container_get_epv (void);

END_GNOME_DECLS

#endif

