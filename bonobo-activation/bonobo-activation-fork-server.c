/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  bonobo-activation: A library for accessing bonobo-activation-server.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000, 2001 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

#include <config.h>

#include <bonobo-activation/bonobo-activation-private.h>
#include <bonobo-activation/bonobo-activation-i18n.h>
#include <bonobo-activation/bonobo-activation-init.h>
#include <bonobo-activation/Bonobo_ActivationContext.h>

#include <orbit/orbit.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>

/* Whacked from gnome-libs/libgnorba/orbitns.c */

#define IORBUFSIZE 2048

typedef struct {
        gboolean   done;
	char iorbuf[IORBUFSIZE];
#ifdef BONOBO_ACTIVATION_DEBUG
	char *do_srv_output;
#endif
        GIOChannel *ioc;

        /* For list compares */
        const Bonobo_ActivationEnvironment *environment;

        const char *act_iid;
        const char *exename;
        BonoboForkReCheckFn re_check;
        gpointer            user_data;
} EXEActivateInfo;

static CORBA_Object
exe_activate_info_to_retval (EXEActivateInfo *ai, CORBA_Environment *ev)
{
        CORBA_Object retval;

        g_strstrip (ai->iorbuf);
        if (!strncmp (ai->iorbuf, "IOR:", 4)) {
                retval = CORBA_ORB_string_to_object (bonobo_activation_orb_get (),
                                                     ai->iorbuf, ev);
                if (ev->_major != CORBA_NO_EXCEPTION)
                        retval = CORBA_OBJECT_NIL;
#ifdef BONOBO_ACTIVATION_DEBUG
                if (ai->do_srv_output)
                        g_message ("Did string_to_object on %s = '%p' (%s)",
                                   ai->iorbuf, retval,
                                   ev->_major == CORBA_NO_EXCEPTION?
                                   "no-exception" : ev->_id);
#endif
        } else {
                Bonobo_GeneralError *errval;

#ifdef BONOBO_ACTIVATION_DEBUG
                if (ai->do_srv_output)
                        g_warning ("string doesn't match IOR:");
#endif

                errval = Bonobo_GeneralError__alloc ();

                if (*ai->iorbuf == '\0')
                        errval->description =
                                CORBA_string_dup (_("Child process did not give an error message, unknown failure occurred"));
                else
                        errval->description = CORBA_string_dup (ai->iorbuf);
                CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
                                     ex_Bonobo_GeneralError, errval);
                retval = CORBA_OBJECT_NIL;
        }

        return retval;
}

static CORBA_Object
scan_list (GSList *l, EXEActivateInfo *seek_ai, CORBA_Environment *ev)
{
        CORBA_Object retval = CORBA_OBJECT_NIL;

        for (; l; l = l->next) {
                EXEActivateInfo *ai = l->data;

                if (strcmp (seek_ai->exename, ai->exename))
                        continue;

                if (seek_ai->environment && ai->environment) {
                        if (!Bonobo_ActivationEnvironment_match (seek_ai->environment,
								 ai->environment))
                                continue;

                } else if (seek_ai->environment || ai->environment)
                        continue;

                /* We run the loop too ... */
                while (!ai->done)
                        g_main_context_iteration (NULL, TRUE);

                if (!strcmp (seek_ai->act_iid, ai->act_iid)) {
#ifdef BONOBO_ACTIVATION_DEBUG
                        g_message ("Hit the jackpot '%s' '%s'",
                                   seek_ai->act_iid, ai->act_iid);
#endif
                        retval = exe_activate_info_to_retval (ai, ev);
                } else if (seek_ai->re_check) {
                        /* It might have just registered the IID */
#ifdef BONOBO_ACTIVATION_DEBUG
                        g_message ("Re-check the thing ... '%s' '%s'",
                                   seek_ai->act_iid, ai->act_iid);
#endif
                        retval = seek_ai->re_check (
					seek_ai->environment,
					seek_ai->act_iid,
					seek_ai->user_data, ev);
                }
        }

        return retval;
}

static gboolean
handle_exepipe (GIOChannel * source,
		GIOCondition condition, 
                EXEActivateInfo * data)
{
	gboolean retval = TRUE;

        /* The expected thing is to get this callback maybe twice,
         * once with G_IO_IN and once G_IO_HUP, of course we need to handle
         * other cases.
         */
        
        if (data->iorbuf[0] == '\0' &&
            (condition & (G_IO_IN | G_IO_PRI))) {
                GString *str = g_string_new ("");
                GError *error = NULL;
                GIOStatus status;

                status = g_io_channel_read_line_string (data->ioc, str, NULL, &error);
                if (status == G_IO_STATUS_ERROR) {
                        g_snprintf (data->iorbuf, IORBUFSIZE,
                                    _("Failed to read from child process: %s\n"),
                                    error->message);
                        g_error_free (error);
                        error = NULL;
                        retval = FALSE;
                } else if (status == G_IO_STATUS_EOF) {
                        g_snprintf (data->iorbuf, IORBUFSIZE,
                                    _("EOF from child process\n"));
                        retval = FALSE;
                } else {
                        strncpy (data->iorbuf, str->str, IORBUFSIZE);
                        retval = TRUE;
                }
                g_string_free (str, TRUE);
        } else {                
                retval = FALSE;
        }

	if (retval && !strncmp (data->iorbuf, "IOR:", 4))
		retval = FALSE;

#ifdef BONOBO_ACTIVATION_DEBUG
	if (data->do_srv_output)
		g_message ("srv output[%d]: '%s'", retval, data->iorbuf);
#endif

	if (!retval)
                data->done = TRUE;

	return retval;
}

