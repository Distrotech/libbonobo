/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-storage-memory.c: Memory based Bonobo::Storage implementation
 *
 * Author:
 *   ÉRDI Gergõ <cactus@cactus.rulez.org>
 *
 * Copyright 2001 Gergõ Érdi
 *
 * TODO: Make it implement PersistStream so you can flatten a
 * StorageMem into a Stream
 */
#include <config.h>

#include <bonobo/bonobo-storage-memory.h>
#include <bonobo/bonobo-stream-memory.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-storage.h>

static BonoboObjectClass *bonobo_storage_mem_parent_class;

typedef struct {

	gboolean      is_directory;
	BonoboObject *child;
} BonoboStorageMemEntry;

struct _BonoboStorageMemPriv {
	GHashTable *entries;
};

typedef struct {
	GList                    *list;
	Bonobo_StorageInfoFields  mask;
} DirCBData;

static void
bonobo_storage_mem_entry_free (gpointer data)
{
	BonoboStorageMemEntry *entry = (BonoboStorageMemEntry*) data;

	if (!data)
		return;
	
	bonobo_object_unref (entry->child);
	
	g_free (entry);
}

static BonoboStorageMemEntry *
bonobo_storage_mem_entry_dup (BonoboStorageMemEntry *entry)
{
	BonoboStorageMemEntry *ret_val = g_new0 (BonoboStorageMemEntry, 1);

	ret_val->is_directory = entry->is_directory;
	ret_val->child = entry->child;

	bonobo_object_ref (ret_val->child);

	return ret_val;
}

static void
split_path (const char  *path,
	    char       **path_head,
	    char       **path_tail)
{
	gchar **path_parts;
	
	if (g_path_is_absolute (path))
		path = g_path_skip_root (path);
	
	path_parts = g_strsplit (path, "/", 1);
	
	*path_head = path_parts[0];
	*path_tail = path_parts[1];

	g_free (path_parts);	
}

static BonoboStorageMem *
smem_get_parent (BonoboStorageMem       *storage,
		 const char             *path,
		 char                  **filename,  /* g_free this */
		 BonoboStorageMemEntry **ret_entry) /* g_free this */
{
	BonoboStorageMem      *ret;
	BonoboStorageMemEntry *entry;
	gchar *path_head, *path_tail;

	if (!strcmp (path, "/") || !strcmp (path, "")) {
		if (filename)
			*filename = g_strdup ("/");
		if (ret_entry) {
			*ret_entry = g_new0 (BonoboStorageMemEntry, 1);
			(*ret_entry)->is_directory = TRUE;
			(*ret_entry)->child = BONOBO_OBJECT (storage);
			bonobo_object_ref ((*ret_entry)->child);
		}

		return storage;
	}

	split_path (path, &path_head, &path_tail);
	entry = g_hash_table_lookup (storage->priv->entries,
				     path_head);

	/* No child is found */
	if (!entry) {
		g_free (path_head);
		g_free (path_tail);

		if (filename)
			*filename = 0;

		if (ret_entry)
			*ret_entry = 0;
		
		return NULL;
	}

	/* This is not the immediate parent */
	if (path_tail && entry->is_directory) {
		ret = smem_get_parent (
			BONOBO_STORAGE_MEM (entry->child),
			path_tail,
			filename,
			ret_entry);
		
		g_free (path_head);
		g_free (path_tail);
		return ret;
	}

	/* This is the immediate parent */
	if (filename)
		*filename = g_strdup (path_head);
	if (ret_entry)
		*ret_entry = bonobo_storage_mem_entry_dup (entry);
	
	g_free (path_tail);
	g_free (path_head);
	
	return storage;
}

