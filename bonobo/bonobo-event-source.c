/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * bonobo-event-source.c: Generic event emitter.
 *
 * Copyright (C) 2000,  Helix Code, Inc.
 *
 * Author:
 *	Alex Graveley (alex@helixcode.com)
 */

#if 0 /* turn this off until we are compiling bonobo-listener.idl */

#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/bonobo-event-source.h>

static BonoboObjectClass    *bonobo_event_source_parent_class;
POA_Bonobo_EventSource__vepv bonobo_event_source_vepv;


struct _BonoboEventSourcePrivate {
	GSList *listeners;  /* CONTAINS: ListenerDesc* */
};

typedef struct {
	Bonobo_Listener listener;
	CORBA_char *event_mask; /* send all events if NULL */
} ListenerDesc;


static inline BonoboEventSource * 
bonobo_event_source_from_servant (PortableServer_Servant servant)
{
	return BONOBO_EVENT_SOURCE (bonobo_object_from_servant (servant));
}

static void
desc_free (ListenerDesc *desc, CORBA_Environment *ev)
{
	if (desc) {
		CORBA_free (desc->event_mask);
		bonobo_object_release_unref (desc->listener, ev);
		g_free (desc);
	}
}

static void
impl_Bonobo_EventSource_addListenerWithMask (PortableServer_Servant servant,
					     const Bonobo_Listener  l,
					     const CORBA_char      *event_mask,
					     CORBA_Environment     *ev)
{
	BonoboEventSource *event_source;
	GSList            *listeners;
	CORBA_char        *mask_copy = NULL;
	ListenerDesc      *desc;

	g_return_if_fail (!CORBA_Object_is_nil (l, ev));

	event_source = bonobo_event_source_from_servant (servant);
	listeners = event_source->priv->listeners;

	if (event_mask)
		mask_copy = CORBA_string_dup (event_mask);

	/* 
	 * Check if listener is already registered, and if so modify 
	 * event_mask to recieve masked events.
	 */
	while (listeners) {
		desc = (ListenerDesc *) listeners->data;

		if (CORBA_Object_is_equivalent (l, desc->listener, ev)) {
			CORBA_free (desc->event_mask);
			desc->event_mask = mask_copy;
			return;
		}

		listeners = listeners->next;
	}

	desc = g_new0 (ListenerDesc, 1);
	desc->listener = bonobo_object_dup_ref (l, ev);
	desc->event_mask = mask_copy;

	listeners = g_slist_prepend (listeners, desc);
}

static void
impl_Bonobo_EventSource_addListener (PortableServer_Servant servant,
				     const Bonobo_Listener  l,
				     CORBA_Environment     *ev)
{
	impl_Bonobo_EventSource_addListenerWithMask (servant, l, NULL, ev);
}

static void
impl_Bonobo_EventSource_removeListener (PortableServer_Servant servant,
					const Bonobo_Listener  l,
					CORBA_Environment     *ev)
{
	BonoboEventSource *event_source;
	GSList *listeners;

	g_return_if_fail (!CORBA_Object_is_nil (l, ev));

	event_source = bonobo_event_source_from_servant (servant);
	listeners = event_source->priv->listeners;

	while (listeners) {
		ListenerDesc *desc = (ListenerDesc *) listeners->data;

		if (CORBA_Object_is_equivalent (l, desc->listener, ev)) {
			event_source->priv->listeners = 
				g_slist_remove (event_source->priv->listeners,
						desc);
			desc_free (desc, ev);
			return;
		}

		listeners = listeners->next;
	}

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_EventSource_UnknownListener, 
			     NULL);
}

void
bonobo_event_source_notify_listeners (BonoboEventSource *event_source,
				      const char        *event_name,
				      CORBA_any         *value,
				      CORBA_Environment *opt_ev)
{
	GSList *listeners;
	CORBA_Environment ev, *my_ev;

	listeners = event_source->priv->listeners;

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	while (listeners) {
		ListenerDesc *desc = (ListenerDesc *) listeners->data;

		if (desc->event_mask == NULL || 
		    strstr (desc->event_mask, event_name))
			Bonobo_Listener_event (desc->listener, 
					       event_name, value, my_ev);

		listeners = listeners->next;
	}
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}


/**
 * bonobo_event_source_get_epv:
 */
