/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-listener.c: Generic listener interface for callbacks.
 *
 * Author:
 *	Mike Kestner (mkestner@ameritech.net)
 */
#include <config.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-listener.h>

static BonoboObjectClass *bonobo_listener_parent_class;

POA_Bonobo_Listener__vepv bonobo_listener_vepv;

#define CLASS(o) BONOBO_LISTENER_CLASS (GTK_OBJECT (o)->klass)

enum SIGNALS {
	EVENT_NOTIFY,
	LAST_SIGNAL
};
static guint signals [LAST_SIGNAL] = { 0 };

static inline BonoboListener *
bonobo_listener_from_servant (PortableServer_Servant servant)
{
	return BONOBO_LISTENER (bonobo_object_from_servant (servant));
}

static void
impl_event (PortableServer_Servant servant, const BonoboArg *data, CORBA_Environment *ev)
{
	BonoboListener *listener = bonobo_listener_from_servant (servant);

	gtk_signal_emit (GTK_OBJECT (listener), signals [EVENT_NOTIFY], data);
}

/**
 * bonobo_listener_get_epv:
 */
POA_Bonobo_Listener__epv *
bonobo_listener_get_epv (void)
{
	POA_Bonobo_Listener__epv *epv;

	epv = g_new0 (POA_Bonobo_Listener__epv, 1);

	epv->event = impl_event;

	return epv;
}

static void
init_listener_corba_class (void)
{
	/* The VEPV */
	bonobo_listener_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_listener_vepv.Bonobo_Listener_epv = bonobo_listener_get_epv ();
}

static void
bonobo_listener_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (object->klass)->destroy (object);
}

static void
bonobo_listener_init (GtkObject *object)
{
}

static void
bonobo_listener_class_init (BonoboListenerClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *)klass;

	bonobo_listener_parent_class = gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_listener_destroy;

	signals [EVENT_NOTIFY] = gtk_signal_new (
		"event_notify", GTK_RUN_LAST, oclass->type,
		GTK_SIGNAL_OFFSET (BonoboListenerClass, event_notify),
		gtk_marshal_NONE__POINTER, GTK_TYPE_NONE, 1,
		GTK_TYPE_POINTER);

	gtk_object_class_add_signals (oclass, signals, LAST_SIGNAL);

	init_listener_corba_class ();
}

/**
 * bonobo_listener_get_type:
 *
 * Returns: the GtkType for a BonoboListener.
 */
GtkType
bonobo_listener_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboListener",
			sizeof (BonoboListener),
			sizeof (BonoboListenerClass),
			(GtkClassInitFunc) bonobo_listener_class_init,
			(GtkObjectInitFunc) bonobo_listener_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

static Bonobo_Listener
bonobo_listener_corba_object_create (BonoboObject *object)
{
	POA_Bonobo_Listener *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Listener *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_listener_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Listener__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_listener_new:
 *
 * Creates a generic event listener.  The listener emits an "event_notify" signal when
 * notified of an event.  The signal callback should be of the form:
 *
 *	void some_callback (GtkObject *, BonoboArg *event_data, gpointer some_data);
 *
 * Returns: A BonoboListener object.
 */
BonoboListener*
bonobo_listener_new ()
{
	BonoboListener *listener;
	Bonobo_Listener corba_listener;

	listener = gtk_type_new (BONOBO_LISTENER_TYPE);

	corba_listener = bonobo_listener_corba_object_create (BONOBO_OBJECT (listener));
	if (corba_listener == CORBA_OBJECT_NIL){
		bonobo_object_unref (BONOBO_OBJECT (listener));
		return NULL;
	}
	
	bonobo_object_construct (BONOBO_OBJECT (listener), corba_listener);

	return listener;
}
