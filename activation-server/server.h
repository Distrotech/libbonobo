/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef SERVER_H
#define SERVER_H

#include <bonobo-activation/bonobo-activation.h>
#include "bonobo-activation/Bonobo_ActivationContext.h"

/*
 *    Time delay after all servers are de-registered / dead
 * before quitting the server. (ms)
 */
#define SERVER_IDLE_QUIT_TIMEOUT 1000

#define NAMING_CONTEXT_IID "OAFIID:Bonobo_CosNaming_NamingContext"

/* object-directory-corba.c */
void                   bonobo_object_directory_init        (PortableServer_POA     poa,
                                                            const char            *domain,
                                                            const char            *source_directory,
                                                            CORBA_Environment     *ev);
void                   bonobo_object_directory_shutdown    (PortableServer_POA     poa,
                                                            CORBA_Environment     *ev);
Bonobo_ObjectDirectory bonobo_object_directory_get         (void);
CORBA_Object           bonobo_object_directory_re_check_fn (const char            *display,
                                                            const char            *od_iorstr,
                                                            gpointer               user_data,
                                                            CORBA_Environment     *ev);
void                   bonobo_object_directory_reload      (void);
void                   reload_object_directory             (void);

/* object-directory-load.c */
void                   bonobo_server_info_load             (char                 **dirs,
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
void                     activation_context_init         (PortableServer_POA     poa,
                                                          Bonobo_ObjectDirectory dir,
                                                          CORBA_Environment     *ev);
void                     activation_context_shutdown     (PortableServer_POA     poa,
                                                          CORBA_Environment     *ev);
Bonobo_ActivationContext activation_context_get          (void);

void                     activation_clients_cache_notify (void);
void                     activation_clients_check        (void);

void                     add_initial_locales             (void);

gboolean                 is_locale_interesting           (const char *locale);

#endif /* SERVER_H */
