/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME::SimpleDataSource
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999 International GNOME Support (http://www.gnome-support.com)
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/gnome-simple-source.h>

/* Parent GTK object class */
static GnomeObjectClass *gnome_simple_data_source_parent_class;

POA_GNOME_SimpleDataSource__epv gnome_simple_data_source_epv;
POA_GNOME_SimpleDataSource__vepv gnome_simple_data_source_vepv;

static void
impl_pop_data (PortableServer_Servant servant,
	       const CORBA_long count,
	       GNOME_SimpleDataSource_iobuf **buffer,	       
	       CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeSimpleDataSource *ssource = GNOME_SIMPLE_DATA_SOURCE (object);
	int result;

	if (ssource->pop_data_fn != NULL)
		result = (*ssource->pop_data_fn) (ssource, count, buffer, ssource->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (ssource)->klass;
		GnomeSimpleDataSourceClass *class = GNOME_SIMPLE_DATA_SOURCE_CLASS (oc);

		result = (*class->pop_data_fn) (ssource, count, buffer);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception\n");
	}
	
} /* impl_pop_data */

static CORBA_long
impl_remaining_data (PortableServer_Servant servant,
		     CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeSimpleDataSource *ssource = GNOME_SIMPLE_DATA_SOURCE (object);

	if (ssource->remaining_data_fn != NULL)
		return (*ssource->remaining_data_fn)(ssource, ssource->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ssource)->klass;
		GnomeSimpleDataSourceClass *class = GNOME_SIMPLE_DATA_SOURCE_CLASS (oc);

		return (*class->remaining_data_fn)(GNOME_SIMPLE_DATA_SOURCE (object));
	}

} /* impl_remaining_data */

static void
init_simple_data_source_corba_class (void)
{
	/*
	 * Initialize the entry point vector for GNOME::SimpleDataSourcea
	 */
	gnome_simple_data_source_epv.pop_data = impl_pop_data;
	gnome_simple_data_source_epv.remaining_data = impl_remaining_data;

	gnome_simple_data_source_vepv.GNOME_Unknown_epv = &gnome_object_epv;
	gnome_simple_data_source_vepv.GNOME_SimpleDataSource_epv =
		&gnome_simple_data_source_epv;
	
} /* init_simple_data_source_corba_class */

static void
gnome_simple_data_source_destroy (GtkObject *object)
{
} /* gnome_simple_data_source_destroy */

static int
gnome_simple_data_source_pop_data_nop (GnomeSimpleDataSource *ssource,
				       const CORBA_long count,
				       GNOME_SimpleDataSource_iobuf **buffer)
{
	return 0;
} /* gnome_simple_data_source_pop_data_nop */

static CORBA_long
gnome_simple_data_source_remaining_data_zero (GnomeSimpleDataSource *ssource)
{
	return (CORBA_long) 0;
} /* gnome_simple_data_source_remaining_data_zero */

static void
gnome_simple_data_source_class_init (GnomeSimpleDataSourceClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_simple_data_source_parent_class =
		gtk_type_class (gnome_object_get_type ());

	/*
	 * Override and initialize methods
	 */
	object_class->destroy = gnome_simple_data_source_destroy;

	class->pop_data_fn = gnome_simple_data_source_pop_data_nop;
	class->remaining_data_fn = gnome_simple_data_source_remaining_data_zero;

	init_simple_data_source_corba_class ();
} /* gnome_simple_data_source_class_init */

static void
gnome_simple_data_source_init (GnomeSimpleDataSource *ssource)
{
} /* gnome_progressive_data_sink_init */

GtkType
gnome_simple_data_source_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/SimpleDataSource:1.0",
			sizeof (GnomeSimpleDataSource),
			sizeof (GnomeSimpleDataSourceClass),
			(GtkClassInitFunc) gnome_simple_data_source_class_init,
			(GtkObjectInitFunc) gnome_simple_data_source_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
} /* gnome_simple_data_source_get_type */

GnomeSimpleDataSource *
gnome_simple_data_source_construct (GnomeSimpleDataSource *ssource,
				    GNOME_SimpleDataSource corba_ssource,
				    GnomeSimpleDataSourcePopDataFn pop_data_fn,
				    GnomeSimpleDataSourceRemainingDataFn remaining_data_fn,
				    void *closure)
{
	g_return_val_if_fail (ssource != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_SIMPLE_DATA_SOURCE (ssource), NULL);
	g_return_val_if_fail (corba_ssource != CORBA_OBJECT_NIL, NULL);

	gnome_object_construct (GNOME_OBJECT (ssource), corba_ssource);

	ssource->pop_data_fn = pop_data_fn;
	ssource->remaining_data_fn = remaining_data_fn;

	ssource->closure = closure;

	return ssource;
} /* gnome_simple_data_source_construct */

static GNOME_SimpleDataSource
create_gnome_simple_data_source (GnomeObject *object)
{
	POA_GNOME_SimpleDataSource *servant;
	CORBA_Object o;

	servant = (POA_GNOME_SimpleDataSource *) g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_simple_data_source_vepv;
	POA_GNOME_SimpleDataSource__init ((PortableServer_Servant) servant, &object->ev);
	if (object->ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	return (GNOME_SimpleDataSource) gnome_object_activate_servant (object, servant);
} /* create_gnome_simple_data_source */

GnomeSimpleDataSource *
gnome_simple_data_source_new (GnomeSimpleDataSourcePopDataFn pop_data_fn,
			      GnomeSimpleDataSourceRemainingDataFn remaining_data_fn,
			      void *closure)
{
	GnomeSimpleDataSource *ssource;
	GNOME_SimpleDataSource corba_ssource;

	ssource = gtk_type_new (gnome_simple_data_source_get_type ());
	corba_ssource = create_gnome_simple_data_source (GNOME_OBJECT (ssource));
	if (corba_ssource == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (ssource));
		return NULL;
	}

	gnome_simple_data_source_construct (ssource, corba_ssource,
					    pop_data_fn,
					    remaining_data_fn,
					    closure);

	return ssource;
} /* gnome_simple_data_source_new */

