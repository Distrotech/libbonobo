/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-main.h: Bonobo Main
 *
 * Author:
 *    Miguel de Icaza  (miguel@gnu.org)
 *    Nat Friedman     (nat@nat.org)
 *    Peter Wainwright (prw@wainpr.demo.co.uk)
 *
 * Copyright 1999 Helix Code, Inc.
 */

#ifndef __LIBBONOBO_MAIN_H__
#define __LIBBONOBO_MAIN_H__

#include <glib-object.h>
#include <bonobo/Bonobo.h>

typedef enum {
        BONOBO_POA_ALL_AT_IDLE,
        BONOBO_POA_ONEWAY_AT_IDLE,
        BONOBO_POA_THREAD_PER_REQUEST
} BonoboThreadHint;

G_BEGIN_DECLS

gboolean                    bonobo_is_initialized        (void);
gboolean		    bonobo_init			 (int *argc,
							  char **argv);
gboolean                    bonobo_init_full             (int *argc,
							  char **argv,
							  CORBA_ORB orb,
							  PortableServer_POA poa,
							  PortableServer_POAManager manager);
int                         bonobo_debug_shutdown        (void);
void			    bonobo_main			 (void);
void                        bonobo_main_quit             (void);

gboolean		    bonobo_activate		 (void);
void			    bonobo_setup_x_error_handler (void);

CORBA_ORB		    bonobo_orb			 (void);
PortableServer_POA	    bonobo_poa			 (void);
PortableServer_POAManager   bonobo_poa_manager		 (void);
PortableServer_POA          bonobo_poa_get_threaded      (BonoboThreadHint hint);

G_END_DECLS

#endif /* __LIBBONOBO_MAIN_H__ */
