#ifndef OAFD_H
#define OAFD_H 1

#include "oaf.h"

#ifdef g_alloca
#define oaf_alloca g_alloca
#else
#define oaf_alloca alloca
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#endif

/* od-corba.c */
OAF_ObjectDirectory
OAF_ObjectDirectory_create(PortableServer_POA poa,
			   const char *domain,
			   const char *source_directory,
			   CORBA_Environment *ev);
/* od-load.c */
OAF_ServerInfo *OAF_ServerInfo_load(const char *source_directory, CORBA_unsigned_long *nservers, GHashTable **by_iid);

/* od-activate.c */
CORBA_Object od_server_activate(OAF_ServerInfo *si);

/* ac-corba.c */
OAF_ActivationContext
OAF_ActivationContext_create(PortableServer_POA poa,
			     CORBA_Environment *ev);

#endif
