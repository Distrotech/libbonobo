#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libbonobo.h>

#define NUM_THREADS 8
#define NUM_GETS    8

static GThread *main_thread = NULL;

static void
get_fn (BonoboPropertyBag *bag,
	BonoboArg         *arg,
	guint              arg_id,
	CORBA_Environment *ev,
	gpointer           user_data)
{
	g_assert (g_thread_self () == main_thread);
	fprintf (stderr, "Check property\n");
	BONOBO_ARG_SET_BOOLEAN (arg, TRUE);
}

static void
test_prop (Bonobo_PropertyBag pb,
	   CORBA_Environment *ev)
{
	g_assert (bonobo_pbclient_get_boolean (pb, "bprop", ev) == TRUE);
	g_assert (!BONOBO_EX (ev));
}

static gpointer
test_thread (gpointer data)
{
	int i;
	CORBA_Environment ev[1];
	Bonobo_PropertyBag pb = data;

	CORBA_exception_init (ev);

	for (i = 0; i < NUM_GETS; i++)
		test_prop (pb, ev);

	CORBA_exception_free (ev);

	return data;
}

int
main (int argc, char *argv [])
{
	CORBA_ORB          orb;
	CORBA_Environment  ev[1];
	BonoboPropertyBag *pb;

	free (malloc (8));

	CORBA_exception_init (ev);

	orb = CORBA_ORB_init (&argc, argv,
			      "orbit-local-mt-orb", ev);

	if (bonobo_init (&argc, argv) == FALSE)
		g_error ("Can not bonobo_init");
	bonobo_activate ();

	pb = bonobo_property_bag_new (get_fn, NULL, NULL);
	bonobo_property_bag_add (pb, "bprop", 0, TC_CORBA_boolean,
				 NULL, "bprop", BONOBO_PROPERTY_READABLE);
	main_thread = g_thread_self ();

	test_prop (BONOBO_OBJREF (pb), ev);

	{
		int i;
		GThread *threads [NUM_THREADS];

		for (i = 0; i < NUM_THREADS; i++)
			threads [i] = g_thread_create
				( test_thread, BONOBO_OBJREF (pb), TRUE, NULL);
		
		for (i = 0; i < NUM_THREADS; i++) {
			if (!(g_thread_join (threads [i]) == BONOBO_OBJREF (pb)))
				g_error ("Wierd thread join problem '%d'", i);
		}
	}

	bonobo_object_unref (pb);

	CORBA_Object_release ((CORBA_Object) orb, ev);

	CORBA_exception_free (ev);

	return bonobo_debug_shutdown ();
}
