/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-storage: Storage manipulation.
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org).
 *
 */
#include <config.h>
#include <gmodule.h>
#include <bonobo/gnome-storage.h>

static GnomeObjectClass *gnome_storage_parent_class;

POA_GNOME_Storage__epv gnome_storage_epv;
POA_GNOME_Storage__vepv gnome_storage_vepv;

#define CLASS(o) GNOME_STORAGE_CLASS(GTK_OBJECT(o)->klass)

static inline GnomeStorage *
gnome_storage_from_servant (PortableServer_Servant servant)
{
	return GNOME_STORAGE (gnome_object_from_servant (servant));
}

static GNOME_Stream
impl_create_stream (PortableServer_Servant servant, CORBA_char *path, CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStream *stream;
	
	if ((stream = CLASS (storage)->create_stream (storage, path, ev)))
		return (GNOME_Stream) CORBA_Object_duplicate (
			gnome_object_corba_objref (GNOME_OBJECT (stream)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static GNOME_Stream
impl_open_stream (PortableServer_Servant servant,
		  CORBA_char *path,
		  GNOME_Storage_OpenMode mode,
		  CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStream *stream;
	
	if ((stream = CLASS (storage)->open_stream (storage, path, mode, ev)))
		return (GNOME_Stream) CORBA_Object_duplicate (
			gnome_object_corba_objref (GNOME_OBJECT (stream)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static GNOME_Storage 
impl_create_storage (PortableServer_Servant servant,
		     CORBA_char *path,
		     CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStorage *new_storage;
	
	if ((new_storage = CLASS(storage)->create_storage (storage, path, ev)))
		return (GNOME_Storage) CORBA_Object_duplicate (
			gnome_object_corba_objref (GNOME_OBJECT (new_storage)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static GNOME_Storage
impl_open_storage (PortableServer_Servant servant,
		   CORBA_char *path,
		   GNOME_Storage_OpenMode mode,
		   CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStorage *open_storage;
	
	if ((open_storage = CLASS(storage)->open_storage (storage, path, ev)))
		return (GNOME_Storage) CORBA_Object_duplicate (
			gnome_object_corba_objref (GNOME_OBJECT (open_storage)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static void
impl_copy_to (PortableServer_Servant servant,
	      const GNOME_Storage target,
	      CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	
	CLASS(storage)->copy_to (storage, target, ev);
}

static void
impl_rename (PortableServer_Servant servant,
	     CORBA_char *path_name,
	     CORBA_char *new_path_name,
	     CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	
	CLASS (storage)->rename (storage, path_name, new_path_name, ev);
}

static void
impl_commit (PortableServer_Servant servant, CORBA_Environment * ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static GNOME_Storage_directory_list *
impl_list_contents (PortableServer_Servant servant,
		    CORBA_char * path,
		    CORBA_Environment * ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);

	return CLASS (storage)->list_contents (storage, path, ev);
}

static void
impl_erase (PortableServer_Servant servant, 
            CORBA_char *path,
            CORBA_Environment * ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	
	CLASS (storage)->erase (storage, path, ev);
}

static void
init_storage_corba_class (void)
{
	/* The EPV for the Storage */
	gnome_storage_epv.create_stream = impl_create_stream;
	gnome_storage_epv.open_stream = impl_open_stream;
	gnome_storage_epv.create_storage = impl_create_storage;
	gnome_storage_epv.open_storage = impl_open_storage;
	gnome_storage_epv.copy_to = impl_copy_to;
	gnome_storage_epv.rename = impl_rename;
	gnome_storage_epv.commit = impl_commit;
	gnome_storage_epv.list_contents = impl_list_contents;
	gnome_storage_epv.erase = impl_erase;

	/* The VEPV */
	gnome_storage_vepv.GNOME_Unknown_epv = &gnome_object_epv;
	gnome_storage_vepv.GNOME_Storage_epv = &gnome_storage_epv;
}

static void
gnome_storage_class_init (GnomeStorageClass *class)
{
	gnome_storage_parent_class = gtk_type_class (gnome_object_get_type ());

	init_storage_corba_class ();
}

static void
gnome_storage_init (GnomeObject *object)
{
}

/**
 * gnome_storage_get_type:
 *
 * Returns: The GtkType for the GnomeStorage class.
 */
GtkType
gnome_storage_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Storage:1.0",
			sizeof (GnomeStorage),
			sizeof (GnomeStorageClass),
			(GtkClassInitFunc) gnome_storage_class_init,
			(GtkObjectInitFunc) gnome_storage_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}

/**
 * gnome_storage_construct:
 * @storage: The GnomeStorage object to be initialized
 * @corba_storage: The CORBA object for the GNOME_Storage interface.
 *
 * Initializes @storage using the CORBA interface object specified
 * by @corba_storage.
 *
 * Returns: the initialized GnomeStorage object @storage.
 */
GnomeStorage *
gnome_storage_construct (GnomeStorage *storage, GNOME_Storage corba_storage)
{
	g_return_val_if_fail (storage != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_STORAGE (storage), NULL);
	g_return_val_if_fail (corba_storage != CORBA_OBJECT_NIL, NULL);

	gnome_object_construct (
		GNOME_OBJECT (storage),
		(CORBA_Object) corba_storage);

	
	return storage;
}

typedef GnomeStorage * (*driver_open_t)(const char *path, gint flags, gint mode);

static driver_open_t
load_storage_driver (const char *driver_name)
{
	GModule *m;
	char *path;
	driver_open_t driver;
	
	path = g_module_build_path (STORAGE_LIB, driver_name);
	m = g_module_open (path, G_MODULE_BIND_LAZY);
	g_free (path);
	
	if (m == NULL){
		g_free (path);
		return NULL;
	}
	
	driver = (driver_open_t) g_module_symbol (m, "gnome_storage_driver_open", NULL);

	return driver;
}

/**
 * gnome_storage_open:
 * @driver: driver to use for opening.
 * @path: path where the base file resides
 * @flags: Unix open(2) flags
 * @mode: Unix open(2) mode
 *
 * Opens or creates the file named at @path with the stream driver @driver.
 *
 * @driver is one of: "efs" or "fs" for now.
 *
 * Returns: a created GnomeStorage object.
 */
GnomeStorage *
gnome_storage_open (const char *driver, const char *path, gint flags, gint mode)
{
	static driver_open_t fs_driver, efs_driver;
	driver_open_t *driver_ptr = NULL;
	
	g_return_val_if_fail (path != NULL, NULL);
	
	if (strcmp (driver, "fs") == 0){
		if (fs_driver == NULL)
			fs_driver = load_storage_driver ("storagefs");
		driver_ptr = &fs_driver;
	} else if (strcmp (driver, "efs") == 0){
		if (efs_driver == NULL)
			efs_driver = load_storage_driver ("storageefs");
		driver_ptr = &efs_driver;
	} else {
		g_warning ("Unknown driver `%s' specified", driver);
		return NULL;
	}

	if (*driver_ptr == NULL)
		return NULL;

	return (*driver_ptr) (path, flags, mode);
}

