/*
 * gnome-storage: Storage manipulation
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org).
 *
 */
#include <config.h>
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
impl_create_stream (PortableServer_Servant servant, const CORBA_char *path, CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStream *stream;
	
	if ((stream = CLASS (storage)->create_stream (storage, path, ev)))
	  return (GNOME_Stream) GNOME_OBJECT (stream)->object;

	return NULL;
}

static GNOME_Stream
impl_open_stream (PortableServer_Servant servant,
		  const CORBA_char *path,
		  const GNOME_Storage_OpenMode mode,
		  CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStream *stream;
	
	if ((stream = CLASS (storage)->open_stream (storage, path, mode, ev)))
	  return (GNOME_Stream) GNOME_OBJECT (stream)->object;

	return NULL;
}

static GNOME_Storage 
impl_create_storage (PortableServer_Servant servant,
		     const CORBA_char *path,
		     CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStorage *new_storage;
	
	if ((new_storage = CLASS(storage)->create_storage (storage, path, ev)))
	  return (GNOME_Storage) GNOME_OBJECT (new_storage)->object;

	return NULL;
}

static GNOME_Storage
impl_open_storage (PortableServer_Servant servant,
		   const CORBA_char *path,
		   const GNOME_Storage_OpenMode mode,
		   CORBA_Environment *ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);
	GnomeStorage *open_storage;
	
	if ((open_storage = CLASS(storage)->open_storage (storage, path, ev)))
	  return (GNOME_Storage) GNOME_OBJECT (open_storage)->object;

	return NULL;
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
	     const CORBA_char *path_name,
	     const CORBA_char *new_path_name,
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
		    const CORBA_char * path,
		    CORBA_Environment * ev)
{
	GnomeStorage *storage = gnome_storage_from_servant (servant);

	CLASS (storage)->list_contents (storage, path, ev);
	return CORBA_OBJECT_NIL;
}

#if 0
gnome_save (GnomeObject *object, GnomeStorage *storage)
{
	GnomePersistStorage *persist;
	
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (storage != NULL, NULL);

	persist = gtk_object_query_interface (
		GTK_OBJECT (object), gnome_persist_storage_get_type ());
	if (!persist)
		return SOME_ERROR;

	id = gnome_persist_storage_getid (persist);
	gnome_persist_storage_save (persist, storage);
	
	if (gnome_persist_storage_save (persist, storage) == NO_ERROR)
		gnome_persist_storage_commit (persist);
}
#endif

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

	/* The VEPV */
	gnome_storage_vepv.GNOME_object_epv = &gnome_object_epv;
	gnome_storage_vepv.GNOME_Storage_epv = &gnome_storage_epv;
}

static void
gnome_storage_class_init (GnomeStorageClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_storage_parent_class = gtk_type_class (gnome_object_get_type ());

	init_storage_corba_class ();
}

static void
gnome_storage_init (GnomeObject *object)
{
}

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

#if 0
GnomeStorage *
gnome_storage_file_open (const char *path, const char *open_mode)
{
	GnomeStorage *storage;
	GNOME_Storage corba_storage;
	
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (open_mode != NULL, NULL);

	storage = gtk_type_new (gnome_storage_get_type ());
	storage->driver = gnome_storage_driver_new ("file", path, open_mode);
	
	corba_storage = create_gnome_storage (
		GNOME_OBJECT (storage));
	if (corba_storage == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (storage));
		return NULL;
	}

	return gnome_storage_construct (storage, corba_storage);
}

#endif
