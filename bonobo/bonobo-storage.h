/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * gnome-storage.h: Storage manipulation.
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 1999 Helix Code, Inc.
 */
#ifndef _BONOBO_STORAGE_H_
#define _BONOBO_STORAGE_H_

#include <bonobo/bonobo-xobject.h>

BEGIN_GNOME_DECLS

/* Constants to make it easier to safely select drivers */
#define BONOBO_IO_DRIVER_FS  "fs"
#define BONOBO_IO_DRIVER_EFS "efs"
#define BONOBO_IO_DRIVER_VFS "vfs"

/* For backwards compatibility */
#define BonoboStream BonoboXObject
#define BONOBO_STREAM(o)       ((BonoboStream *)(o))
#define BONOBO_STREAM_CLASS(o) (o)

#define BONOBO_STORAGE_TYPE        (bonobo_storage_get_type ())
#define BONOBO_STORAGE(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_STORAGE_TYPE, BonoboStorage))
#define BONOBO_STORAGE_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_STORAGE_TYPE, BonoboStorageClass))
#define BONOBO_IS_STORAGE(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_STORAGE_TYPE))
#define BONOBO_IS_STORAGE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_TYPE))

typedef struct _BonoboStoragePrivate BonoboStoragePrivate;

typedef struct {
        BonoboXObject object;

	BonoboStoragePrivate *priv;
} BonoboStorage;

typedef struct {
	BonoboXObjectClass parent_class;

	POA_Bonobo_Storage__epv epv;

	/* virtual methods */
	Bonobo_StorageInfo *(*get_info) (BonoboStorage *storage,
					 const CORBA_char *path,
					 const Bonobo_StorageInfoFields mask,
					 CORBA_Environment *ev);
	void          (*set_info)       (BonoboStorage *storage,
					 const CORBA_char *path,
					 const Bonobo_StorageInfo * info,
					 const Bonobo_StorageInfoFields mask,
					 CORBA_Environment *ev);
	BonoboStream  *(*open_stream)   (BonoboStorage *storage,
					 const CORBA_char *path,
					 Bonobo_Storage_OpenMode, 
					 CORBA_Environment *ev);
	BonoboStorage *(*open_storage)  (BonoboStorage *storage,
					 const CORBA_char *path,
					 Bonobo_Storage_OpenMode, 
					 CORBA_Environment *ev);
	void         (*copy_to)         (BonoboStorage *storage, 
					 Bonobo_Storage target,
					 CORBA_Environment *ev);
	void         (*rename)          (BonoboStorage *storage,
					 const CORBA_char *path_name,
					 const CORBA_char *new_path_name,
					 CORBA_Environment *ev);
	void         (*commit)          (BonoboStorage *storage,
					 CORBA_Environment *ev);
	void         (*revert)          (BonoboStorage *storage,
					 CORBA_Environment *ev);
	Bonobo_Storage_DirectoryList *
	             (*list_contents)   (BonoboStorage *storage,
					 const CORBA_char *path,
					 Bonobo_StorageInfoFields mask,
					 CORBA_Environment *ev);
	void         (*erase)           (BonoboStorage *storage,
                                         const CORBA_char *path,
                                         CORBA_Environment *ev);
} BonoboStorageClass;

GType          bonobo_storage_get_type     (void);

BonoboStorage   *bonobo_storage_open         (const char *driver,
					      const char *path,
					      gint flags,
					      gint mode);

BonoboStorage   *bonobo_storage_open_full    (const char *driver,
					      const char *path,
					      gint flags,
					      gint mode,
					      CORBA_Environment *opt_ev);

void             bonobo_storage_copy_to      (Bonobo_Storage src, 
					      Bonobo_Storage dest,
					      CORBA_Environment *ev);
  
void             bonobo_storage_write_class_id (BonoboStorage *storage,
						char *class_id);

void             bonobo_stream_write_class_id  (BonoboStream *stream,
						char *class_id);

BonoboStream *bonobo_stream_open (const char *driver,
				  const char *path,
				  gint flags,
				  gint mode,
				  CORBA_Environment *opt_ev);


END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_H_ */

