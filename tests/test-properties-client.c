#include <config.h>
#include <stdio.h>
#include <libbonobo.h>

CORBA_ORB		 orb;
Bonobo_PropertyBag	 pb;
CORBA_Environment	 ev;
Bonobo_PropertyBag       pb;

static char *
simple_print_type (CORBA_TypeCode tc)
{
	static char s[1024];

	switch (tc->kind) {
	case CORBA_tk_boolean:
		g_snprintf (s, sizeof (s), "boolean");
		break;
	case CORBA_tk_short:
		g_snprintf (s, sizeof (s), "short");
		break;
	case CORBA_tk_ushort:
		g_snprintf (s, sizeof (s), "ushort");
		break;
	case CORBA_tk_long:
		g_snprintf (s, sizeof (s), "long");
		break;
	case CORBA_tk_ulong:
		g_snprintf (s, sizeof (s), "ulong");
		break;
	case CORBA_tk_float:
		g_snprintf (s, sizeof (s), "float");
		break;
	case CORBA_tk_double:
		g_snprintf (s, sizeof (s), "double");
		break;
	case CORBA_tk_string:
		g_snprintf (s, sizeof (s), "string");
		break;
	default:
		g_snprintf (s, sizeof (s), "Unknown");
		break;
	}

	return s;
}

static char *
simple_print_value (char *name, CORBA_TypeCode tc)
{
	static char s[1024];

	switch (tc->kind) {
	case CORBA_tk_boolean:
		g_snprintf (s, sizeof (s), "%s",
		    bonobo_pbclient_get_boolean (pb, name, NULL) ?
			"True" : "False");
		break;
	case CORBA_tk_long:
		g_snprintf (s, sizeof (s), "%d",
		    bonobo_pbclient_get_long (pb, name, NULL));
		break;
	case CORBA_tk_float:
		g_snprintf (s, sizeof (s), "%f",
		    bonobo_pbclient_get_float (pb, name, NULL));
		break;
	case CORBA_tk_double:
		g_snprintf (s, sizeof (s), "%f",
		    bonobo_pbclient_get_double (pb, name, NULL));
		break;
	case CORBA_tk_string:
		g_snprintf (s, sizeof (s), "%s",
		    bonobo_pbclient_get_string (pb, name, NULL));
		break;
	default:
		g_snprintf (s, sizeof (s), "Unknown");
		break;
	}

	return s;
}


static char *
simple_print_default_value (char *name, CORBA_TypeCode tc)
{
	static char s[1024];

	switch (tc->kind) {
	case CORBA_tk_boolean:
		g_snprintf (s, sizeof (s), "%s",
		    bonobo_pbclient_get_default_boolean (pb, name, NULL) ?
			"True" : "False");
		break;
	case CORBA_tk_long:
		g_snprintf (s, sizeof (s), "%d",
		    bonobo_pbclient_get_default_long (pb, name, NULL));
		break;
	case CORBA_tk_float:
		g_snprintf (s, sizeof (s), "%f",
		    bonobo_pbclient_get_default_float (pb, name, NULL));
		break;
	case CORBA_tk_double:
		g_snprintf (s, sizeof (s), "%f",
		    bonobo_pbclient_get_default_double (pb, name, NULL));
		break;
	case CORBA_tk_string:
		g_snprintf (s, sizeof (s), "%s",
		    bonobo_pbclient_get_default_string (pb, name, NULL));
		break;
	default:
		g_snprintf (s, sizeof (s), "Unknown");
		break;
	}

	return s;
}


static char *
simple_print_read_only (char *name)
{
	Bonobo_PropertyFlags flags;

	flags = bonobo_pbclient_get_flags (pb, name, NULL);

	return (flags & Bonobo_PROPERTY_READABLE) ?
		"ReadOnly" : "ReadWrite";
}

static void
print_props (void)
{
	GList *props;
	GList *l;

	props = bonobo_pbclient_get_keys (pb, NULL);

	for (l = props; l != NULL; l = l->next) {
		CORBA_TypeCode tc;
		char *name = l->data;

		tc = bonobo_pbclient_get_type (pb, name, NULL);

		g_print ("%s [%s] %s %s %s\n",
			 name,
			 simple_print_type (tc),
			 simple_print_value (name, tc),
			 simple_print_default_value (name, tc),
			 simple_print_read_only (name));
	}

	bonobo_pbclient_free_keys (props);
}


static guint
create_bag_client (void)
{
	print_props ();

	bonobo_pbclient_set_boolean (pb, "boolean-test", FALSE, NULL);
	bonobo_pbclient_set_long    (pb, "long-test", 3, NULL);
	bonobo_pbclient_set_float   (pb, "float-test", 0.00001, NULL);
	bonobo_pbclient_set_double  (pb, "double-test", 2.0001, NULL);
	bonobo_pbclient_set_string  (pb, "string-test",
				     "life is a wonderful gift", NULL);

	bonobo_pbclient_set (
		pb, NULL,
		"boolean-test", TC_CORBA_boolean, FALSE,
		"long-test", TC_CORBA_long, 3,
		"float-test", TC_CORBA_float, 0.00001,
		"double-test", TC_CORBA_double, 2.0001,
		"string-test", TC_CORBA_string, "you are a precious flower",
		NULL);

	bonobo_object_release_unref (pb, NULL);

	bonobo_main_quit ();

	return FALSE;
}


int
main (int argc, char **argv)
{
	if (argc < 2) {
		fprintf (stderr, "Usage: %s <IOR of PropertyBag server>\n", *argv);
		return 1;
	}

	CORBA_exception_init (&ev);

	if (!bonobo_init (&argc, argv))
		g_error ("Could not initialize Bonobo");

	pb = CORBA_ORB_string_to_object (orb, argv [1], &ev);

	if (pb == CORBA_OBJECT_NIL) {
		g_error ("Could not bind to PropertyBag object");
		return 1;
	}

	g_idle_add ((GSourceFunc) create_bag_client, NULL);

	bonobo_main ();

	return 0;
}
