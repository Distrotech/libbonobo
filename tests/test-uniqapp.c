/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <config.h>
#include <libbonobo.h>
#include <string.h>
#include "bonobo/bonobo-marshal.h"

#define TEST_MESSAGE    "test-message"
#define CLOSURE_MESSAGE "closure-message"


static GValue *
message_quit_cb (BonoboAppClient *app_client, const gchar *message, GValueArray *args)
{
	bonobo_main_quit ();
	return NULL;
}

static GValue *
message_cb (BonoboAppClient *app_client, const gchar *message, GValueArray *args)
{
	GValue *retval;

	g_return_val_if_fail (strcmp (message, TEST_MESSAGE) == 0, NULL);
	g_return_val_if_fail (args->n_values == 1, NULL);
	g_return_val_if_fail (G_VALUE_HOLDS_FLOAT (&args->values[0]), NULL);

	g_message ("message_cb: %s", message);

	retval = g_new0 (GValue, 1);
	g_value_init (retval, G_TYPE_FLOAT);
	g_value_set_float (retval, 2 * g_value_get_float (&args->values[0]));
	return retval;
}

static void
test_app_hook (BonoboApplication *app, gpointer data)
{
	g_message ("App '%s' created; data == %p", app->name, data);
}

static gint
new_instance_cb (BonoboApplication *app, gint argc, char *argv[])
{
	int i;

	g_message ("new-instance received. argc = %i; argv follows:", argc);
	for (i = 0; i < argc; ++i)
		g_message ("argv[%i] = \"%s\"", i, argv[i]);
	g_message ("new-instance: returning argc (%i)", argc);
	return argc;
}


static gfloat
closure_message_cb (BonoboApplication *app, gint arg_1, gfloat arg_2, gpointer data2)
{
	g_message("closure_message_cb: %p, %i, %f, %p",
		  app, arg_1, arg_2, data2);
	return arg_1 * arg_2;
}


int
main (int argc, char *argv [])
{
	BonoboApplication *app;
	BonoboAppClient   *client;
	float              msg_arg = 3.141592654;
	GClosure          *closure;

	if (bonobo_init (&argc, argv) == FALSE)
		g_error ("Can not bonobo_init");
	bonobo_activate ();

	bonobo_application_add_hook (test_app_hook, (gpointer) 0xdeadbeef);

	app = bonobo_application_new ("Libbonobo-Test-Uniqapp");

	closure = g_cclosure_new (G_CALLBACK (closure_message_cb),
				  (gpointer) 0xdeadbeef,  NULL);
	g_closure_set_marshal (closure, bonobo_marshal_FLOAT__LONG_FLOAT);
	bonobo_application_register_message (app, CLOSURE_MESSAGE,
					     "This is a test message",
					     closure,
					     G_TYPE_FLOAT, G_TYPE_LONG,
					     G_TYPE_FLOAT, G_TYPE_NONE);

	client = bonobo_application_register_unique (app);

	if (client)
	{
		BonoboAppClientMsgDesc const *msgdescs;
		GValue                       *retval;
		int                           i;

		g_message ("I am an application client.");
		bonobo_object_unref (BONOBO_OBJECT (app));
		app = NULL;

		msgdescs = bonobo_app_client_msg_list (client);
		g_assert (msgdescs);

		for (i = 0; msgdescs[i].name; ++i)
			g_message ("Application supports message '%s'", msgdescs[i].name);

		g_message ("Sending message string '%s' with argument %f",
			   TEST_MESSAGE, msg_arg);
		retval = bonobo_app_client_msg_send (client, TEST_MESSAGE,
						     G_TYPE_FLOAT, msg_arg,
						     G_TYPE_NONE);
		g_message ("Return value: %f", g_value_get_float (retval));
		if (retval) {
			g_value_unset (retval);
			g_free (retval);
		}

		g_message ("Sending message string '%s' with arguments %i and %f",
			   CLOSURE_MESSAGE, 10, 3.141592654);
		retval = bonobo_app_client_msg_send (client, CLOSURE_MESSAGE,
						     G_TYPE_LONG, 10,
						     G_TYPE_FLOAT, 3.141592654,
						     G_TYPE_NONE);
		g_message ("Return value: %f", g_value_get_float (retval));
		if (retval) {
			g_value_unset (retval);
			g_free (retval);
		}

		g_message ("Sending new-instance, with argc/argv");
		i = bonobo_app_client_new_instance (client, argc, argv);
		g_message ("new-instance returned %i", i);

		g_message ("Asking the server to quit");
		retval = bonobo_app_client_msg_send (client, "quit", G_TYPE_NONE);
		if (retval) {
			g_value_unset (retval);
			g_free (retval);
		}

		g_object_unref (client);
		return bonobo_debug_shutdown ();
	} else {
		g_message ("I am an application server");
		g_signal_connect (app, "message::test-message",
				  G_CALLBACK (message_cb), NULL);
		g_signal_connect (app, "new-instance",
				  G_CALLBACK (new_instance_cb), NULL);
		bonobo_application_new_instance (app, argc, argv);
		g_signal_connect (app, "message::quit",
				  G_CALLBACK (message_quit_cb), NULL);
	}

	bonobo_main ();

	if (app) bonobo_object_unref (BONOBO_OBJECT (app));

	return bonobo_debug_shutdown ();
}