static BonoboStorageMem *
smem_get_last_storage (BonoboStorageMem  *storage,
		       const char        *path,
		       char             **last_path)
{
	BonoboStorageMem      *ret;
	BonoboStorageMemEntry *entry;
	gchar *path_head, *path_tail;

	if (!strcmp (path, "/") || !strcmp (path, "")) {
		if (last_path)
			*last_path = 0;
		return storage;
	}
	
	split_path (path, &path_head, &path_tail);
	entry = g_hash_table_lookup (storage->priv->entries,
				     path_head);

	/* No appropriate child is found */
	if (!entry) {
		if (path_tail) {
			g_free (path_head);
			g_free (path_tail);

			if (last_path)
				*last_path = 0;
			return NULL;
		} else {
			if (last_path)
				*last_path = path_head;
			
			return storage;
		}
	}

	if (!path_tail) {
		if (entry->is_directory) {
			g_free (path_head);

			if (last_path)
				*last_path = 0;
			
			return BONOBO_STORAGE_MEM (entry->child);
		} else {
			if (last_path)
				*last_path = path_head;
			
			return storage;
		}
	}

	if (path_tail && entry->is_directory) {
		ret = smem_get_last_storage (
			BONOBO_STORAGE_MEM (entry->child),
			path_tail, last_path);
		g_free (path_head);
		g_free (path_tail);
		return ret;
	}

	g_free (path_tail);
	g_free (path_head);

	if (last_path)
		*last_path = 0;
	
	return NULL;
}

static Bonobo_StorageInfo *
smem_get_stream_info (BonoboObject                   *stream,
		      const Bonobo_StorageInfoFields  mask,
		      CORBA_Environment              *ev)
{
	Bonobo_StorageInfo *ret_val;
	CORBA_Environment   my_ev;
	
	CORBA_exception_init (&my_ev);
	ret_val = Bonobo_Stream_getInfo (bonobo_object_corba_objref (stream),
					 mask, &my_ev);
	
	if (BONOBO_EX (&my_ev)) {
		if (BONOBO_USER_EX (&my_ev, ex_Bonobo_Stream_IOError))
			bonobo_exception_set (ev, ex_Bonobo_Storage_IOError);
		if (BONOBO_USER_EX (&my_ev, ex_Bonobo_Stream_NoPermission))
			bonobo_exception_set (ev, ex_Bonobo_Storage_NoPermission);
		if (BONOBO_USER_EX (&my_ev, ex_Bonobo_Stream_NotSupported))
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotSupported);
	}
	
	if (mask & Bonobo_FIELD_TYPE)
		ret_val->type = Bonobo_STORAGE_TYPE_REGULAR;
	
	CORBA_exception_free (&my_ev);

	return ret_val;
}

static Bonobo_Stream
smem_open_stream (PortableServer_Servant   servant,
		  const CORBA_char        *path,
		  Bonobo_Storage_OpenMode  mode,
		  CORBA_Environment       *ev)
{
	BonoboStorageMem       *storage, *last_storage;
	BonoboStorageMemEntry  *entry;
	gchar                  *path_last;
	BonoboObject           *stream = 0;
	
	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));

	last_storage = smem_get_last_storage (storage, path, &path_last);
	
	if (!last_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto ex_out;
	}

	
	entry = g_hash_table_lookup (last_storage->priv->entries, path_last);

	/* Error cases */
	/* Case 1: Stream not found */
	if (!entry && !(mode & Bonobo_Storage_CREATE)) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto ex_out;
	}

	/* Case 2: A storage by the same name exists */
	if (entry && entry->is_directory) {
		if (mode & Bonobo_Storage_CREATE)
			bonobo_exception_set (ev, ex_Bonobo_Storage_NameExists);
		else
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotStream);
		goto ex_out;
	}

	if (!entry) {
		stream = bonobo_stream_mem_create (NULL, 0,
						   FALSE, TRUE);
		entry = g_new0 (BonoboStorageMemEntry, 1);
		entry->is_directory = FALSE;
		entry->child = stream;

		g_hash_table_insert (last_storage->priv->entries,
				     g_strdup (path_last),
				     entry);
		goto ok_out;
	}
	
	stream = entry->child;
	
 ok_out:
	g_free (path_last);
	
	return bonobo_object_corba_objref (stream);
	
 ex_out:
	g_free (path_last);
	
	return CORBA_OBJECT_NIL;
}

