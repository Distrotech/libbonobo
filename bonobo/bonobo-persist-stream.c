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
#include <bonobo/bonobo-persist-stream.h>

/* Parent GTK object class */
static BonoboPersistClass *bonobo_persist_stream_parent_class;

/* The CORBA entry point vectors */
POA_Bonobo_PersistStream__vepv bonobo_persist_stream_vepv;

static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *pstream = BONOBO_PERSIST_STREAM (object);

	return pstream->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   Bonobo_Stream stream,
	   CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (object);
	int result;
	
	if (ps->load_fn != NULL)
		result = (*ps->load_fn)(ps, stream, ps->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ps)->klass;
		BonoboPersistStreamClass *class = BONOBO_PERSIST_STREAM_CLASS (oc);
		
		result = (*class->load)(ps, stream);
	}
	if (result != 0){
		g_warning ("FIXME: should report an exception\n");
	}
}

static void
impl_save (PortableServer_Servant servant,
	   Bonobo_Stream stream,
	   CORBA_Environment *ev)
{
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (object);
	int result;
	
	if (ps->save_fn != NULL)
		result = (*ps->save_fn)(ps, stream, ps->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (ps)->klass;
		BonoboPersistStreamClass *class = BONOBO_PERSIST_STREAM_CLASS (oc);
		
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
	BonoboObject *object = bonobo_object_from_servant (servant);
	BonoboPersistStream *ps = BONOBO_PERSIST_STREAM (object);
	GtkObjectClass *oc = GTK_OBJECT (object)->klass;
	BonoboPersistStreamClass *class = BONOBO_PERSIST_STREAM_CLASS (oc);


	if (ps->get_size_max_fn != NULL)
		return (*ps->get_size_max_fn)(ps, ps->closure);

	return (*class->get_size_max)(BONOBO_PERSIST_STREAM (object));
}

/**
 * bonobo_persist_stream_get_epv:
 *
 */
POA_Bonobo_PersistStream__epv *
bonobo_persist_stream_get_epv (void)
{
	POA_Bonobo_PersistStream__epv *epv;

	epv = g_new0 (POA_Bonobo_PersistStream__epv, 1);

	epv->load		= impl_load;
	epv->save		= impl_save;
	epv->get_size_max	= impl_get_size_max;
	epv->is_dirty		= impl_is_dirty;

	return epv;
}

static void
init_persist_stream_corba_class (void)
{
	bonobo_persist_stream_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_persist_stream_vepv.Bonobo_Persist_epv = bonobo_persist_get_epv ();
	bonobo_persist_stream_vepv.Bonobo_PersistStream_epv = bonobo_persist_stream_get_epv ();
}

static int
bonobo_persist_stream_nop (BonoboPersistStream *ps, Bonobo_Stream stream)
{
	/* Nothing: just return success */
	return 0;
}

static CORBA_long
bonobo_persist_stream_zero (BonoboPersistStream *ps)
{
	return 0;
}

static void
bonobo_persist_stream_class_init (BonoboPersistStreamClass *klass)
{
	bonobo_persist_stream_parent_class = gtk_type_class (bonobo_persist_get_type ());

	/*
	 * Override and initialize methods
	 */

	klass->save = bonobo_persist_stream_nop;
	klass->load = bonobo_persist_stream_nop;
	klass->get_size_max = bonobo_persist_stream_zero;

	init_persist_stream_corba_class ();
}

static void
bonobo_persist_stream_init (BonoboPersistStream *ps)
{
}

/**
 * bonobo_persist_stream_get_type:
 *
 * Returns: The GtkType for the BonoboPersistStream class.
 */
GtkType
bonobo_persist_stream_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/PersistStream:1.0",
			sizeof (BonoboPersistStream),
			sizeof (BonoboPersistStreamClass),
			(GtkClassInitFunc) bonobo_persist_stream_class_init,
			(GtkObjectInitFunc) bonobo_persist_stream_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_persist_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_persist_stream_construct:
 * @ps: A GnomerPersistStream object
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the BonoboPersistStream object.  The load and save
 * operations for the object are performed by the provided @load_fn
 * and @save_fn callback functions, which are passed @closure when
 * they are invoked.  If either @load_fn or @save_fn is %NULL, the
 * corresponding operation is performed by the class load and save
 * routines.
 *
 * Returns: The initialized BonoboPersistStream object.
 */
BonoboPersistStream *
bonobo_persist_stream_construct (BonoboPersistStream *ps,
				Bonobo_PersistStream corba_ps,
				BonoboPersistStreamIOFn load_fn,
				BonoboPersistStreamIOFn save_fn,
				void *closure)
{
	g_return_val_if_fail (ps != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PERSIST_STREAM (ps), NULL);
	g_return_val_if_fail (corba_ps != CORBA_OBJECT_NIL, NULL);

	bonobo_persist_construct (BONOBO_PERSIST (ps), corba_ps);
	
	ps->load_fn = load_fn;
	ps->save_fn = save_fn;
	ps->closure = closure;
	
	return ps;
}

static Bonobo_PersistStream
create_bonobo_persist_stream (BonoboObject *object)
{
	POA_Bonobo_PersistStream *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_PersistStream *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_persist_stream_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_PersistStream__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return (Bonobo_PersistStream) bonobo_object_activate_servant (object, servant);
}


/**
 * bonobo_persist_stream_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Creates a new BonoboPersistStream object.  The load and save
 * operations for the object are performed by the provided @load_fn
 * and @save_fn callback functions, which are passed @closure when
 * they are invoked.  If either @load_fn or @save_fn is %NULL, the
 * corresponding operation is performed by the class load and save
 * routines.
 *
 * Returns: the newly-created BonoboPersistStream object.
 */
BonoboPersistStream *
bonobo_persist_stream_new (BonoboPersistStreamIOFn load_fn,
			  BonoboPersistStreamIOFn save_fn,
			  void *closure)
{
	BonoboPersistStream *ps;
	Bonobo_PersistStream corba_ps;

	ps = gtk_type_new (bonobo_persist_stream_get_type ());
	corba_ps = create_bonobo_persist_stream (
		BONOBO_OBJECT (ps));
	if (corba_ps == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (ps));
		return NULL;
	}

	bonobo_persist_stream_construct (ps, corba_ps, load_fn, save_fn, closure);

	return ps;
}

/**
 * bonobo_persist_stream_set_dirty:
 * @ps: A GnomerPersistStream object
 * @dirty: A boolean value representing whether the object is dirty or not
 *
 * This routine sets the dirty bit for the PersistStream object.
 */
void
bonobo_persist_stream_set_dirty (BonoboPersistStream *pstream, gboolean dirty)
{
	g_return_if_fail (pstream != NULL);
	g_return_if_fail (BONOBO_IS_PERSIST_STREAM (pstream));

	pstream->is_dirty = dirty;
}
