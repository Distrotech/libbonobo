/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * object-directory.h: Directory based object
 *
 * Author:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2003 Ximian, Inc.
 */
#ifndef _OBJECT_DIRECTORY_H_
#define _OBJECT_DIRECTORY_H_

#include <bonobo/bonobo-object.h>
#include <bonobo-activation/Bonobo_ObjectDirectory.h>

G_BEGIN_DECLS

typedef struct _ObjectDirectory        ObjectDirectory;
typedef struct _ObjectDirectoryPrivate ObjectDirectoryPrivate;

#define OBJECT_TYPE_DIRECTORY        (object_directory_get_type ())
#define OBJECT_DIRECTORY(o)          (G_TYPE_CHECK_INSTANCE_CAST ((bonobo_object (o)), OBJECT_TYPE_DIRECTORY, ObjectDirectory))
#define OBJECT_DIRECTORY_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), OBJECT_TYPE_DIRECTORY, ObjectDirectoryClass))
#define OBJECT_IS_DIRECTORY(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), OBJECT_TYPE_DIRECTORY))
#define OBJECT_IS_DIRECTORY_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), OBJECT_TYPE_DIRECTORY))

struct _ObjectDirectory {
	BonoboObject parent;

        /* Information on all servers */
	GHashTable           *by_iid;
	Bonobo_ServerInfoList attr_servers;
	Bonobo_CacheTime      time_list_changed;

        /* CORBA Object tracking */
	GHashTable      *active_server_lists;
	guint            n_active_servers;
        guint            no_servers_timeout;
	Bonobo_CacheTime time_active_changed;

        /* Source polling bits */
        char           **registry_source_directories;
        time_t           time_did_stat;
        GHashTable      *registry_directory_mtimes;
};

typedef struct {
	BonoboObjectClass parent_class;

	POA_Bonobo_ObjectDirectory__epv epv;
} ObjectDirectoryClass;

GType                  object_directory_get_type           (void) G_GNUC_CONST;
void                   bonobo_object_directory_init        (PortableServer_POA     poa,
                                                            const char            *source_directory,
                                                            CORBA_Environment     *ev);
void                   bonobo_object_directory_shutdown    (PortableServer_POA     poa,
                                                            CORBA_Environment     *ev);
Bonobo_ObjectDirectory bonobo_object_directory_get         (void);
CORBA_Object           bonobo_object_directory_re_check_fn (const Bonobo_ActivationEnvironment *environment,
                                                            const char                         *od_iorstr,
                                                            gpointer                            user_data,
                                                            CORBA_Environment                  *ev);
void                   bonobo_object_directory_reload      (void);
void                   reload_object_directory             (void);
void                   check_quit                          (void);

G_END_DECLS

#endif /* _OBJECT_DIRECTORY_H_ */