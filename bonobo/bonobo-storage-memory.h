/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-storage-memory.h: Memory based Bonobo::Storage implementation
 *
 * Author:
 *   ÉRDI Gergõ <cactus@cactus.rulez.org>
 *
 * Copyright 2001 Gergõ Érdi
 */
#ifndef _BONOBO_STORAGE_MEM_H_
#define _BONOBO_STORAGE_MEM_H_

#include <bonobo/bonobo-storage.h>

G_BEGIN_DECLS

#define BONOBO_STORAGE_MEM_TYPE        (bonobo_storage_mem_get_type ())
#define BONOBO_STORAGE_MEM(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_STORAGE_MEM_TYPE, BonoboStorageMem))
#define BONOBO_STORAGE_MEM_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_STORAGE_MEM_TYPE, BonoboStorageMemClass))
#define BONOBO_IS_STORAGE_MEM(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_STORAGE_MEM_TYPE))
#define BONOBO_IS_STORAGE_MEM_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_MEM_TYPE))

typedef struct _BonoboStorageMemPriv BonoboStorageMemPriv;

typedef struct {
	BonoboObject parent;

	BonoboStorageMemPriv *priv;
} BonoboStorageMem;

typedef struct {
	BonoboObjectClass parent_class;

	POA_Bonobo_Storage__epv epv;

	Bonobo_StorageInfo * (*get_info) (BonoboStorageMem         *storage,
					  const CORBA_char         *path,
					  Bonobo_StorageInfoFields  mask,
					  CORBA_Environment
					  *ev);
	void (*set_info) (BonoboStorageMem         *storage,
			  const CORBA_char         *path,
			  const Bonobo_StorageInfo *info,
			  Bonobo_StorageInfoFields  mask,
			  CORBA_Environment        *ev);

	Bonobo_Stream (*open_stream) (BonoboStorageMem       *storage,
				      const CORBA_char       *path,
				      Bonobo_Storage_OpenMode mode,
				      CORBA_Environment      *ev);

	Bonobo_Storage (*open_storage) (BonoboStorageMem         *storage,
					const CORBA_char         *path,
					Bonobo_Storage_OpenMode   mode,
					CORBA_Environment        *ev);

	void (*copy_to) (BonoboStorageMem     *storage,
			 const Bonobo_Storage  target,
			 CORBA_Environment    *ev);
	
	Bonobo_Storage_DirectoryList * (*list_contents) (BonoboStorageMem         *storage,
							 const CORBA_char         *path,
							 Bonobo_StorageInfoFields  mask,
							 CORBA_Environment        *ev);
	
	void (*erase) (BonoboStorageMem  *storage,
		       const CORBA_char  *path,
		       CORBA_Environment *ev);

	void (*rename) (BonoboStorageMem  *storage,
			const CORBA_char  *path,
			const CORBA_char  *new_path,
			CORBA_Environment *ev);

	void (*commit) (BonoboStorageMem  *storage,
			CORBA_Environment *ev);

	void (*revert) (BonoboStorageMem  *storage,
			CORBA_Environment *ev);

} BonoboStorageMemClass;

GType             bonobo_storage_mem_get_type   (void) G_GNUC_CONST;
BonoboObject     *bonobo_storage_mem_create     (void);

G_END_DECLS

#endif /* _BONOBO_STORAGE_MEM_H_ */
