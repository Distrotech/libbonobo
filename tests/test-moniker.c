#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>

#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <orb/orbit.h>
#include <bonobo/bonobo.h>

static void
check_string (const char *escaped, const char *unescaped)
{
	BonoboMoniker *moniker;
	char          *str;
	const char    *cstr;

	moniker = gtk_type_new (bonobo_moniker_get_type ());

	bonobo_moniker_set_name (moniker, escaped, strlen (escaped));
	cstr = bonobo_moniker_get_name (moniker, 0);
	fprintf (stderr, "'%s' == '%s'\n", unescaped, cstr);
	g_assert (!strcmp (cstr, unescaped));

	str = bonobo_moniker_get_name_escaped (moniker, 0);
	fprintf (stderr, "'%s' == '%s'\n", str, escaped);
	g_assert (!strcmp (str, escaped));
	g_free (str);

	bonobo_object_unref (BONOBO_OBJECT (moniker));
}

int
main (int argc, char *argv [])
{
	CORBA_Environment real_ev, *ev;
	CORBA_ORB    orb;

	free (malloc (8));

	ev = &real_ev;
	CORBA_exception_init (ev);

	gtk_type_init ();

	orb = oaf_init (argc, argv);
	
	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Can not bonobo_init");

	check_string ("a:\\\\", "a:\\");

	check_string ("a:\\#", "a:#");

	check_string ("a:\\!", "a:!");

	check_string ("a:1\\!\\#\\!\\!\\#\\\\",
		      "a:1!#!!#\\");

	return 0;
}
