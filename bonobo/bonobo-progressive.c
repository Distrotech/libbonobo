/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * GNOME::ProgressiveDataSink
 *
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999 International GNOME Support (http://www.gnome-support.com)
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/gnome-progressive.h>

/* Parent GTK object class */
static GnomeObjectClass *gnome_progressive_data_sink_parent_class;

POA_GNOME_ProgressiveDataSink__epv gnome_progressive_data_sink_epv;
POA_GNOME_ProgressiveDataSink__vepv gnome_progressive_data_sink_vepv;

static void
impl_start (PortableServer_Servant servant,
	    CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeProgressiveDataSink *psink = GNOME_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->start_fn != NULL)
		result = (*psink->start_fn) (psink, psink->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		GnomeProgressiveDataSinkClass *class = GNOME_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->start_fn) (psink);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception\n");
	}
	
} /* impl_start */

static void
impl_end (PortableServer_Servant servant,
	  CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeProgressiveDataSink *psink = GNOME_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->end_fn != NULL)
		result = (*psink->end_fn) (psink, psink->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		GnomeProgressiveDataSinkClass *class = GNOME_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->end_fn) (psink);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception\n");
	}
	
} /* impl_end */

static void
impl_add_data (PortableServer_Servant servant,
	       const GNOME_ProgressiveDataSink_iobuf *buffer,
	       CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeProgressiveDataSink *psink = GNOME_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->add_data_fn != NULL)
		result = (*psink->add_data_fn) (psink,
						buffer,
						psink->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		GnomeProgressiveDataSinkClass *class =
			GNOME_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->add_data_fn) (psink, buffer);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception\n");
	}
	
} /* impl_add_data */

static void
impl_set_size (PortableServer_Servant servant,
	       CORBA_long count,
	       CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomeProgressiveDataSink *psink = GNOME_PROGRESSIVE_DATA_SINK (object);
	int result;

	if (psink->set_size_fn != NULL)
		result = (*psink->set_size_fn) (psink,
						count,
						psink->closure);
	else
	{
		GtkObjectClass *oc = GTK_OBJECT (psink)->klass;
		GnomeProgressiveDataSinkClass *class =
			GNOME_PROGRESSIVE_DATA_SINK_CLASS (oc);

		result = (*class->set_size_fn) (psink, count);
	}

	if (result != 0)
	{
		g_warning ("FIXME: should report an exception\n");
	}
	
} /* impl_set_size */

static void
init_progressive_data_sink_corba_class (void)
{
	/*
	 * Initialize the entry point vector for GNOME::ProgressiveDataSink.
	 */
	gnome_progressive_data_sink_epv.start = impl_start;
	gnome_progressive_data_sink_epv.end = impl_end;
	gnome_progressive_data_sink_epv.add_data = impl_add_data;
	gnome_progressive_data_sink_epv.set_size = impl_set_size;

	gnome_progressive_data_sink_vepv.GNOME_Unknown_epv = &gnome_object_epv;
	gnome_progressive_data_sink_vepv.GNOME_ProgressiveDataSink_epv =
		&gnome_progressive_data_sink_epv;
	
}

static void
gnome_progressive_data_sink_destroy (GtkObject *object)
{
}

static int
gnome_progressive_data_sink_start_end_nop (GnomeProgressiveDataSink *psink)
{
	return 0;
}

static int
gnome_progressive_data_sink_add_data_nop (GnomeProgressiveDataSink *psink,
					  const GNOME_ProgressiveDataSink_iobuf *buffer)
{
	return 0;
}

static int
gnome_progressive_data_sink_set_size_nop (GnomeProgressiveDataSink *psink,
					  const CORBA_long count)
{
	return 0;
}

static void
gnome_progressive_data_sink_class_init (GnomeProgressiveDataSinkClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_progressive_data_sink_parent_class =
		gtk_type_class (gnome_object_get_type ());

	/*
	 * Override and initialize methods
	 */
	object_class->destroy = gnome_progressive_data_sink_destroy;

	class->start_fn = gnome_progressive_data_sink_start_end_nop;
	class->end_fn = gnome_progressive_data_sink_start_end_nop;
	class->add_data_fn = gnome_progressive_data_sink_add_data_nop;
	class->set_size_fn = gnome_progressive_data_sink_set_size_nop;

	init_progressive_data_sink_corba_class ();
}

static void
gnome_progressive_data_sink_init (GnomeProgressiveDataSink *psink)
{
}

/**
 * gnome_progressive_data_sink_get_type:
 *
 * Returns: The GtkType for the GnomeProgressiveDataSink class type.
 */
GtkType
gnome_progressive_data_sink_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/ProgressiveDataSink:1.0",
			sizeof (GnomeProgressiveDataSink),
			sizeof (GnomeProgressiveDataSinkClass),
			(GtkClassInitFunc) gnome_progressive_data_sink_class_init,
			(GtkObjectInitFunc) gnome_progressive_data_sink_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
} 

