/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_LISTENER_H_
#define _BONOBO_LISTENER_H_

#include <bonobo/bonobo-arg.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

typedef struct _BonoboListener BonoboListener;

#define BONOBO_LISTENER_TYPE        (bonobo_listener_get_type ())
#define BONOBO_LISTENER(o)          (GTK_CHECK_CAST ((o), BONOBO_LISTENER_TYPE, BonoboListener))
#define BONOBO_LISTENER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_LISTENER_TYPE, BonoboListenerClass))
#define BONOBO_IS_LISTENER(o)       (GTK_CHECK_TYPE ((o), BONOBO_LISTENER_TYPE))
#define BONOBO_IS_LISTENER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_LISTENER_TYPE))


struct _BonoboListener {
        BonoboObject		object;
};

typedef struct {
	BonoboObjectClass parent_class;

	/* Signal definition */
	void (* event_notify) (BonoboListener *listener, BonoboArg *event_data);
} BonoboListenerClass;


GtkType bonobo_listener_get_type (void);

POA_Bonobo_Listener__epv *bonobo_listener_get_epv (void);

BonoboListener *bonobo_listener_new (void);


END_GNOME_DECLS

#endif /* _BONOBO_LISTENER_H_ */

