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

static POA_GNOME_Storage__epv gnome_storage_epv;
static POA_GNOME_Storage__vepv gnome_storage_vepv;

static GNOME_Stream
impl_create_stream (PortableServer_Servant servant, const CORBA_char * path, CORBA_Environment * ev)
{
}

static GNOME_Stream
impl_open_stream (PortableServer_Servant servant,
		  const CORBA_char * path,
		  const GNOME_Storage_OpenMode mode,
		  CORBA_Environment * ev)
{

}

static GNOME_Storage 
impl_create_storage (PortableServer_Servant servant,
		     const CORBA_char * path,
		     CORBA_Environment * ev)
{

}

static GNOME_Storage
impl_open_storage (PortableServer_Servant servant,
		   const CORBA_char * path,
		   const GNOME_Storage_OpenMode mode,
		   CORBA_Environment * ev)
{
}

static void
impl_copy_to (PortableServer_Servant servant,
	      const GNOME_Storage target,
	      CORBA_Environment * ev)
{
}

static void
impl_rename (PortableServer_Servant servant,
	     const CORBA_char * path_name,
	     const CORBA_char * new_path_name,
	     CORBA_Environment * ev)
{
}

static void
impl_commit (PortableServer_Servant servant, CORBA_Environment * ev)
{
}

static GNOME_Storage_directory_list *
impl_list_contents (PortableServer_Servant servant,
		    const CORBA_char * path,
		    CORBA_Environment * ev)
{
}

gnome_save (GnomeObject *object, GnomeStorage *storage)
{
	GnomePersistStorage *persist;
	
	g_return_val_if_fail (object != NULL);
	g_return_val_if_fail (storage != NULL);

	persist = gtk_object_query_interface (
		GTK_OBJECT (object), gnome_persist_storage_get_type ());
	if (!persist)
		return SOME_ERROR;

	id = gnome_persist_storage_getid (persist);
	gnome_persist_storage_save (persist, storage);
	
	if (gnome_persist_storage_save (persist, storage) == NO_ERROR)
		gnome_persist_storage_commit (persist);
}

static void
init_storage_corba_class (void)
{
	/* The EPV for the Storage */
	gnome_storage_epv.create_stream = impl_create_stream;
	gnome_storage_epv.open_stream = impl_open_stream;
	gnome_storage_epv.create_storage = impl_create_storage;
	gnome_storage_epv.open_storage = impl_open_storage;
	gnome_storage_epv.open_to = impl_open_to;
	gnome_storage_epv.rename = impl_rename;
	gnome_storage_epv.commit = impl_commit;
	gnome_storage_epv.list_contents = impl_list_contents;
	gnome_storage_epv.destroy = impl_destroy;

	/* The VEPV */
	gnome_storage_vepv.GNOME_object_epv = &gnome_object_epv;
	gnome_storage_vepv.GNOME_Storage_epv = &gnome_storage_epv;
}

static void
gnome_storage_class_init (GnomeInPlaceComponentClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_storage_parent_class = gtk_type_class (gnome_component_get_type ());
	object_class->destroy = gnome_storage_destroy;

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
gnome_storage_construct (GnomeStorage *storage, GNOME_Storage corba_storage,
			 const char *path, const char *open_mode)
{
	StorageHandle *handle;
	
	g_return_val_if_fail (storage != NULL);
	g_return_val_if_fail (GNOME_IS_STORAGE (storage), NULL);
	g_return_val_if_fail (corba_storage != CORBA_OBJECT_NIL, NULL);

	handle = storage_file_open (path, open_mode);
	if (handle == NULL){
		CORBA_free (corba_storage);
		gtk_object_unref (GTK_OBJECT (storage));
		return NULL;
	}
	
	gnome_object_construct (
		GNOME_OBJECT (storage),
		(CORBA_Object) corba_storage);

	
	return storage;
}

GnomeStorage *
gnome_storage_file_open (const char *path, const char *open_mode)
{
	GnomeStorage *storage;
	GNOME_Storage corba_storage;
	
	g_return_val_if_fail (path != NULL, NULL);
	g_return_val_if_fail (open_mode != NULL, NULL);

	storage = gtk_type_new (gnome_storage_get_type ());
	corba_storage = create_gnome_storage (
		GNOME_OBJECT (storage));
	if (corba_storage == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (storage));
		return NULL;
	}

	return gnome_storage_construct (storage, corba_storage);
}

