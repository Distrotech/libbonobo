/*
 * GNOME PersistStream
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/gnome-persist-stream.h>

/* Parent GTK object class */
static GnomePersistClass *gnome_persist_stream_parent_class;

/* The CORBA entry point vectors */
static POA_GNOME_Persist__epv gnome_persist_epv;
POA_GNOME_PersistStream__epv gnome_persist_stream_epv;

static CORBA_char *
impl_get_class_id (PortableServer_Servant servant, CORBA_Environment * ev)
{
	g_error ("Implement me");
	return CORBA_OBJECT_NIL;
}


static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeObject *object = gnome_object_from_servant (servant);
	GnomePersistStream *pstream = GNOME_PERSIST_STREAM (object);

	return pstream->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   const GNOME_Stream stream,
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
	   const GNOME_Stream stream,
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
	GtkObjectClass *oc = GTK_OBJECT (object)->klass;
	GnomePersistStreamClass *class = GNOME_PERSIST_STREAM_CLASS (oc);

	return ((*class->get_size_max)(GNOME_PERSIST_STREAM (object)));
}

static void
init_persist_stream_corba_class (void)
{
	/*
	 * The Entry Point Vectors for GNOME::Persist
	 * and GNOME::PersistStream
	 */
	gnome_persist_epv.get_class_id = impl_get_class_id;
	gnome_persist_stream_epv.load = impl_load;
	gnome_persist_stream_epv.save = impl_save;
	gnome_persist_stream_epv.get_size_max = impl_get_size_max;
}

static int
gnome_persist_stream_nop (GnomePersistStream *ps, const GNOME_Stream stream)
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
gnome_persist_stream_class_init (GnomePersistStreamClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_persist_stream_parent_class = gtk_type_class (gnome_persist_get_type ());

	/*
	 * Override and initialize methods
	 */

	class->save = gnome_persist_stream_nop;
	class->load = gnome_persist_stream_nop;
	class->get_size_max = gnome_persist_stream_zero;
	
	init_persist_stream_corba_class ();
}

static void
gnome_persist_stream_init (GnomePersistStream *ps)
{
}

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
 * @ps: A GnomerPersistStream
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the GnomePersistStream object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
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

	gnome_persist_construct (GNOME_PERSIST (ps), corba_ps);
	
	ps->load_fn = load_fn;
	ps->save_fn = save_fn;
	ps->closure = closure;

	return ps;
}

/**
 * gnome_persist_stream_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Creates a GnomePersistStream object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
GnomePersistStream *
gnome_persist_stream_new (GnomePersistStreamIOFn load_fn,
			  GnomePersistStreamIOFn save_fn,
			  void *closure)
{
	GnomePersistStream *ps;
	GNOME_PersistStream corba_ps;

	g_error ("This is an abstract class");
#if 0
	ps = gtk_type_new (gnome_persist_stream_get_type ());
	corba_ps = create_gnome_persist_stream (
		GNOME_OBJECT (ps));
	if (corba_ps == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (ps));
		return NULL;
	}

	gnome_persist_stream_construct (ps, load_fn, save_fn, closure);
#endif
	return ps;
}

