/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * gnome-storage.c: Storage manipulation.
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 1999 Helix Code, Inc.
 */
#include <config.h>
#include <gmodule.h>
#include <bonobo/bonobo-storage.h>
#include <bonobo/bonobo-storage-plugin.h>

static BonoboObjectClass *bonobo_storage_parent_class;

static POA_Bonobo_Storage__vepv bonobo_storage_vepv;

#define CLASS(o) BONOBO_STORAGE_CLASS(GTK_OBJECT(o)->klass)

static inline BonoboStorage *
bonobo_storage_from_servant (PortableServer_Servant servant)
{
	return BONOBO_STORAGE (bonobo_object_from_servant (servant));
}

static Bonobo_StorageInfo*
impl_get_info (PortableServer_Servant servant,
	       const CORBA_char * path,
	       const Bonobo_StorageInfoFields mask,
	       CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	return CLASS (storage)->get_info (storage, path, mask, ev);
}

static void          
impl_set_info (PortableServer_Servant servant,
	       const CORBA_char * path,
	       const Bonobo_StorageInfo *info,
	       const Bonobo_StorageInfoFields mask,
	       CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	CLASS (storage)->set_info (storage, path, info, mask, ev);
}

static Bonobo_Stream
impl_open_stream (PortableServer_Servant servant,
		  const CORBA_char      *path,
		  Bonobo_Storage_OpenMode mode,
		  CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	BonoboStream *stream;
	
	if ((stream = CLASS (storage)->open_stream (storage, path, mode, ev)))
		return (Bonobo_Stream) CORBA_Object_duplicate (
			bonobo_object_corba_objref (BONOBO_OBJECT (stream)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static Bonobo_Storage
impl_open_storage (PortableServer_Servant servant,
		   const CORBA_char      *path,
		   Bonobo_Storage_OpenMode mode,
		   CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	BonoboStorage *open_storage;
	
	if ((open_storage = CLASS(storage)->open_storage (storage, path, mode, ev)))
		return (Bonobo_Storage) CORBA_Object_duplicate (
			bonobo_object_corba_objref (BONOBO_OBJECT (open_storage)), ev);
	else
		return CORBA_OBJECT_NIL;
}

static void
impl_copy_to (PortableServer_Servant servant,
	      Bonobo_Storage          target,
	      CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS(storage)->copy_to (storage, target, ev);
}

static void
impl_rename (PortableServer_Servant servant,
	     const CORBA_char      *path_name,
	     const CORBA_char      *new_path_name,
	     CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->rename (storage, path_name, new_path_name, ev);
}

static void
impl_commit (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static void
impl_revert (PortableServer_Servant servant, CORBA_Environment *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->commit (storage, ev);
}

static Bonobo_Storage_DirectoryList *
impl_list_contents (PortableServer_Servant servant,
		    const CORBA_char      *path,
		    Bonobo_StorageInfoFields mask,
		    CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);

	return CLASS (storage)->list_contents (storage, path, mask, ev);
}

static void
impl_erase (PortableServer_Servant servant, 
            const CORBA_char      *path,
            CORBA_Environment     *ev)
{
	BonoboStorage *storage = bonobo_storage_from_servant (servant);
	
	CLASS (storage)->erase (storage, path, ev);
}

/**
 * bonobo_storage_get_epv:
 *
 */
POA_Bonobo_Storage__epv *
bonobo_storage_get_epv (void)
{
	POA_Bonobo_Storage__epv *epv;

	epv = g_new0 (POA_Bonobo_Storage__epv, 1);

	epv->get_info	        = impl_get_info;
	epv->set_info	        = impl_set_info;
	epv->open_stream	= impl_open_stream;
	epv->open_storage	= impl_open_storage;
	epv->copy_to		= impl_copy_to;
	epv->rename		= impl_rename;
	epv->commit		= impl_commit;
	epv->revert		= impl_revert;
	epv->list_contents	= impl_list_contents;
	epv->erase		= impl_erase;

	return epv;
}

static void
init_storage_corba_class (void)
{
	/* The VEPV */
	bonobo_storage_vepv.Bonobo_Unknown_epv = bonobo_object_get_epv ();
	bonobo_storage_vepv.Bonobo_Storage_epv = bonobo_storage_get_epv ();
}

static void
bonobo_storage_class_init (BonoboStorageClass *klass)
{
	bonobo_storage_parent_class = gtk_type_class (bonobo_object_get_type ());

	init_storage_corba_class ();
}

static void
bonobo_storage_init (BonoboObject *object)
{
}

/**
 * bonobo_storage_get_type:
 *
 * Returns: The GtkType for the BonoboStorage class.
 */
GtkType
bonobo_storage_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStorage",
			sizeof (BonoboStorage),
			sizeof (BonoboStorageClass),
			(GtkClassInitFunc) bonobo_storage_class_init,
			(GtkObjectInitFunc) bonobo_storage_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_storage_construct:
 * @storage: The BonoboStorage object to be initialized
 * @corba_storage: The CORBA object for the Bonobo_Storage interface.
 *
 * Initializes @storage using the CORBA interface object specified
 * by @corba_storage.
 *
 * Returns: the initialized BonoboStorage object @storage.
 */
BonoboStorage *
bonobo_storage_construct (BonoboStorage *storage, Bonobo_Storage corba_storage)
{
	g_return_val_if_fail (storage != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_STORAGE (storage), NULL);
	g_return_val_if_fail (corba_storage != CORBA_OBJECT_NIL, NULL);

	bonobo_object_construct (
		BONOBO_OBJECT (storage),
		(CORBA_Object) corba_storage);

	
	return storage;
}

/**
 * bonobo_storage_open:
 * @driver: driver to use for opening.
 * @path: path where the base file resides
 * @flags: Bonobo Storage OpenMode
 * @mode: Unix open(2) mode
 *
 * Opens or creates the file named at @path with the stream driver @driver.
 *
 * @driver is one of: "efs", "vfs" or "fs" for now.
 *
 * Returns: a created BonoboStorage object.
 */
BonoboStorage *
bonobo_storage_open (const char *driver, const char *path, gint flags, 
		     gint mode)
{
	StoragePlugin *p;

	g_return_val_if_fail (driver != NULL, NULL);
	g_return_val_if_fail (path != NULL, NULL);

	if (!(p = bonobo_storage_plugin_find (driver))) return NULL;

	if (p->storage_open) return p->storage_open (path, flags, mode);

	return NULL;
}

/**
 * bonobo_storage_corba_object_create:
 * @object: the GtkObject that will wrap the CORBA object
 *
 * Creates and activates the CORBA object that is wrapped by the
 * @object BonoboObject.
 *
 * Returns: An activated object reference to the created object
 * or %CORBA_OBJECT_NIL in case of failure.
 */
Bonobo_Storage
bonobo_storage_corba_object_create (BonoboObject *object)
{
        POA_Bonobo_Storage *servant;
        CORBA_Environment ev;

        servant = (POA_Bonobo_Storage *) g_new0 (BonoboObjectServant, 1);
        servant->vepv = &bonobo_storage_vepv;

        CORBA_exception_init (&ev);

        POA_Bonobo_Storage__init ((PortableServer_Servant) servant, &ev);
        if (ev._major != CORBA_NO_EXCEPTION){
                g_free (servant);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        CORBA_exception_free (&ev);
        return (Bonobo_Storage) bonobo_object_activate_servant (object, servant);
}