/**
 * gnome_progressive_data_sink_construct:
 * @psink: The #GnomeProgressiveDataSink object to initialize.
 * @corba_psink: The CORBA object for the #GNOME_ProgressiveDataSink interface.
 * @start_fn: A callback which is invoked when the #start method is
 * called on the interface.  The #start method is used to indicate that
 * a new set of data is being loaded into the #GNOME_ProgressiveDataSink interface.
 * @end_fn: A callback which is invoked when the #end method is called
 * on the interface.  The #end method is called after the entire data transmission
 * is complete.
 * @add_data_fn: A callback which is invoked when the #add_data method is called
 * on the interface.  This method is called whenever new data is available for
 * the current transmission.  The new data is passed as an argument to the method
 * @set_size_fn: A callback which is invoked when the #set_size method is called
 * on the interface.  The #set_size method is used by the caller to specify
 * the total size of the current transmission.  This method may be used
 * at any point after #start has been called.  Objects implementing #GNOME_ProgressiveDataSink
 * may want to use this to know what percentage of the data has been received.
 * @closure: A closure which pis passed to all the callback functions.
 *
 * This function initializes @psink with the CORBA interface
 * provided in @corba_psink and the callback functions
 * specified by @start_fn, @end_fn, @add_data_fn and @set_size_fn.
 * 
 * Returns: the initialized GnomeProgressiveDataSink object.
 */
GnomeProgressiveDataSink *
gnome_progressive_data_sink_construct (GnomeProgressiveDataSink *psink,
				       GNOME_ProgressiveDataSink corba_psink,
				       GnomeProgressiveDataSinkStartFn start_fn,
				       GnomeProgressiveDataSinkEndFn end_fn,
				       GnomeProgressiveDataSinkAddDataFn add_data_fn,
				       GnomeProgressiveDataSinkSetSizeFn set_size_fn,
				       void *closure)
{
	g_return_val_if_fail (psink != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROGRESSIVE_DATA_SINK (psink), NULL);
	g_return_val_if_fail (corba_psink != CORBA_OBJECT_NIL, NULL);

	gnome_object_construct (GNOME_OBJECT (psink), corba_psink);

	psink->start_fn = start_fn;
	psink->end_fn = end_fn;
	psink->add_data_fn = add_data_fn;
	psink->set_size_fn = set_size_fn;

	psink->closure = closure;

	return psink;
} 

static GNOME_ProgressiveDataSink
create_gnome_progressive_data_sink (GnomeObject *object)
{
	POA_GNOME_ProgressiveDataSink *servant;

	servant = (POA_GNOME_ProgressiveDataSink *)g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_progressive_data_sink_vepv;
	POA_GNOME_ProgressiveDataSink__init ((PortableServer_Servant) servant, &object->ev);
	if (object->ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	return (GNOME_ProgressiveDataSink) gnome_object_activate_servant (object, servant);
} 

/**
 * gnome_progressive_data_sink_new:
 * @start_fn: A callback which is invoked when the #start method is
 * called on the interface.  The #start method is used to indicate that
 * a new set of data is being loaded into the #GNOME_ProgressiveDataSink interface.
 * @end_fn: A callback which is invoked when the #end method is called
 * on the interface.  The #end method is called after the entire data transmission
 * is complete.
 * @add_data_fn: A callback which is invoked when the #add_data method is called
 * on the interface.  This method is called whenever new data is available for
 * the current transmission.  The new data is passed as an argument to the method
 * @set_size_fn: A callback which is invoked when the #set_size method is called
 * on the interface.  The #set_size method is used by the caller to specify
 * the total size of the current transmission.  This method may be used
 * at any point after #start has been called.  Objects implementing #GNOME_ProgressiveDataSink
 * may want to use this to know what percentage of the data has been received.
 * @closure: A closure which pis passed to all the callback functions.
 *
 * This function creates a new GnomeProgressiveDataSink object and the
 * corresponding CORBA interface object.  The new object is
 * initialized with the callback functionss specified by @start_fn,
 * @end_fn, @add_data_fn and @set_size_fn.
 * 
 * Returns: the newly-constructed GnomeProgressiveDataSink object.
 */
GnomeProgressiveDataSink *
gnome_progressive_data_sink_new (GnomeProgressiveDataSinkStartFn start_fn,
				 GnomeProgressiveDataSinkEndFn end_fn,
				 GnomeProgressiveDataSinkAddDataFn add_data_fn,
				 GnomeProgressiveDataSinkSetSizeFn set_size_fn,
				 void *closure)
{
	GnomeProgressiveDataSink *psink;
	GNOME_ProgressiveDataSink corba_psink;

	psink = gtk_type_new (gnome_progressive_data_sink_get_type ());
	corba_psink = create_gnome_progressive_data_sink (GNOME_OBJECT (psink));
	if (corba_psink == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (psink));
		return NULL;
	}

	gnome_progressive_data_sink_construct (psink, corba_psink, start_fn,
					       end_fn, add_data_fn,
					       set_size_fn, closure);

	return psink;
} 
