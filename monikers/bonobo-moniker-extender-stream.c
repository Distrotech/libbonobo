/*
 * gnome-moniker-extender-file.c: 
 *
 * Author:
 *	Dietmar Maurer (dietmar@maurer-it.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>
#include <stdlib.h>
  
#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-init.h>
#include <gtk/gtk.h>

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-generic-factory.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <libgnome/gnome-mime.h>
#include <liboaf/liboaf.h>

static int running_objects = 0;
static BonoboGenericFactory *extender_factory = NULL;

static gchar *
get_stream_type (Bonobo_Stream stream, CORBA_Environment *ev)
{
	Bonobo_StorageInfo *info;
	gchar              *type;

	g_return_val_if_fail (stream != CORBA_OBJECT_NIL, NULL);

	info = Bonobo_Stream_getInfo (stream, Bonobo_FIELD_CONTENT_TYPE, ev);
	
	if (BONOBO_EX (ev)) /* FIXME: we could try and do it ourselfs here */
		return NULL;

	type = g_strdup (info->content_type);

	CORBA_free (info);

	return type;
}

static Bonobo_Unknown
stream_extender_resolve (BonoboMonikerExtender       *extender,
			 const Bonobo_Moniker         m,
			 const Bonobo_ResolveOptions *options,
			 const CORBA_char            *display_name,
			 const CORBA_char            *requested_interface,
			 CORBA_Environment           *ev)
{
	const char    *mime_type;
	char          *oaf_requirements;
	Bonobo_Unknown object;
	Bonobo_Unknown stream;
	Bonobo_Persist persist;
	OAF_ActivationID ret_id;

	g_warning ("Stream extender: '%s'", display_name);

	if (!m)
		return CORBA_OBJECT_NIL;

	stream = Bonobo_Moniker_resolve (m, options, "IDL:Bonobo/Stream:1.0", ev);

	if (!stream)
		return CORBA_OBJECT_NIL;

	mime_type = get_stream_type (stream, ev);
	if (!mime_type)
		goto unref_stream_exception;

	oaf_requirements = g_strdup_printf (
		"bonobo:supported_mime_types.has ('%s') AND repo_ids.has ('%s') AND "
		"repo_ids.has (['IDL:Bonobo/PersistStream:1.0'])",
		mime_type, requested_interface);
		
	object = oaf_activate (oaf_requirements, NULL, 0, &ret_id, ev);
	g_warning ("Attempt activate object satisfying '%s': %p",
		   oaf_requirements, object);
	g_free (oaf_requirements);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto unref_stream_exception;
		
	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Can't find object satisfying requirements");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		goto unref_stream_exception;
	}

	persist = Bonobo_Unknown_queryInterface (
		object, "IDL:Bonobo/PersistStream:1.0", ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto unref_object_exception;

	if (persist != CORBA_OBJECT_NIL) {
		Bonobo_PersistStream_load (
			persist, stream, (const Bonobo_Persist_ContentType) mime_type, ev);

		bonobo_object_release_unref (persist, ev);
		bonobo_object_release_unref (stream, ev);

		return bonobo_moniker_util_qi_return (
			object, requested_interface, ev);
	}

 unref_object_exception:
	bonobo_object_release_unref (object, ev);

 unref_stream_exception:
	bonobo_object_release_unref (stream, ev);

	return CORBA_OBJECT_NIL;
}


static void
extender_destroy_cb (BonoboMonikerExtender *extender, gpointer dummy)
{
	running_objects--;

	if (running_objects > 0)
		return;

	bonobo_object_unref (BONOBO_OBJECT (extender_factory));
	gtk_main_quit ();
}

static BonoboObject *
file_extender_factory (BonoboGenericFactory *this,
		       void                 *data)
{
	BonoboMonikerExtender *extender = NULL;
	BonoboObject  *object  = NULL;
	
	extender = bonobo_moniker_extender_new (stream_extender_resolve, NULL);

	object = BONOBO_OBJECT (extender);
	
	if (object) {
		running_objects++;
		
		gtk_signal_connect (GTK_OBJECT (extender), "destroy",
				    GTK_SIGNAL_FUNC (extender_destroy_cb), NULL);
	}

	return object;
}

int
main (int argc, char *argv [])
{
	CORBA_Environment ev;
	CORBA_ORB orb = CORBA_OBJECT_NIL;
	char *dummy;

	dummy = malloc (8); if (dummy) free (dummy);

	CORBA_exception_init (&ev);

        gnome_init_with_popt_table ("bonobo-moniker-extender-stream", "0.0", argc, argv,
				    oaf_popt_options, 0, NULL); 
	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL) == FALSE)
		g_error (_("I could not initialize Bonobo"));

	extender_factory = bonobo_generic_factory_new (
		"OAFIID:Bonobo_MonikerExtender_streamFactory",
		file_extender_factory, NULL);	

	bonobo_main ();

	printf ("EXIT file extender\n");

	CORBA_exception_free (&ev);

	return 0;
}
