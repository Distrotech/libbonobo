/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <liboaf/liboaf.h>
#include <stdlib.h>
#include "empty.h"

int
main (int argc, char *argv[])
{
	CORBA_Object obj;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	oaf_init (argc, argv);

//      putenv("OAF_BARRIER_INIT=1");
	obj =
		oaf_activate ("repo_ids.has('IDL:Empty:1.0')", NULL, 0, NULL,
			      &ev);

	if (CORBA_Object_is_nil (obj, &ev)) {
		g_warning ("Activation failed!");
	} else if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Activation failed: %s\n",
			   CORBA_exception_id (&ev));
	} else {
		Empty_doNothing (obj, &ev);
		if (ev._major != CORBA_NO_EXCEPTION)
			g_warning ("Call failed: %s\n",
				   CORBA_exception_id (&ev));
	}

	CORBA_exception_free (&ev);

	return 0;
}
