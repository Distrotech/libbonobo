#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <libbonobo.h>
#include <orbit/poa/poa.h>

static int
ret_ex_test (CORBA_Environment *ev)
{
	BONOBO_RET_VAL_EX (ev, 1);

	return 0;
}

static void
ex_test (CORBA_Environment *ev)
{
	BONOBO_RET_EX (ev);
}

int
main (int argc, char *argv [])
{
	BonoboObject     *object;
	Bonobo_Unknown    ref;
	CORBA_Environment *ev, real_ev;

	free (malloc (8));

	if (bonobo_init (&argc, argv) == FALSE)
		g_error ("Can not bonobo_init");
	bonobo_activate ();

	ev = &real_ev;
	CORBA_exception_init (ev);

	fprintf (stderr, "Local lifecycle\n");
	{
		object = BONOBO_OBJECT (g_object_new (
			bonobo_moniker_get_type (), NULL));
	
		g_assert (bonobo_object (object) == object);
		g_assert (bonobo_object (&object->servant) == object);

		bonobo_object_unref (BONOBO_OBJECT (object));
	}

	fprintf (stderr, "In-proc lifecycle\n");
	{
		object = BONOBO_OBJECT (g_object_new (
			bonobo_moniker_get_type (), NULL));

		ref = CORBA_Object_duplicate (BONOBO_OBJREF (object), NULL);

		bonobo_object_release_unref (ref, NULL);
	}

	fprintf (stderr, "Out of proc lifecycle\n");
	{
		object = BONOBO_OBJECT (g_object_new (
			bonobo_moniker_get_type (), NULL));

		ref = CORBA_Object_duplicate (BONOBO_OBJREF (object), NULL);

		ORBit_small_handle_request (
			ORBIT_STUB_GetPoaObj (BONOBO_OBJREF (object)),
			"unref", NULL, NULL, NULL, NULL, ev);
		g_assert (!BONOBO_EX (ev));

		CORBA_Object_release (ref, ev);
		g_assert (!BONOBO_EX (ev));
	}

	fprintf (stderr, "Ret-ex tests...\n");

	g_assert (!ret_ex_test (ev));
	ex_test (ev);

	CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			     ex_Bonobo_PropertyBag_NotFound, NULL);
	g_assert (ret_ex_test (ev));

	CORBA_exception_free (ev);

	fprintf (stderr, "All tests passed\n");

	return 0;
}
