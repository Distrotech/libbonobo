/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_OBJECT_CLIENT_H_
#define _BONOBO_OBJECT_CLIENT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>

#define BONOBO_OBJECT_CLIENT_TYPE        (bonobo_object_client_get_type ())
#define BONOBO_OBJECT_CLIENT(o)          (GTK_CHECK_CAST ((o), BONOBO_OBJECT_CLIENT_TYPE, BonoboObjectClient))
#define BONOBO_OBJECT_CLIENT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_OBJECT_CLIENT_TYPE, BonoboObjectClientClass))
#define BONOBO_IS_OBJECT_CLIENT(o)       (GTK_CHECK_TYPE ((o), BONOBO_OBJECT_CLIENT_TYPE))
#define BONOBO_IS_OBJECT_CLIENT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_OBJECT_CLIENT_TYPE))

typedef struct {
	BonoboObject parent;
} BonoboObjectClient;

typedef struct {
	BonoboObjectClass parent_class;
} BonoboObjectClientClass;

GtkType             bonobo_object_client_get_type        (void);

BonoboObjectClient *bonobo_object_client_construct       (BonoboObjectClient *object_client,
							  CORBA_Object corba_object);

BonoboObjectClient *bonobo_object_activate               (const char *object_desc,
							  gint oaf_or_goad_flags);
BonoboObjectClient *bonobo_object_activate_with_goad_id  (gpointer dummy,
							  const char *goad_id,
							  gint        goad_flags,
							  const char **params);
BonoboObjectClient *bonobo_object_activate_with_oaf_id   (const char *oaf_id,
							  gint flags);
Bonobo_Unknown      bonobo_object_restore_from_url       (const char *goad_id,
							  const char *url);
BonoboObjectClient *bonobo_object_client_from_corba      (Bonobo_Unknown o);

/* Convenience Bonobo_Unknown wrappers */
gboolean           bonobo_object_client_has_interface    (BonoboObjectClient *object,
							  const char *interface_desc,
							  CORBA_Environment *opt_ev);
Bonobo_Unknown      bonobo_object_client_query_interface (BonoboObjectClient *object,
							  const char *interface_desc,
							  CORBA_Environment *opt_ev);

#endif /* _BONOBO_OBJECT_CLIENT_H_ */

