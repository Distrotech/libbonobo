/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "config.h"

#include "liboaf/liboaf-private.h"
#include <stdio.h>

OAF_RegistrationResult
oaf_active_server_register (const char *iid, CORBA_Object obj)
{
	OAF_ObjectDirectory od;
	OAFRegistrationCategory regcat = { "IDL:OAF/ObjectDirectory:1.0" };
	OAFRegistrationCategory ac_regcat;
	CORBA_Environment ev;
	OAF_RegistrationResult retval;
	const char *actid;
	static gboolean need_printout = TRUE;

	CORBA_exception_init (&ev);

	actid = oaf_activation_iid_get ();

	if (actid && !strcmp (actid, iid) && need_printout) {
		char *iorstr;
		FILE *fh;
		int iorfd = oaf_ior_fd_get ();

		need_printout = FALSE;

		if (iorfd == 1)
			fh = stdout;
		else {
			fh = fdopen (iorfd, "w");
			if (!fh)
				fh = stdout;
		}

		iorstr =
			CORBA_ORB_object_to_string (oaf_orb_get (), obj, &ev);
		if (ev._major == CORBA_NO_EXCEPTION) {
			fprintf (fh, "%s\n", iorstr);
			CORBA_free (iorstr);
		}

		if (fh != stdout)
			fclose (fh);
		else if (iorfd > 2)
			close (iorfd);
	}
#ifdef OAF_DEBUG
        else if (actid && need_printout) {
                g_message ("Unusual '%s' was activated, but "
                           "'%s' is needed", iid, actid);
        }
#endif

        if (actid && !strcmp(actid, iid) && oaf_private)
                return OAF_REG_SUCCESS;

	regcat.session_name = oaf_session_name_get ();
	regcat.username = oaf_username_get ();
	regcat.hostname = oaf_hostname_get ();

	od = oaf_service_get (&regcat);

	if (CORBA_Object_is_nil (od, &ev)) {
		CORBA_Object ac;

		/* If we can't get an object directory, get an activation context (in case oafd needs starting)
		 * and then try again */
		ac_regcat = regcat;
		ac_regcat.name = "IDL:OAF/ActivationContext:1.0";
		ac = oaf_service_get (&ac_regcat);
		if (CORBA_Object_is_nil (ac, &ev))
			return OAF_REG_ERROR;
		od = oaf_service_get (&regcat);
		if (CORBA_Object_is_nil (od, &ev))
			return OAF_REG_ERROR;
	}

	retval =
		OAF_ObjectDirectory_register_new (od, (char *) iid, obj, &ev);
	CORBA_exception_free (&ev);

	return retval;
}

void
oaf_active_server_unregister (const char *iid, CORBA_Object obj)
{
	OAF_ObjectDirectory od;
	OAFRegistrationCategory regcat = { "IDL:OAF/ObjectDirectory:1.0" };
	CORBA_Environment ev;
	const char *actid;

	actid = oaf_activation_iid_get();
	if(actid && !strcmp(actid, iid) && oaf_private)
		return;

	regcat.session_name = oaf_session_name_get ();
	regcat.username = oaf_username_get ();
	regcat.hostname = oaf_hostname_get ();

	od = oaf_service_get (&regcat);

	CORBA_exception_init (&ev);
	if (CORBA_Object_is_nil (od, &ev))
		return;

	OAF_ObjectDirectory_unregister (od, (char *) iid, obj, &ev);
	CORBA_exception_free (&ev);
}
