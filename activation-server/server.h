#ifndef OAFD_H
#define OAFD_H 1

/* od-corba.c */
OAF_ObjectDirectory
OAF_ObjectDirectory_create(PortableServer_POA poa,
			   const char *domain,
			   const char *hostname,
			   const char **source_directories,
			   CORBA_Environment *ev);

/* ac-corba.c */
OAF_ActivationContext
OAF_ActivationContext_create(PortableServer_POA poa,
			     CORBA_Environment *ev);

#endif
