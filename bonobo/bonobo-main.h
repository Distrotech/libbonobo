/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef __GNOME_MAIN_H__
#define __GNOME_MAIN_H__ 1

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>

gboolean   bonobo_init      (CORBA_ORB orb,
			     PortableServer_POA poa,
			     PortableServer_POAManager manager);

extern CORBA_ORB                 __bonobo_orb;
extern PortableServer_POA        __bonobo_poa;
extern PortableServer_POAManager __bonobo_poa_manager;

#define bonobo_orb() __bonobo_orb
#define bonobo_poa() __bonobo_poa
#define bonobo_poa_manager() __bonobo_poa_manager

#endif  __GNOME_MAIN_H__
