/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_TRANSIENT_H_
#define _BONOBO_TRANSIENT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>

BEGIN_GNOME_DECLS

#define BONOBO_TRANSIENT_TYPE        (bonobo_transient_get_type ())
#define BONOBO_TRANSIENT(o)          (GTK_CHECK_CAST ((o), BONOBO_TRANSIENT_TYPE, BonoboTransient))
#define BONOBO_TRANSIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_TRANSIENT_TYPE, BonoboTransientClass))
#define BONOBO_IS_TRANSIENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_TRANSIENT_TYPE))
#define BONOBO_IS_TRANSIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_TRANSIENT_TYPE))

typedef struct _BonoboTransientPriv BonoboTransientPriv;

typedef struct {
	GtkObject parent;

	PortableServer_POA  poa;
	BonoboTransientPriv *priv;
} BonoboTransient;

typedef struct {
	GtkObjectClass parent_class;
} BonoboTransientClass;

typedef PortableServer_Servant
       (*BonoboTransientServantNew) (PortableServer_POA, BonoboTransient *, char *name, gpointer *data);

typedef void (*BonoboTransientServantDestroy) (PortableServer_Servant servant, gpointer *data);

BonoboTransient *bonobo_transient_new (BonoboTransientServantNew     new_servant,
				       BonoboTransientServantDestroy destroy_servant,
				       gpointer data);

GtkType bonobo_transient_get_type (void);

END_GNOME_DECLS

#endif /* _BONOBO_TRANSIENT_H_ */
