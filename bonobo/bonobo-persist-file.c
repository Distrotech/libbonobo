/*
 * GNOME PersistFile
 *
 * Author:
 *   Matt Loper (matt@gnome-support.com)
 */

#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/gnome-persist-file.h>

/* Parent GTK object class */
static GnomePersistClass *gnome_persist_file_parent_class;

/* The CORBA entry point vectors */
static POA_GNOME_Persist__epv gnome_persist_epv;
POA_GNOME_PersistFile__epv gnome_persist_file_epv;
POA_GNOME_PersistFile__vepv gnome_persist_file_vepv;

static CORBA_char *
impl_get_current_file (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeUnknown *object = gnome_unknown_from_servant (servant);
	GnomePersistFile *pfile = GNOME_PERSIST_FILE (object);

	/* if our persist_file has a filename with any length, return it */
	if (pfile->filename && strlen(pfile->filename))
		return CORBA_string_dup ((CORBA_char*)pfile->filename);
	else
	{
		/* otherwise, raise a `NoCurrentName' exception */
		GNOME_PersistFile_NoCurrentName *exception;
		exception = g_new (GNOME_PersistFile_NoCurrentName, 1);
		if (exception == NULL) {
			CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY,
						    CORBA_COMPLETED_NO);

			return NULL;
		}
		
		exception->extension = "";
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_PersistFile_NoCurrentName,
				     exception);
		return NULL;
	}	
}


static CORBA_char *
impl_get_class_id (PortableServer_Servant servant, CORBA_Environment * ev)
{
	g_error ("Implement me");
	return CORBA_OBJECT_NIL;
}


static CORBA_boolean
impl_is_dirty (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeUnknown *object = gnome_unknown_from_servant (servant);
	GnomePersistFile *pfile = GNOME_PERSIST_FILE (object);

	return pfile->is_dirty;
}

static void
impl_load (PortableServer_Servant servant,
	   const CORBA_char *filename,
	   CORBA_Environment *ev)
{
	GnomeUnknown *object = gnome_unknown_from_servant (servant);
	GnomePersistFile *pf = GNOME_PERSIST_FILE (object);
	int result;
	
	if (pf->load_fn != NULL)
		result = (*pf->load_fn)(pf, filename, pf->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (pf)->klass;
		GnomePersistFileClass *class = GNOME_PERSIST_FILE_CLASS (oc);
		
		result = (*class->load)(pf, filename);
	}
	if (result != 0){
		g_warning ("FIXME: should report an exception\n");
	}
}

static void
impl_save (PortableServer_Servant servant,
	   const CORBA_char *filename,
	   CORBA_Environment *ev)
{
	GnomeUnknown *object = gnome_unknown_from_servant (servant);
	GnomePersistFile *pf = GNOME_PERSIST_FILE (object);
	int result;
	
	if (pf->save_fn != NULL)
		result = (*pf->save_fn)(pf, filename, pf->closure);
	else {
		GtkObjectClass *oc = GTK_OBJECT (pf)->klass;
		GnomePersistFileClass *class = GNOME_PERSIST_FILE_CLASS (oc);
		
		result = (*class->save)(pf, filename);
	}
	
	if (result != 0){
		g_warning ("FIXME: should report an exception\n");
	}
	pf->is_dirty = FALSE;
}

static void
init_persist_file_corba_class (void)
{
	/*
	 * The Entry Point Vectors for GNOME::Persist
	 * and GNOME::PersistFile
	 */
	gnome_persist_epv.get_class_id = impl_get_class_id;
	gnome_persist_file_epv.load = impl_load;
	gnome_persist_file_epv.save = impl_save;
	gnome_persist_file_epv.get_current_file = impl_get_current_file;

	gnome_persist_file_vepv.GNOME_Unknown_epv = &gnome_unknown_epv;
	gnome_persist_file_vepv.GNOME_PersistFile_epv = &gnome_persist_file_epv;
}

static int
gnome_persist_file_nop (GnomePersistFile *pf, const CORBA_char *filename)
{
	return -1;
}

static CORBA_char *
gnome_persist_file_get_current_file (GnomePersistFile *pf)
{
	if (pf->filename)
		return pf->filename;
	return "";
}

static void
gnome_persist_file_class_init (GnomePersistFileClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_persist_file_parent_class = gtk_type_class (gnome_persist_get_type ());

	/*
	 * Override and initialize methods
	 */

	class->save = gnome_persist_file_nop;
	class->load = gnome_persist_file_nop;
	class->get_current_file = gnome_persist_file_get_current_file;
	
	init_persist_file_corba_class ();
}

static void
gnome_persist_file_init (GnomePersistFile *pf)
{
}

GtkType
gnome_persist_file_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/PersistFile:1.0",
			sizeof (GnomePersistFile),
			sizeof (GnomePersistFileClass),
			(GtkClassInitFunc) gnome_persist_file_class_init,
			(GtkObjectInitFunc) gnome_persist_file_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_persist_get_type (), &info);
	}

	return type;
}

/**
 * gnome_persist_file_construct:
 * @pf: A GnomePersistFile
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Initializes the GnomePersistFile object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
GnomePersistFile *
gnome_persist_file_construct (GnomePersistFile *pf,
			      GNOME_PersistFile corba_pf,
			      GnomePersistFileIOFn load_fn,
			      GnomePersistFileIOFn save_fn,
			      void *closure)
{
	g_return_val_if_fail (pf != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PERSIST_FILE (pf), NULL);
	g_return_val_if_fail (corba_pf != CORBA_OBJECT_NIL, NULL);

	gnome_persist_construct (GNOME_PERSIST (pf), corba_pf);
	
	pf->load_fn = load_fn;
	pf->save_fn = save_fn;
	pf->closure = closure;

	return pf;
}

static GNOME_PersistFile
create_gnome_persist_file (GnomeUnknown *object)
{
	POA_GNOME_PersistFile *servant;
	CORBA_Object o;

	servant = (POA_GNOME_PersistFile *) g_new0 (GnomeUnknownServant, 1);
	servant->vepv = &gnome_persist_file_vepv;
	POA_GNOME_PersistFile__init ((PortableServer_Servant) servant, &object->ev);
	if (object->ev._major != CORBA_NO_EXCEPTION){
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}
	return (GNOME_PersistFile) gnome_unknown_activate_servant (object, servant);
}


/**
 * gnome_persist_file_new:
 * @load_fn: Loading routine
 * @save_fn: Saving routine
 * @closure: Data passed to IO routines.
 *
 * Creates a GnomePersistFile object.  The @load_fn and @save_fn
 * parameters might be NULL.  If this is the case, the load and save 
 * operations are performed by the class load and save methods
 */
GnomePersistFile *
gnome_persist_file_new (GnomePersistFileIOFn load_fn,
			GnomePersistFileIOFn save_fn,
			void *closure)
{
	GnomePersistFile *pf;
	GNOME_PersistFile corba_pf;

	pf = gtk_type_new (gnome_persist_file_get_type ());
	corba_pf = create_gnome_persist_file (
		GNOME_UNKNOWN (pf));
	if (corba_pf == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (pf));
		return NULL;
	}

	pf->filename = NULL;

	gnome_persist_file_construct (pf, corba_pf, load_fn, save_fn, closure);

	return pf;
}