static Bonobo_Storage
smem_open_storage (PortableServer_Servant   servant,
		   const CORBA_char        *path,
		   Bonobo_Storage_OpenMode  mode,
		   CORBA_Environment       *ev)
{
	BonoboStorageMem       *storage, *parent_storage;
	BonoboStorageMemEntry  *entry;
	BonoboObject           *ret = 0;
	gchar                  *path_last = 0;
	
	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));

	parent_storage = smem_get_last_storage (storage, path, &path_last);
	
	if (!parent_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto ex_out;
	}

	entry = g_hash_table_lookup (parent_storage->priv->entries, path_last);
	
	/* Error cases */
	/* Case 1: Storage not found */
	if (!entry && !(mode & Bonobo_Storage_CREATE)) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto ex_out;
	}

	/* Case 2: A stream by the same name exists */
	if (entry && !entry->is_directory) {
		if (mode & Bonobo_Storage_CREATE)
			bonobo_exception_set (ev, ex_Bonobo_Storage_NameExists);
		else
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotStorage);
		goto ex_out;
	}

	if (!entry) {
		ret = bonobo_storage_mem_create ();
		entry = g_new0 (BonoboStorageMemEntry, 1);
		entry->is_directory = TRUE;
		entry->child = ret;

		g_hash_table_insert (parent_storage->priv->entries,
				     g_strdup (path_last),
				     entry);
		goto ok_out;
	}

	ret = entry->child;
	
 ok_out:
	g_free (path_last);
	
	return bonobo_object_corba_objref (ret);
	
 ex_out:
	g_free (path_last);
	
	return CORBA_OBJECT_NIL;
}

static Bonobo_StorageInfo*
smem_get_info (PortableServer_Servant          servant,
	       const CORBA_char               *path,
	       const Bonobo_StorageInfoFields  mask,
	       CORBA_Environment              *ev)
{
	Bonobo_StorageInfo     *ret_val = 0;
	BonoboStorageMem       *storage, *parent_storage = 0;
	BonoboStorageMemEntry  *entry = 0;
	gchar                  *filename = 0;

	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));

	parent_storage = smem_get_parent (storage, path, &filename, &entry);

	if (!parent_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto out;
	}

	if (entry->is_directory) {
		if (mask & Bonobo_FIELD_CONTENT_TYPE ||
		    mask & Bonobo_FIELD_SIZE) {
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotSupported);
			goto out;
		}
		
		ret_val = Bonobo_StorageInfo__alloc ();

		ret_val->name = CORBA_string_dup (filename);

		if (mask & Bonobo_FIELD_TYPE)
			ret_val->type = Bonobo_STORAGE_TYPE_DIRECTORY;
		
	} else {
		
		if (mask & Bonobo_FIELD_CONTENT_TYPE ||
		    mask & Bonobo_FIELD_SIZE)
			ret_val = smem_get_stream_info (entry->child, mask, ev);
		else
			ret_val = Bonobo_StorageInfo__alloc ();

		ret_val->name = CORBA_string_dup (filename);
		ret_val->type = Bonobo_STORAGE_TYPE_REGULAR;
		
	}


 out:
	bonobo_storage_mem_entry_free (entry);	
	g_free (filename);
	
	return ret_val;
}

static void
smem_set_info (PortableServer_Servant          servant,
	       const CORBA_char               *path,
	       const Bonobo_StorageInfo       *info,
	       const Bonobo_StorageInfoFields  mask,
	       CORBA_Environment              *ev)
{
	BonoboStorageMem      *storage, *parent_storage;
	BonoboStorageMemEntry *entry = 0;
	gchar                 *filename;

	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));

	parent_storage = smem_get_parent (storage, path, &filename, &entry);

	if (!parent_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto out;
	}

	if (entry->is_directory)
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotSupported);
	else {
		CORBA_Environment my_ev;

		CORBA_exception_init (&my_ev);
		Bonobo_Stream_setInfo (
			bonobo_object_corba_objref (entry->child),
			info, mask,
			&my_ev);
		
		if (BONOBO_EX (&my_ev)) {
			if (BONOBO_USER_EX (&my_ev, ex_Bonobo_Stream_IOError))
				bonobo_exception_set (ev, ex_Bonobo_Storage_IOError);
			if (BONOBO_USER_EX (&my_ev, ex_Bonobo_Stream_NoPermission))
				bonobo_exception_set (ev, ex_Bonobo_Storage_NoPermission);
			if (BONOBO_USER_EX (&my_ev, ex_Bonobo_Stream_NotSupported))
				bonobo_exception_set (ev, ex_Bonobo_Storage_NotSupported);
		}
			
		CORBA_exception_free (&my_ev);
	}
	
 out:
	g_free (filename);
	bonobo_storage_mem_entry_free (entry);
}

