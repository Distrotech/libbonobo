/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <liboaf/liboaf.h>
#include <stdio.h>
#include <stdlib.h>
#include "empty.h"

void
test_empty (CORBA_Object obj, CORBA_Environment *ev, const char *type)
{
	if (CORBA_Object_is_nil (obj, ev)) {
		g_warning ("Activation %s failed!", type);

	} else if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning ("Activation %s failed: %s\n", type,
			   CORBA_exception_id (ev));
	} else {
		Empty_doNothing (obj, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			g_warning ("Call failed: %s\n",
				   CORBA_exception_id (ev));
                else
                        fprintf (stderr, "Test %s succeeded\n", type);
	}
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

        test_empty (obj, &ev, "by query");

	obj = oaf_activate_from_id ("OAFIID:Empty:19991025", 0, NULL, &ev);

        test_empty (obj, &ev, "from id");

	obj = oaf_activate_from_id ("OAFAID:[OAFIID:Empty:19991025]", 0, NULL, &ev);

        test_empty (obj, &ev, "from aid");

	CORBA_exception_free (&ev);

	return 0;
}
