/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Bonobo Main
 *
 * Author:
 *    Miguel de Icaza (miguel@gnu.org)
 *    Nat Friedman (nat@gnome-support.com)
 *
 * Copyright 1999 International GNOME Support (http://www.gnome-support.com)
 */
#include <config.h>
#include <bonobo/gnome-main.h>
#include <libgnorba/gnorba.h>

#include <X11/Xlib.h>

CORBA_ORB                 __bonobo_orb;
PortableServer_POA        __bonobo_poa;
PortableServer_POAManager __bonobo_poa_manager;

static int
bonobo_x_error_handler (Display *display, XErrorEvent *error)
{
	char buf [64];

	if (!error->error_code)
		return 0;

	/*
	 * If we got a Bad Drawable, we ignore it for now.  FIXME: We
	 * need to somehow distinguish real errors from
	 * X-server-induced errors.  Keeping a list of windows for
	 * which we will ignore BadDrawables would be a good idea.
	 */
	if (error->error_code == BadDrawable)
		return 0;

	/*
	 * If it wasn't a BadDrawable error, we abort.
	 */

	XGetErrorText (display, error->error_code, buf, 63);

	g_error ("%s\n  serial %ld error_code %d request_code %da minor_code %d\n",
		 buf, error->serial, error->error_code, error->request_code,
		 error->minor_code);

	return 0;
}

/**
 * bonobo_setup_x_error_handler:
 *
 * To do graphical embedding in the X window system, Bonobo
 * uses the classic foreign-window-reparenting trick.  The
 * GtkPlug/GtkSocket widgets are used for thise purpose.  However,
 * serious robustness problems arise if the GtkSocket end of the
 * connection unexpectedly dies.  The X server sends out DestroyNotify
 * events for the descendents of the GtkPlug (i.e., your embedded
 * component's windows) in effectively random order.  Furthermore, if
 * you happened to be drawing on any of those windows when the
 * GtkSocket was destroyed (a common state of affairs), an X error
 * will kill your application.
 *
 * To solve this latter problem, Bonobo sets up its own X error
 * handler which ignores certain X errors that might have been
 * caused by such a scenario.  Other X errors get passed to gdk_x_error
 * normally.
 */
void
bonobo_setup_x_error_handler (void)
{
	static gboolean error_handler_setup = FALSE;

	if (error_handler_setup)
		return;

	error_handler_setup = TRUE;

	XSetErrorHandler (bonobo_x_error_handler);
}

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
	
	if (CORBA_Object_is_nil ((CORBA_Object)poa, &ev)){
		poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
		if (ev._major != CORBA_NO_EXCEPTION){
			g_warning ("Can not resolve initial reference to RootPOA");
			CORBA_exception_free (&ev);
			return FALSE;
		}
		
	}

	if (CORBA_Object_is_nil ((CORBA_Object)manager, &ev)){
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


