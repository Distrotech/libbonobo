
#include <config.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-bonobo.h>
#include <stdio.h>

CORBA_ORB		orb;
GNOME_PropertyBag	pb;
CORBA_Environment	ev;
GnomePropertyBagClient *pbc;

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
		    gnome_property_bag_client_get_value_boolean (pbc, name) ?
			"True" : "False");
		break;
	case CORBA_tk_short:
		g_snprintf (s, sizeof (s), "%d",
		    gnome_property_bag_client_get_value_short (pbc, name));
		break;
	case CORBA_tk_ushort:
		g_snprintf (s, sizeof (s), "%d",
		    gnome_property_bag_client_get_value_ushort (pbc, name));
		break;
	case CORBA_tk_long:
		g_snprintf (s, sizeof (s), "%ld",
		    gnome_property_bag_client_get_value_long (pbc, name));
		break;
	case CORBA_tk_ulong:
		g_snprintf (s, sizeof (s), "%ld",
		    gnome_property_bag_client_get_value_ulong (pbc, name));
		break;
	case CORBA_tk_float:
		g_snprintf (s, sizeof (s), "%f",
		    gnome_property_bag_client_get_value_float (pbc, name));
		break;
	case CORBA_tk_double:
		g_snprintf (s, sizeof (s), "%f",
		    gnome_property_bag_client_get_value_double (pbc, name));
		break;
	case CORBA_tk_string:
		g_snprintf (s, sizeof (s), "%s",
		    gnome_property_bag_client_get_value_string (pbc, name));
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
		    gnome_property_bag_client_get_default_boolean (pbc, name) ?
			"True" : "False");
		break;
	case CORBA_tk_short:
		g_snprintf (s, sizeof (s), "%d",
		    gnome_property_bag_client_get_default_short (pbc, name));
		break;
	case CORBA_tk_ushort:
		g_snprintf (s, sizeof (s), "%d",
		    gnome_property_bag_client_get_default_ushort (pbc, name));
		break;
	case CORBA_tk_long:
		g_snprintf (s, sizeof (s), "%ld",
		    gnome_property_bag_client_get_default_long (pbc, name));
		break;
	case CORBA_tk_ulong:
		g_snprintf (s, sizeof (s), "%ld",
		    gnome_property_bag_client_get_default_ulong (pbc, name));
		break;
	case CORBA_tk_float:
		g_snprintf (s, sizeof (s), "%f",
		    gnome_property_bag_client_get_default_float (pbc, name));
		break;
	case CORBA_tk_double:
		g_snprintf (s, sizeof (s), "%f",
		    gnome_property_bag_client_get_default_double (pbc, name));
		break;
	case CORBA_tk_string:
		g_snprintf (s, sizeof (s), "%s",
		    gnome_property_bag_client_get_default_string (pbc, name));
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
	GnomePropertyFlags flags;

	flags = gnome_property_bag_client_get_flags (pbc, name);

	return (flags & GNOME_PROPERTY_READ_ONLY) ?
		"ReadOnly" : "ReadWrite";
}

static void
print_props (void)
{
	GList *props;
	GList *l;

	props = gnome_property_bag_client_get_property_names (pbc);

	for (l = props; l != NULL; l = l->next) {
		CORBA_TypeCode tc;
		char *name = l->data;

		tc = gnome_property_bag_client_get_property_type (pbc, name);

		g_print ("%s [%s] %s %s %s\n",
			 name,
			 simple_print_type (tc),
			 simple_print_value (name, tc),
			 simple_print_default_value (name, tc),
			 simple_print_read_only (name));
	}

	g_list_free (props);
}

static guint
create_bag_client (void)
{
	pbc = gnome_property_bag_client_new (pb);

	if (pbc == NULL) {
		g_error ("Could not create PropertyBagClient!\n");
		exit (1);
	}

	print_props ();

	gnome_property_bag_client_set_value_boolean (pbc, "boolean-test", FALSE);
	gnome_property_bag_client_set_value_short (pbc, "short-test", (gshort) 3);
	gnome_property_bag_client_set_value_string (pbc, "string-test", "life is pain");

	exit (0);

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

	gnome_CORBA_init_with_popt_table (
		"test property client", "0.0", &argc, argv,
		NULL, 0, NULL, 0, &ev);

	orb = gnome_CORBA_ORB ();

	if (! bonobo_init (orb, NULL, NULL))
		g_error ("Could not initialize Bonobo");

	pb = CORBA_ORB_string_to_object (orb, argv [1], &ev);
	if (pb == CORBA_OBJECT_NIL) {
		g_error ("Could not bind to PropertyBag object\n");
		return 1;
	}

	gtk_idle_add ((GtkFunction) create_bag_client, NULL);

	bonobo_main ();

	return 0;
}
