/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef SERVER_H
#define SERVER_H

#include "Bonobo_ActivationContext.h"
#include <bonobo-activation/bonobo-activation.h>

/* od-corba.c */
Bonobo_ObjectDirectory Bonobo_ObjectDirectory_create (PortableServer_POA poa,
                                                const char *domain,
                                                const char *source_directory,
                                                CORBA_Environment * ev);
/* od-load.c */

void Bonobo_ServerInfo_load (char                **dirs,
                          Bonobo_ServerInfoList   *servers,
                          GHashTable          **by_iid,
                          const char           *host, 
                          const char           *domain);

/* od-activate.c */
typedef struct
{
	Bonobo_ActivationContext ac;
	Bonobo_ActivationFlags flags;
	CORBA_Context ctx;
}
ODActivationInfo;

CORBA_Object od_server_activate (Bonobo_ServerInfo * si,
				 ODActivationInfo * actinfo,
				 CORBA_Object od_obj, CORBA_Environment * ev);

/* ac-corba.c */
Bonobo_ActivationContext
Bonobo_ActivationContext_create (PortableServer_POA poa, CORBA_Environment * ev);

#endif /* SERVER_H */
