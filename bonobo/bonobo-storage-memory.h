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
} BonoboStorageMemClass;

GType             bonobo_storage_mem_get_type   (void);
BonoboObject     *bonobo_storage_mem_create     (void);

G_END_DECLS

#endif /* _BONOBO_STORAGE_MEM_H_ */