POA_Bonobo_EventSource__epv *
bonobo_event_source_get_epv (void)
{
	POA_Bonobo_EventSource__epv *epv;

	epv = g_new0 (POA_Bonobo_EventSource__epv, 1);

	epv->addListener         = impl_Bonobo_EventSource_addListener;
	epv->addListenerWithMask = impl_Bonobo_EventSource_addListenerWithMask;
	epv->removeListener      = impl_Bonobo_EventSource_removeListener;

	return epv;
}

static void
init_event_source_corba_class (void)
{
	/* The VEPV */
	bonobo_event_source_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_event_source_vepv.Bonobo_EventSource_epv = bonobo_event_source_get_epv ();
}

static void
bonobo_event_source_destroy (GtkObject *object)
{
	BonoboEventSource *event_source;
	GSList            *l;
	CORBA_Environment  ev;
	
	event_source = BONOBO_EVENT_SOURCE (object);

	CORBA_exception_init (&ev);

	for (l = event_source->priv->listeners; l; l = l->next)
		desc_free (l->data, &ev);

	CORBA_exception_free (&ev);

	g_slist_free (event_source->priv->listeners);
	g_free (event_source->priv);

	GTK_OBJECT_CLASS (object->klass)->destroy (object);
}

static void
bonobo_event_source_init (GtkObject *object)
{
	BonoboEventSource *event_source;

	event_source = BONOBO_EVENT_SOURCE (object);
	event_source->priv = g_new (BonoboEventSourcePrivate, 1);
	event_source->priv->listeners = NULL;
}

static void
bonobo_event_source_class_init (BonoboEventSourceClass *klass)
{
	GtkObjectClass *oclass = (GtkObjectClass *) klass;

	bonobo_event_source_parent_class = 
		gtk_type_class (bonobo_object_get_type ());

	oclass->destroy = bonobo_event_source_destroy;

	init_event_source_corba_class ();
}

GtkType
bonobo_event_source_get_type (void)
{
        static GtkType type = 0;

        if (!type) {
                GtkTypeInfo info = {
                        "BonoboEventSource",
                        sizeof (BonoboEventSource),
                        sizeof (BonoboEventSourceClass),
                        (GtkClassInitFunc) bonobo_event_source_class_init,
                        (GtkObjectInitFunc) bonobo_event_source_init,
                        NULL, /* reserved 1 */
                        NULL, /* reserved 2 */
                        (GtkClassInitFunc) NULL
                };

                type = gtk_type_unique (bonobo_object_get_type (), &info);
        }

        return type;
}

Bonobo_EventSource
bonobo_event_source_corba_object_create (BonoboObject *object)
{
        POA_Bonobo_EventSource *servant;
        CORBA_Environment ev;

        servant = (POA_Bonobo_EventSource *) g_new0 (BonoboObjectServant, 1);
        servant->vepv = &bonobo_event_source_vepv;

        CORBA_exception_init (&ev);

        POA_Bonobo_EventSource__init ((PortableServer_Servant) servant, &ev);
        if (ev._major != CORBA_NO_EXCEPTION) {
                g_free (servant);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        CORBA_exception_free (&ev);

        return bonobo_object_activate_servant (object, servant);
}

BonoboEventSource *
bonobo_event_source_construct (BonoboEventSource  *event_source, 
			       Bonobo_EventSource corba_event_source) 
{
        g_return_val_if_fail (event_source != NULL, NULL);
        g_return_val_if_fail (BONOBO_IS_EVENT_SOURCE (event_source), NULL);
        g_return_val_if_fail (corba_event_source != NULL, NULL);

        bonobo_object_construct (BONOBO_OBJECT (event_source), 
				 corba_event_source);

        return event_source;
}

BonoboEventSource *
bonobo_event_source_new (void)
{
	BonoboEventSource *event_source;
	Bonobo_EventSource corba_event_source;
	
	event_source = gtk_type_new (BONOBO_EVENT_SOURCE_TYPE);
	corba_event_source = bonobo_event_source_corba_object_create (
		BONOBO_OBJECT (event_source));
	
	if (corba_event_source == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (event_source));
		return NULL;
	}
	
	return bonobo_event_source_construct (
		event_source, corba_event_source);
}

#endif
