/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_LISTENER_H_
#define _BONOBO_LISTENER_H_

#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_LISTENER_TYPE        (bonobo_listener_get_type ())
#define BONOBO_LISTENER(o)          (GTK_CHECK_CAST ((o), BONOBO_LISTENER_TYPE, BonoboListener))
#define BONOBO_LISTENER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_LISTENER_TYPE, BonoboListenerClass))
#define BONOBO_IS_LISTENER(o)       (GTK_CHECK_TYPE ((o), BONOBO_LISTENER_TYPE))
#define BONOBO_IS_LISTENER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_LISTENER_TYPE))

typedef struct _BonoboListener        BonoboListener;
typedef struct _BonoboListenerPrivate BonoboListenerPrivate;

struct _BonoboListener {
        BonoboObject	       parent;

	BonoboListenerPrivate *priv;
};

typedef struct {
	BonoboObjectClass parent_class;

	/* Signals */
	void (* event_notify) (BonoboListener    *listener, 
			       char              *event_name,
			       BonoboArg         *event_data, 
			       CORBA_Environment *ev);
} BonoboListenerClass;


typedef void (*BonoboListenerCallbackFn)  (BonoboListener    *listener,
					   char              *event_name, 
					   CORBA_any         *any,
					   CORBA_Environment *ev,
					   gpointer           user_data);

GtkType         bonobo_listener_get_type  (void);

BonoboListener *bonobo_listener_new       (BonoboListenerCallbackFn event_callback, 
					   gpointer                 user_data);

BonoboListener *bonobo_listener_construct (BonoboListener          *listener, 
					   Bonobo_Listener          corba_listener);

POA_Bonobo_Listener__epv *bonobo_listener_get_epv   (void);
Bonobo_Listener bonobo_listener_corba_object_create (BonoboObject *object);


END_GNOME_DECLS

#endif /* _BONOBO_LISTENER_H_ */

