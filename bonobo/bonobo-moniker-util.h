/**
 * bonobo-moniker-util.h
 *
 * Copyright (C) 2000  Helix Code, Inc.
 *
 * Authors:
 *	Michael Meeks    (michael@helixcode.com)
 *	Ettore Perazzoli (ettore@helixcode.com)
 */

#ifndef _BONOBO_MONIKER_UTIL_H
#define _BONOBO_MONIKER_UTIL_H

#include <bonobo/bonobo-object-client.h>

/* Useful client functions */
Bonobo_Unknown      bonobo_get_object                            (const CORBA_char        *name,      
								  const char              *interface_name,
								  CORBA_Environment       *ev);


Bonobo_Moniker      bonobo_moniker_client_new_from_name          (const CORBA_char        *name,
								  CORBA_Environment       *ev);

CORBA_char         *bonobo_moniker_client_get_name               (Bonobo_Moniker     moniker,
								  CORBA_Environment *ev);

Bonobo_Unknown      bonobo_moniker_client_resolve_default        (Bonobo_Moniker     moniker,
								  const char        *interface_name,
								  CORBA_Environment *ev);
BonoboObjectClient *bonobo_moniker_client_resolve_client_default (Bonobo_Moniker     moniker,
								  const char        *interface_name,
								  CORBA_Environment *ev);
/* The same client interface, but asynchronous */
typedef struct _BonoboMonikerASyncHandle BonoboMonikerASyncHandle;

Bonobo_Moniker      bonobo_moniker_async_handle_get_moniker     (BonoboMonikerASyncHandle *handle);
Bonobo_Unknown      bonobo_moniker_async_handle_get_object      (BonoboMonikerASyncHandle *handle);
CORBA_Environment  *bonobo_moniker_async_handle_get_environment (BonoboMonikerASyncHandle *handle);
void                bonobo_moniker_async_handle_release         (BonoboMonikerASyncHandle *handle);

typedef void      (*BonoboMonikerCallback)                      (BonoboMonikerASyncHandle *handle,
								 gpointer                  user_data);

void                bonobo_moniker_client_new_from_name_async    (const CORBA_char        *name,
								  CORBA_Environment       *ev,
								  BonoboMonikerCallback    cb,
								  gpointer                 user_data);

void                bonobo_moniker_resolve_async                 (Bonobo_Moniker           moniker,
								  Bonobo_ResolveOptions   *options,
								  const char              *interface_name,
								  CORBA_Environment       *ev,
								  BonoboMonikerCallback    cb,
								  gpointer                 user_data);

void                bonobo_moniker_resolve_async_default         (Bonobo_Moniker           moniker,
								  const char              *interface_name,
								  CORBA_Environment       *ev,
								  BonoboMonikerCallback    cb,
								  gpointer                 user_data);

/* Useful moniker implementation helper functions */
Bonobo_Moniker bonobo_moniker_util_new_from_name_full   (Bonobo_Moniker     parent,
							 const CORBA_char  *name,
							 CORBA_Environment *ev);
CORBA_char    *bonobo_moniker_util_get_parent_name      (Bonobo_Moniker     moniker,
							 CORBA_Environment *ev);
Bonobo_Unknown bonobo_moniker_util_qi_return            (Bonobo_Unknown     object,
							 const CORBA_char  *requested_interface,
							 CORBA_Environment *ev);
int            bonobo_moniker_util_seek_std_separator   (const CORBA_char  *str,
							 int                min_idx);

#endif /* _BONOBO_MONIKER_UTIL_H */
