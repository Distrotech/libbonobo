/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * GNOME PersistStream implementation.  Can be used as a base class,
 * or directly for implementing objects that use PersistStream.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/gnome-persist-stream.h>

/* Parent GTK object class */
static GnomePersistClass *gnome_persist_stream_parent_class;

/* The CORBA entry point vectors */
POA_GNOME_PersistStream__vepv gnome_persist_stream_vepv;

static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomePersistStream *pstream = GNOME_PERSIST_STREAM (object);

	return pstream->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   GNOME_Stream stream,
	   CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomePersistStream *ps = GNOME_PERSIST_STREAM (object);
	int result;
	
	if (ps->load_fn != NULL)
		result = (*ps->load_fn)(ps, stream, ps->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ps)->klass;
		GnomePersistStreamClass *class = GNOME_PERSIST_STREAM_CLASS (oc);
		
		result = (*class->load)(ps, stream);
	}
	if (result != 0){
		g_warning ("FIXME: should report an exception\n");
	}
}

static void
impl_save (PortableServer_Servant servant,
	   GNOME_Stream stream,
	   CORBA_Environment *ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomePersistStream *ps = GNOME_PERSIST_STREAM (object);
	int result;
	
	if (ps->save_fn != NULL)
		result = (*ps->save_fn)(ps, stream, ps->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ps)->klass;
		GnomePersistStreamClass *class = GNOME_PERSIST_STREAM_CLASS (oc);
		
		result = (*class->save)(ps, stream);
	}
	
	if (result != 0){
		g_warning ("FIXME: should report an exception\n");
	}
	ps->is_dirty = FALSE;
}

static CORBA_long
impl_get_size_max (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomePersistStream *ps = GNOME_PERSIST_STREAM (object);
	GtkObjectClass *oc = GTK_OBJECT (object)->klass;
	GnomePersistStreamClass *class = GNOME_PERSIST_STREAM_CLASS (oc);


	if (ps->get_size_max_fn != NULL)
		return (*ps->get_size_max_fn)(ps, ps->closure);

	return (*class->get_size_max)(GNOME_PERSIST_STREAM (object));
}

/**
 * gnome_persist_stream_get_epv:
 *
 */
POA_GNOME_PersistStream__epv *
gnome_persist_stream_get_epv (void)
{
	POA_GNOME_PersistStream__epv *epv;

	epv = g_new0 (POA_GNOME_PersistStream__epv, 1);

	epv->load		= impl_load;
	epv->save		= impl_save;
	epv->get_size_max	= impl_get_size_max;
	epv->is_dirty		= impl_is_dirty;

	return epv;
}

static void
init_persist_stream_corba_class (void)
{
	gnome_persist_stream_vepv.GNOME_Unknown_epv = gnome_object_get_epv ();
	gnome_persist_stream_vepv.GNOME_Persist_epv = gnome_persist_get_epv ();
	gnome_persist_stream_vepv.GNOME_PersistStream_epv = gnome_persist_stream_get_epv ();
}

static int
gnome_persist_stream_nop (GnomePersistStream *ps, GNOME_Stream stream)
{
	/* Nothing: just return success */
	return 0;
}

static CORBA_long
gnome_persist_stream_zero (GnomePersistStream *ps)
{
	return 0;
}

static void
gnome_persist_stream_class_init (GnomePersistStreamClass *klass)
{
	gnome_persist_stream_parent_class = gtk_type_class (gnome_persist_get_type ());

	/*
	 * Override and initialize methods
	 */

	klass->save = gnome_persist_stream_nop;
	klass->load = gnome_persist_stream_nop;
	klass->get_size_max = gnome_persist_stream_zero;

	init_persist_stream_corba_class ();
}

static void
gnome_persist_stream_init (GnomePersistStream *ps)
{
}

/**
 * gnome_persist_stream_get_type:
 *
 * Returns: The GtkType for the GnomePersistStream class.
 */
GtkType
gnome_persist_stream_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/PersistStream:1.0",
			sizeof (GnomePersistStream),
			sizeof (GnomePersistStreamClass),
			(GtkClassInitFunc) gnome_persist_stream_class_init,
			(GtkObjectInitFunc) gnome_persist_stream_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_persist_get_type (), &info);
	}

	return type;
}

/**
 * gnome_persist_stream_construct:
 * @ps: A GnomerPersistStream object
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the GnomePersistStream object.  The load and save
 * operations for the object are performed by the provided @load_fn
 * and @save_fn callback functions, which are passed @closure when
 * they are invoked.  If either @load_fn or @save_fn is %NULL, the
 * corresponding operation is performed by the class load and save
 * routines.
 *
 * Returns: The initialized GnomePersistStream object.
 */
GnomePersistStream *
gnome_persist_stream_construct (GnomePersistStream *ps,
				GNOME_PersistStream corba_ps,
				GnomePersistStreamIOFn load_fn,
				GnomePersistStreamIOFn save_fn,
				void *closure)
{
	g_return_val_if_fail (ps != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PERSIST_STREAM (ps), NULL);
	g_return_val_if_fail (corba_ps != CORBA_OBJECT_NIL, NULL);

	gnome_persist_construct (GNOME_PERSIST (ps), corba_ps);
	
	ps->load_fn = load_fn;
	ps->save_fn = save_fn;
	ps->closure = closure;
	
	return ps;
}

static GNOME_PersistStream
create_gnome_persist_stream (GnomeObject *object)
{
	POA_GNOME_PersistStream *servant;
	CORBA_Environment ev;

	servant = (POA_GNOME_PersistStream *) g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_persist_stream_vepv;

	CORBA_exception_init (&ev);

	POA_GNOME_PersistStream__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return (GNOME_PersistStream) gnome_object_activate_servant (object, servant);
}


/**
 * gnome_persist_stream_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Creates a new GnomePersistStream object.  The load and save
 * operations for the object are performed by the provided @load_fn
 * and @save_fn callback functions, which are passed @closure when
 * they are invoked.  If either @load_fn or @save_fn is %NULL, the
 * corresponding operation is performed by the class load and save
 * routines.
 *
 * Returns: the newly-created GnomePersistStream object.
 */
GnomePersistStream *
gnome_persist_stream_new (GnomePersistStreamIOFn load_fn,
			  GnomePersistStreamIOFn save_fn,
			  void *closure)
{
	GnomePersistStream *ps;
	GNOME_PersistStream corba_ps;

	ps = gtk_type_new (gnome_persist_stream_get_type ());
	corba_ps = create_gnome_persist_stream (
		GNOME_OBJECT (ps));
	if (corba_ps == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (ps));
		return NULL;
	}

	gnome_persist_stream_construct (ps, corba_ps, load_fn, save_fn, closure);

	return ps;
}

/**
 * gnome_persist_stream_set_dirty:
 * @ps: A GnomerPersistStream object
 * @dirty: A boolean value representing whether the object is dirty or not
 *
 * This routine sets the dirty bit for the PersistStream object.
 */
void
gnome_persist_stream_set_dirty (GnomePersistStream *pstream, gboolean dirty)
{
	g_return_if_fail (pstream != NULL);
	g_return_if_fail (GNOME_IS_PERSIST_STREAM (pstream));

	pstream->is_dirty = dirty;
}
