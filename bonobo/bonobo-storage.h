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

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-stream.h>

BEGIN_GNOME_DECLS

#define BONOBO_STORAGE_TYPE        (bonobo_storage_get_type ())
#define BONOBO_STORAGE(o)          (GTK_CHECK_CAST ((o), BONOBO_STORAGE_TYPE, BonoboStorage))
#define BONOBO_STORAGE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_STORAGE_TYPE, BonoboStorageClass))
#define BONOBO_IS_STORAGE(o)       (GTK_CHECK_TYPE ((o), BONOBO_STORAGE_TYPE))
#define BONOBO_IS_STORAGE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_STORAGE_TYPE))

typedef struct _BonoboStoragePrivate BonoboStoragePrivate;

typedef struct {
        BonoboObject object;

	BonoboStoragePrivate *priv;
} BonoboStorage;

typedef struct {
	BonoboObjectClass parent_class;

	/*
	 * virtual methods
	 */
	BonoboStream  *(*create_stream)  (BonoboStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	BonoboStream  *(*open_stream)    (BonoboStorage *storage,
					 const CORBA_char *path,
					 Bonobo_Storage_OpenMode, CORBA_Environment *ev);
	BonoboStorage *(*create_storage) (BonoboStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	BonoboStorage *(*open_storage)   (BonoboStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	void         (*copy_to)         (BonoboStorage *storage, Bonobo_Storage target,
					 CORBA_Environment *ev);
	void         (*rename)          (BonoboStorage *storage,
					 const CORBA_char *path_name,
					 const CORBA_char *new_path_name,
					 CORBA_Environment *ev);
	void         (*commit)          (BonoboStorage *storage,
					 CORBA_Environment *ev);
	Bonobo_Storage_directory_list *
	             (*list_contents)   (BonoboStorage *storage,
					 const CORBA_char *path,
					 CORBA_Environment *ev);
	void         (*erase)          (BonoboStorage *storage,
                                        const CORBA_char *path,
                                        CORBA_Environment *ev);
} BonoboStorageClass;

GtkType          bonobo_storage_get_type     (void);
BonoboStorage   *bonobo_storage_construct    (BonoboStorage *storage,
					      Bonobo_Storage corba_storage);

BonoboStorage   *bonobo_storage_open         (const char *driver,
					      const char *path,
					      gint flags,
					      gint mode);

Bonobo_Storage   bonobo_storage_corba_object_create (BonoboObject *object);

void             bonobo_storage_write_class_id (BonoboStorage *storage,
						char *class_id);

void             bonobo_stream_write_class_id  (BonoboStream *stream,
						char *class_id);

POA_Bonobo_Storage__epv *bonobo_storage_get_epv (void);


END_GNOME_DECLS

#endif /* _BONOBO_STORAGE_H_ */

