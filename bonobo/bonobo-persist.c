/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME Persist
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/bonobo-persist.h>

/* Parent GTK object class */
static BonoboObjectClass *bonobo_persist_parent_class;

/**
 * bonobo_persist_get_epv:
 */
POA_Bonobo_Persist__epv *
bonobo_persist_get_epv (void)
{
	POA_Bonobo_Persist__epv *epv;

	epv = g_new0 (POA_Bonobo_Persist__epv, 1);

	return epv;
}

static void
init_persist_corba_class (void)
{
}

static void
bonobo_persist_destroy (GtkObject *object)
{
	GTK_OBJECT_CLASS (bonobo_persist_parent_class)->destroy (object);
}

static void
bonobo_persist_class_init (BonoboPersistClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	bonobo_persist_parent_class = gtk_type_class (bonobo_object_get_type ());

	/*
	 * Override and initialize methods
	 */
	object_class->destroy = bonobo_persist_destroy;

	init_persist_corba_class ();
}

static void
bonobo_persist_init (BonoboPersist *persist)
{
}

BonoboPersist *
bonobo_persist_construct (BonoboPersist *persist,
			 Bonobo_Persist corba_persist)
{
	g_return_val_if_fail (persist != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PERSIST (persist), NULL);
	g_return_val_if_fail (corba_persist != CORBA_OBJECT_NIL, NULL);
	
	bonobo_object_construct (BONOBO_OBJECT (persist), corba_persist);

	return persist;
}

/**
 * bonobo_persist_get_type:
 *
 * Returns: the GtkType for the BonoboPersist class.
 */
GtkType
bonobo_persist_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Persist:1.0",
			sizeof (BonoboPersist),
			sizeof (BonoboPersistClass),
			(GtkClassInitFunc) bonobo_persist_class_init,
			(GtkObjectInitFunc) bonobo_persist_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}