static void
smem_dir_hash_cb (gpointer key,
		  gpointer value,
		  gpointer user_data)
{
	DirCBData                *cb_data = user_data;
	gchar                    *filename = key;
	BonoboStorageMemEntry    *entry = value;
	Bonobo_StorageInfo       *info;
	Bonobo_StorageInfoFields  mask = cb_data->mask;
	
	if (entry->is_directory) {
		info = Bonobo_StorageInfo__alloc ();
		info->name = CORBA_string_dup (filename);
		info->type = Bonobo_STORAGE_TYPE_DIRECTORY;
	} else {
		if (mask & Bonobo_FIELD_CONTENT_TYPE ||
		    mask & Bonobo_FIELD_SIZE) {
			CORBA_Environment my_ev;
			
			CORBA_exception_init (&my_ev);
			info = smem_get_stream_info (entry->child, mask, &my_ev);
			CORBA_exception_free (&my_ev);
		}
		else
			info = Bonobo_StorageInfo__alloc ();

		info->name = CORBA_string_dup (filename);
		info->type = Bonobo_STORAGE_TYPE_REGULAR;
	}

	cb_data->list = g_list_prepend (cb_data->list, info);
}

static Bonobo_Storage_DirectoryList *
smem_get_dir (PortableServer_Servant          servant,
	      const CORBA_char               *path,
	      const Bonobo_StorageInfoFields  mask,
	      CORBA_Environment              *ev)
{
	Bonobo_Storage_DirectoryList *ret_val = 0;
	Bonobo_StorageInfo           *info;
	BonoboStorageMem             *storage, *last_storage;
	gchar                        *path_last;
	GList                        *list;
	DirCBData                     cb_data;
	int                           i;
	
	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));

	last_storage = smem_get_last_storage (storage, path, &path_last);

	if (!last_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto out;
	}

	if (path_last) { /* The requested entry is a stream or does not
			    exist */
		if (g_hash_table_lookup (last_storage->priv->entries, path_last))
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotStorage);
		else
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto out;
	}

	cb_data.list = 0;
	cb_data.mask = mask;

	g_hash_table_foreach (last_storage->priv->entries,
			      smem_dir_hash_cb, &cb_data);

	ret_val = Bonobo_Storage_DirectoryList__alloc ();
	list = cb_data.list;
	ret_val->_length = g_list_length (list);
	ret_val->_buffer = Bonobo_Storage_DirectoryList_allocbuf (ret_val->_length);

	for (i = 0; list != NULL; list = list->next, i++) {
		info = list->data;

		ret_val->_buffer[i].name = CORBA_string_dup (info->name);
		ret_val->_buffer[i].type = info->type;
		ret_val->_buffer[i].content_type = CORBA_string_dup (info->content_type);
		ret_val->_buffer[i].size = info->size;

		CORBA_free (info);
	}
	g_list_free (cb_data.list);
	
 out:
	if (path_last)
		g_free (path_last);
	return ret_val;
}

static void
smem_erase (PortableServer_Servant  servant,
	    const CORBA_char       *path,
	    CORBA_Environment      *ev)
{
	BonoboStorageMemEntry  *entry = 0;
	BonoboStorageMem       *storage, *parent_storage;
	gchar                  *filename = 0;
			

	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));
	
	parent_storage = smem_get_parent (storage, path, &filename, &entry);
	if (!parent_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
	}

	if (entry->is_directory) {
		BonoboStorageMem *storage_to_remove =
			BONOBO_STORAGE_MEM (entry->child);
		
		/* You can't remove the root item */
		if (!strcmp (path, "/") || !strcmp (path, "")) {
			bonobo_exception_set (ev, ex_Bonobo_Storage_IOError);
			goto out;
		}

		/* Is the storage empty? */
		if (g_hash_table_size (storage_to_remove->priv->entries)) {
			bonobo_exception_set (ev, ex_Bonobo_Storage_NotEmpty);
			goto out;
		}
		g_hash_table_remove (parent_storage->priv->entries, filename);
		
	} else {
		
		g_hash_table_remove (parent_storage->priv->entries,
				     filename);
	}

 out:
	bonobo_storage_mem_entry_free (entry);
	g_free (filename);
}

