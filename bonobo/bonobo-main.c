/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo Main
 *
 * Author:
 *    Miguel de Icaza (miguel@gnu.org)
 */
#include <config.h>
#include <bonobo/gnome-main.h>
#include <libgnorba/gnorba.h>

CORBA_ORB                 __bonobo_orb;
PortableServer_POA        __bonobo_poa;
PortableServer_POAManager __bonobo_poa_manager;

/**
 * bonobo_init:
 * @orb: the ORB in which we run
 * @poa: optional, a POA.
 * @manager: optional, a POA Manager
 *
 * Initializes the bonobo document model.  It requires at least
 * the value for @orb.  If @poa is CORBA_OBJECT_NIL, then the
 * RootPOA will be used, in this case @manager should be CORBA_OBJECT_NIL.
 *
 * Returns %TRUE on success, or %FALSE on failure.
 */
gboolean
bonobo_init (CORBA_ORB orb, PortableServer_POA poa, PortableServer_POAManager manager)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	if (orb == CORBA_OBJECT_NIL)
		orb = gnome_CORBA_ORB();
	
	if (CORBA_Object_is_nil((CORBA_Object)poa, &ev)){
		poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
		if (ev._major != CORBA_NO_EXCEPTION){
			g_warning ("Can not resolve initial reference to RootPOA");
			CORBA_exception_free (&ev);
			return FALSE;
		}
		
	}

	if (CORBA_Object_is_nil((CORBA_Object)manager, &ev)){
		manager = PortableServer_POA__get_the_POAManager (poa, &ev);
		if (ev._major != CORBA_NO_EXCEPTION){
			g_warning ("Can not get the POA manager");
			CORBA_exception_free (&ev);
			return FALSE;
		}
	}
	
	PortableServer_POAManager_activate (manager, &ev);
	__bonobo_orb = orb;
	__bonobo_poa = poa;
	__bonobo_poa_manager = manager;

	return TRUE;
}


