/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <liboaf/liboaf.h>
#include <stdio.h>
#include <stdlib.h>
#include "empty.h"

CORBA_Object name_service = CORBA_OBJECT_NIL;

static char *
oaf_exception_id (CORBA_Environment *ev)
{
        if (ev->_major == CORBA_USER_EXCEPTION) {
                if (!strcmp (ev->_repo_id, "IDL:OAF/GeneralError:1.0")) {
                        OAF_GeneralError *err = ev->_params;
                        
                        if (!err || !err->description)
                                return "No general exception error message";
                        else
                                return err->description;
                } else
                        return ev->_repo_id;
        } else
                return CORBA_exception_id (ev);
}

static gboolean
test_oafd (CORBA_Environment *ev, const char *type)
{
        CORBA_Object ns;

        ns = oaf_name_service_get (ev);
        if (ev->_major != CORBA_NO_EXCEPTION) {
                g_warning ("Exception '%s' finding oafd %s",
                           oaf_exception_id (ev), type);
                return FALSE;
        }

        if (name_service != CORBA_OBJECT_NIL &&
            name_service != ns) {
                g_warning ("oafd crashed %s", type);
                return FALSE;
        }

        name_service = ns;

        return TRUE;
}

static gboolean
test_object (CORBA_Object obj, CORBA_Environment *ev, const char *type)
{
	if (CORBA_Object_is_nil (obj, ev)) {
		g_warning ("Activation %s failed!", type);

	} else if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning ("Activation %s failed: %s\n", type,
			   oaf_exception_id (ev));
	} else
                return TRUE;

        if (!test_oafd (ev, type))
                return FALSE;

        return FALSE;
}

static void
test_empty (CORBA_Object obj, CORBA_Environment *ev, const char *type)
{
        Empty_doNothing (obj, ev);

        if (ev->_major != CORBA_NO_EXCEPTION)
                g_warning ("Call failed: %s\n",
                           oaf_exception_id (ev));
        else
                fprintf (stderr, "Test %s succeeded\n", type);
}

int
main (int argc, char *argv[])
{
	CORBA_Object obj;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	oaf_init (argc, argv);

/*      putenv("OAF_BARRIER_INIT=1"); */

	obj = oaf_activate ("repo_ids.has('IDL:Empty:1.0')", NULL, 0, NULL,
                            &ev);
        if (test_object (obj, &ev, "by query"))
                test_empty (obj, &ev, "by query");


	obj = oaf_activate_from_id ("OAFIID:Empty:19991025", 0, NULL, &ev);
        if (test_object (obj, &ev, "from id"))
                test_empty (obj, &ev, "from id");


	obj = oaf_activate_from_id ("OAFAID:[OAFIID:Empty:19991025]", 0, NULL, &ev);
        if (test_object (obj, &ev, "from aid"))
                test_empty (obj, &ev, "from aid");


        fprintf (stderr, "Broken link test ");
        obj = oaf_activate_from_id ("OAFIID:Bogus:20000526", 0, NULL, &ev);
        if (obj || ev._major == CORBA_NO_EXCEPTION)
                fprintf (stderr, "failed 1");
        else {
                fprintf (stderr, "passed 1");
                CORBA_exception_free (&ev);
        }
        if (test_oafd (&ev, "with broken factory link"))
                fprintf (stderr, ", passed 2");
        else
                fprintf (stderr, ", failed 2");
        fprintf (stderr, "\n");

	CORBA_exception_free (&ev);

	return 0;
}