static void
smem_copy (PortableServer_Servant  servant,
	   Bonobo_Storage          dest,
	   CORBA_Environment      *ev)
{
	BonoboObject *storage = bonobo_object_from_servant (servant);

	bonobo_storage_copy_to (bonobo_object_corba_objref (storage),
				dest, ev);
}

static void
smem_rename (PortableServer_Servant  servant,
	     const CORBA_char       *path,
	     const CORBA_char       *new_path,
	     CORBA_Environment      *ev)
{
	BonoboStorageMem      *storage, *parent_storage, *target_storage;
	BonoboStorageMemEntry *entry;
	gchar                 *filename, *new_filename;
	BonoboStorageMemEntry *tmp_entry;
	
	storage = BONOBO_STORAGE_MEM (bonobo_object_from_servant (servant));

	if (!strcmp (path, "/") || !strcmp (path, "")) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_IOError);
		goto out;
	}

	parent_storage = smem_get_parent (storage, path, &filename, &entry);
	target_storage = smem_get_last_storage (storage, new_path, &new_filename);

	/* Source or target does not exists */
	if (!parent_storage || !target_storage) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NotFound);
		goto out;
	}

	/* Target exists and is not a storage */
	if (new_filename && g_hash_table_lookup (target_storage->priv->entries,
						 new_filename)) {
		bonobo_exception_set (ev, ex_Bonobo_Storage_NameExists);
		goto out;
	}

	tmp_entry = bonobo_storage_mem_entry_dup (entry);
	g_hash_table_remove (parent_storage->priv->entries, filename);
	entry = 0;
	
	/* If target does not exists, new_filename will be non-NULL */
	if (new_filename)
		g_hash_table_insert (target_storage->priv->entries,
				     new_filename, tmp_entry);
	else
		g_hash_table_insert (target_storage->priv->entries,
				     g_strdup (filename), tmp_entry);
	
 out:
	bonobo_storage_mem_entry_free (entry);
	g_free (filename);
}

static void
smem_unimplemented (PortableServer_Servant  servant,
		    CORBA_Environment      *ev)
{
	bonobo_exception_set (ev, ex_Bonobo_Storage_NotSupported);
}

static void
bonobo_storage_mem_finalize (GObject *object)
{
	BonoboStorageMem *smem = BONOBO_STORAGE_MEM (object);

	if (smem->priv) {
		g_hash_table_destroy (smem->priv->entries);
		g_free (smem->priv);
		smem->priv = NULL;
	}
	
	G_OBJECT_CLASS (bonobo_storage_mem_parent_class)->finalize (object);
}

static void
bonobo_storage_mem_init (BonoboStorageMem *smem)
{
	smem->priv = g_new0 (BonoboStorageMemPriv, 1);

	smem->priv->entries = g_hash_table_new_full (g_str_hash,
						     g_str_equal,
						     g_free,
						     bonobo_storage_mem_entry_free);
}

static void
bonobo_storage_mem_class_init (BonoboStorageMemClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	POA_Bonobo_Storage__epv *epv = &klass->epv;
	
	bonobo_storage_mem_parent_class = g_type_class_peek_parent (klass);

	object_class->finalize = bonobo_storage_mem_finalize;

	epv->getInfo  = smem_get_info;
	epv->setInfo  = smem_set_info;
	epv->listContents = smem_get_dir;

	epv->openStream = smem_open_stream;
	epv->openStorage = smem_open_storage;

	epv->copyTo = smem_copy;
	epv->erase = smem_erase;
	epv->rename = smem_rename;
	
	epv->commit = smem_unimplemented;
	epv->revert = smem_unimplemented;
}

BONOBO_TYPE_FUNC_FULL (BonoboStorageMem, 
		       Bonobo_Storage,
		       BONOBO_OBJECT_TYPE,
		       bonobo_storage_mem);

BonoboObject *
bonobo_storage_mem_create ()
{
	BonoboStorageMem *smem;

	smem = g_object_new (bonobo_storage_mem_get_type (), NULL);
	
	if (!smem)
		return NULL;
	
	return BONOBO_STORAGE (smem);
}