CORBA_Object
bonobo_activation_server_by_forking (
	const char                         **cmd,
	gboolean                             set_process_group,
	int                                  fd_arg, 
	const Bonobo_ActivationEnvironment  *environment,
	const char                          *od_iorstr,
	const char                          *act_iid,
        gboolean                             use_new_loop,
	BonoboForkReCheckFn                  re_check,
	gpointer                             user_data,
	CORBA_Environment                   *ev)
{
        int i;
	gint iopipes[2];
	CORBA_Object retval = CORBA_OBJECT_NIL;
        static GSList *running_activations = NULL;
        EXEActivateInfo ai;
        GError *error = NULL;
        GSource *source;
        GMainContext *context;
        char **newenv = NULL;

#if defined(__APPLE__) && defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif

#ifndef G_OS_WIN32
        extern char **environ;
#endif

        g_return_val_if_fail (cmd != NULL, CORBA_OBJECT_NIL);
        g_return_val_if_fail (cmd [0] != NULL, CORBA_OBJECT_NIL);
        g_return_val_if_fail (act_iid != NULL, CORBA_OBJECT_NIL);

        ai.environment = environment;
        ai.act_iid = act_iid;
        ai.exename = cmd [0];
        ai.re_check = re_check;
        ai.user_data = user_data;

#ifdef BONOBO_ACTIVATION_DEBUG
        ai.do_srv_output = getenv ("BONOBO_ACTIVATION_DEBUG_EXERUN");
#endif
                
        if (!use_new_loop &&
            (retval = scan_list (running_activations, &ai, ev)) != CORBA_OBJECT_NIL)
                return retval;
        
     	pipe (iopipes);

#ifdef BONOBO_ACTIVATION_DEBUG
        if (ai.do_srv_output)
                fprintf (stderr, " SPAWNING: '%s' for '%s'\n", cmd[0], act_iid);
#endif

#ifdef G_OS_WIN32
        ai.ioc = g_io_channel_win32_new_fd (iopipes[0]);
#else
        ai.ioc = g_io_channel_unix_new (iopipes[0]);
#endif
        g_io_channel_set_encoding (ai.ioc, NULL, NULL);

        source = g_io_create_watch
                (ai.ioc, G_IO_IN | G_IO_PRI | G_IO_HUP | G_IO_NVAL | G_IO_ERR);
        g_source_set_callback (source, (GSourceFunc) handle_exepipe, &ai, NULL);
        g_source_set_can_recurse (source, TRUE);

        if (use_new_loop)
                context = g_main_context_new ();
        else
                context = g_main_context_default ();
        g_source_attach (source, context);

        /* Set up environment for child */
        if (environment && environment->_length > 0) {
                int i, n;
                char **ep;

                n = environment->_length;

                ep = environ;
                while (*ep) {
                        ep++;
                        n++;
                }
                        
                newenv = g_new (char *, n+1);

                for (i = n = 0; i < environment->_length; i++, n++) {
                        newenv [n] = g_strconcat (environment->_buffer [i].name,
                                                  "=",
                                                  environment->_buffer [i].value,
                                                  NULL);
                }

                ep = environ;
                while (*ep) {
                        char *equal = strchr (*ep, '=');
                        if (equal) {
                                for (i = 0; i < environment->_length; i++)
                                        if (equal - *ep == strlen (environment->_buffer [i].name) &&
                                            memcmp (*ep, environment->_buffer [i].name,
                                                    equal - *ep) == 0)
                                                break;
                                if (i == environment->_length) {
                                        newenv [n] = g_strdup (*ep);
                                        n++;
                                }
                        }
                        ep++;
                }
                newenv [n] = NULL;
        }

        /* Pass the IOR pipe's write end to the child */ 
        if (fd_arg != 0)
                cmd[fd_arg] = g_strdup_printf (cmd[fd_arg], iopipes[1]);

        ai.iorbuf[0] = '\0';
        ai.done = FALSE;
        
        running_activations = g_slist_prepend (running_activations, &ai);

	/* Spawn */
        if (!g_spawn_async (NULL, (gchar **) cmd, newenv,
                            G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
                            G_SPAWN_SEARCH_PATH |
                            G_SPAWN_CHILD_INHERITS_STDIN,
                            NULL, NULL,
                            NULL,
                            &error)) {
                Bonobo_GeneralError *errval;
                gchar *error_message = g_strconcat (_("Couldn't spawn a new process"),
                                                    ":",
                                                    error->message,
                                                    NULL);
                g_error_free (error);
                error = NULL;

		errval = Bonobo_GeneralError__alloc ();
		errval->description = CORBA_string_dup (error_message);
                g_free (error_message);

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_GeneralError, errval);

                running_activations = g_slist_remove (running_activations, &ai);

                g_source_destroy (source);
                g_source_unref (source);
                
                g_io_channel_unref (ai.ioc);
                
                if (use_new_loop)
                        g_main_context_unref (context);

		return CORBA_OBJECT_NIL;
	}

        close (iopipes[1]);

        if (newenv) {
                char **ep = newenv;

                while (*ep) {
                        g_free (*ep);
                        ep++;
                }
                g_free (newenv);
        }

        if (fd_arg != 0)
                g_free ((char *) cmd[fd_arg]);

        /* Get the IOR from the pipe */
        while (!ai.done) {
                g_main_context_iteration (context, TRUE);
        }

        g_source_destroy (source);
        g_source_unref (source);
        
        g_io_channel_shutdown (ai.ioc, FALSE, NULL);
        g_io_channel_unref (ai.ioc);
        
        if (use_new_loop)
                g_main_context_unref (context);
        
        running_activations = g_slist_remove (running_activations, &ai);

        retval = exe_activate_info_to_retval (&ai, ev); 

        close (iopipes[0]);
                
	return retval;
}
