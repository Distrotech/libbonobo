#include <config.h>
#include <stdlib.h>
#include <libbonobo.h>
#include <orbit/poa/poa.h>

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

	/* Local lifecycle */
	{
		object = BONOBO_OBJECT (g_object_new (
			bonobo_moniker_get_type (), NULL));
	
		g_assert (bonobo_object (object) == object);
		g_assert (bonobo_object (&object->servant) == object);

		bonobo_object_unref (BONOBO_OBJECT (object));
	}

	/* In-proc lifecycle */
	{
		object = BONOBO_OBJECT (g_object_new (
			bonobo_moniker_get_type (), NULL));

		ref = CORBA_Object_duplicate (BONOBO_OBJREF (object), NULL);

		bonobo_object_release_unref (ref, NULL);
	}

	/* Out-of-proc lifecycle */
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

	CORBA_exception_free (ev);

	return 0;
}
