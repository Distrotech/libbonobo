/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef SERVER_H
#define SERVER_H

#include "Bonobo_ActivationContext.h"
#include <bonobo-activation/bonobo-activation.h>

/* object-directory-corba.c */
Bonobo_ObjectDirectory   Bonobo_ObjectDirectory_create   (PortableServer_POA     poa,
                                                          const char            *domain,
                                                          const char            *source_directory,
                                                          CORBA_Environment     *ev);
CORBA_Object         bonobo_object_directory_re_check_fn (const char            *display,
                                                          const char            *od_iorstr,
                                                          gpointer               user_data,
                                                          CORBA_Environment     *ev);
void                     reload_object_directory         (void);

/* object-directory-load.c */
void                     Bonobo_ServerInfo_load          (char                 **dirs,
                                                          Bonobo_ServerInfoList *servers,
                                                          GHashTable           **by_iid,
                                                          const char            *host, 
                                                          const char            *domain);

/* od-activate.c */
typedef struct {
	Bonobo_ActivationContext ac;
	Bonobo_ActivationFlags flags;
	CORBA_Context ctx;
} ODActivationInfo;

/* object-directory-activate.c */
CORBA_Object             od_server_activate              (Bonobo_ServerInfo *si,
                                                          ODActivationInfo  *actinfo,
                                                          CORBA_Object       od_obj,
                                                          CORBA_Environment *ev);

/* activation-context-corba.c */
Bonobo_ActivationContext Bonobo_ActivationContext_create (PortableServer_POA poa,
                                                          CORBA_Environment *ev);

void                     notify_clients_cache_reset      (void);

void                     add_initial_locales             (void);

gboolean                 is_locale_interesting           (const char *locale);

#endif /* SERVER_H */
