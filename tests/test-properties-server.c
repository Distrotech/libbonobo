
#include <config.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-bonobo.h>
#include <stdio.h>

CORBA_ORB	   orb;
GnomePropertyBag  *pb;
CORBA_Environment  ev;

static char *
simple_prop_to_string (char *type, gpointer v)
{
	static char s[1024];

	if (v == NULL) {
		g_snprintf (s, sizeof (s), "NULL");
	} else if (! strcmp (type, "boolean")) {
		g_snprintf (s, sizeof (s), "%s", (*(gboolean *) v) ?
			"True" : "False");
	} else if (! strcmp (type, "short")) {
		g_snprintf (s, sizeof (s), "%d", *(gshort *) v);
	} else if (! strcmp (type, "ushort")) {
		g_snprintf (s, sizeof (s), "%d", *(gushort *) v);
	} else if (! strcmp (type, "long")) {
		g_snprintf (s, sizeof (s), "%ld", *(glong *) v);
	} else if (! strcmp (type, "ulong")) {
		g_snprintf (s, sizeof (s), "%ld", *(gulong *) v);
	} else if (! strcmp (type, "string")) {
		g_snprintf (s, sizeof (s), "%s", (char *) v);
	} else if (! strcmp (type, "float")) {
		g_snprintf (s, sizeof (s), "%f", *(gfloat *) v);
	} else if (! strcmp (type, "double")) {
		g_snprintf (s, sizeof (s), "%f", *(gdouble *) v);
	} else {
		g_error ("Unhandled type: %s\n", type);
	}

	return s;
}

static void
value_changed_cb (GnomePropertyBag *pb, char *name, char *type,
		  gpointer old_value, gpointer new_value,
		  gpointer user_data)
{
	char *s1, *s2;

	s1 = g_strdup (simple_prop_to_string (type, old_value));
	s2 = g_strdup (simple_prop_to_string (type, new_value));
	g_print ("Prop %s [%s]: value changed from %s to %s\n",
		 name, type, s1, s2);
	g_free (s1);
	g_free (s2);

}

static void
create_bag (void)
{
	char		  *str;
	char		  *dstr;
	gshort		  *s;
	gshort		  *ds;
	gushort		  *u;
	gushort		  *du;
	glong		  *l;
	glong		  *dl;
	gboolean	  *b;
	gboolean	  *db;
	gfloat		  *f;
	gfloat		  *df;
	char		  *ior;

	/* Create the property bag. */
	pb = gnome_property_bag_new ();

	/* Add properties */
	*(s = g_new0 (gshort, 1)) = 5;
	*(ds = g_new0 (gshort, 1)) = 532;
	gnome_property_bag_add (pb, "short-test", "short", s, ds, NULL, 0);

	str = g_strdup ("Testing 1 2 3");
	dstr = g_strdup ("Default string");
	gnome_property_bag_add (pb, "string-test", "string", str, dstr, NULL, 0);
	
	*(l = g_new0 (glong, 1)) = 43243;
	*(dl = g_new0 (glong, 1)) = 26;
	gnome_property_bag_add (pb, "long-test", "long", l, dl, NULL, 0);

	*(b = g_new0 (gboolean, 1)) = TRUE;
	*(db = g_new0 (gboolean, 1)) = FALSE;
	gnome_property_bag_add (pb, "boolean-test", "boolean", b, db, NULL, 0);

	*(f = g_new0 (gfloat, 1)) = 3.14159265358979323;
	*(df = g_new0 (gfloat, 1)) = 2.718281828;
	gnome_property_bag_add (pb, "float-test", "float", f, df, NULL, 0);

	*(u = g_new0 (gushort, 1)) = 234;
	*(du = g_new0 (gushort, 1)) = 0;
	gnome_property_bag_add (pb, "ushort-test", "ushort", u, du, NULL, 0);

	/* Connect to the signal */
	gtk_signal_connect (GTK_OBJECT (pb), "value_changed",
			    value_changed_cb, NULL);

	/* Print out the IOR for this object. */
	ior = CORBA_ORB_object_to_string (
		orb, gnome_object_corba_objref (GNOME_OBJECT (pb)), &ev);

	g_print ("%s\n", ior);
	fflush (stdout);

	CORBA_free (ior);
}

static void
print_props (void)
{
	GList *props;
	GList *l;

	/* This is a private function; we're just using it for testing. */
	props = gnome_property_bag_get_prop_list (pb);

	for (l = props; l != NULL; l = l->next) {
		GnomeProperty *prop = l->data;
		char *s1, *s2;

		s1 = g_strdup (simple_prop_to_string (prop->type, prop->value));
		s2 = g_strdup (simple_prop_to_string (prop->type, prop->default_value));

		g_print ("Prop %s [%s] %s %s %s %s %s %s\n",
			 prop->name, prop->type,
			 s1, s2,
			 prop->docstring,
			 prop->flags & GNOME_PROPERTY_UNSTORED        ? "Unstored"         : "Stored",
			 prop->flags & GNOME_PROPERTY_READ_ONLY       ? "ReadOnly"         : "ReadWrite",
			 prop->flags & GNOME_PROPERTY_USE_DEFAULT_OPT ? "DefaultOptimized" : "NotDefaultOptimized");

		g_free (s1);
		g_free (s2);

	}

	g_list_free (props);
}

int
main (int argc, char **argv)
{
	CORBA_exception_init (&ev);

	gnome_CORBA_init_with_popt_table (
		"test property server", "0.0", &argc, argv,
		NULL, 0, NULL, GNORBA_INIT_SERVER_FUNC, &ev);

	orb = gnome_CORBA_ORB ();

	if (! bonobo_init (orb, NULL, NULL))
		g_error ("Could not initialize Bonobo");

	create_bag ();

	print_props ();

	bonobo_main ();

	return 0;
}
