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
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>


/* Whacked from gnome-libs/libgnorba/orbitns.c */

#define IORBUFSIZE 2048

typedef struct
{
	GMainLoop *mloop;
	char iorbuf[IORBUFSIZE];
#ifdef BONOBO_ACTIVATION_DEBUG
	char *do_srv_output;
#endif
	FILE *fh;
}
EXEActivateInfo;

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
            (condition & G_IO_IN)) {
                if (!fgets (data->iorbuf, sizeof (data->iorbuf), data->fh)) {
                        g_snprintf (data->iorbuf, IORBUFSIZE,
                                    _("Failed to read from child process: %s\n"),
                                    strerror (errno));

                        retval = FALSE;
                } else {
                        retval = TRUE;
                }
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
		g_main_quit (data->mloop);

	return retval;
}

#ifdef BONOBO_ACTIVATION_DEBUG
static void
print_exit_status (int status)
{
	if (WIFEXITED (status))
		g_message ("Exit status was %d", WEXITSTATUS (status));

	if (WIFSIGNALED (status))
		g_message ("signal was %d", WTERMSIG (status));
}
#endif

static void
bonobo_activation_setenv (const char *name, const char *value) 
{
#if HAVE_SETENV
        setenv (name, value, 1);
#else
        char *tmp;
                
        tmp = g_strconcat (name, "=", value, NULL);
        
        putenv (tmp);
#endif
}

CORBA_Object
bonobo_activation_server_by_forking (
        const char       **cmd,
        gboolean           set_process_group,
        int                fd_arg, 
        const char        *display,
        const char        *od_iorstr,
        CORBA_Environment *ev)
{
	gint iopipes[2];
	CORBA_Object retval = CORBA_OBJECT_NIL;
	Bonobo_GeneralError *errval;
        FILE *iorfh;
        EXEActivateInfo ai;
        GIOChannel *gioc;
        int childpid;
        int status;
        guint watchid;
        struct sigaction sa;
        sigset_t mask, omask;
        int parent_pid;
        
     	pipe (iopipes);

        /* Block SIGCHLD so no one else can wait() on the child before us. */
        sigemptyset (&mask);
        sigaddset (&mask, SIGCHLD);
        sigprocmask (SIG_BLOCK, &mask, &omask);

        parent_pid = getpid ();
        
	/* fork & get the IOR from the magic pipe */
	childpid = fork ();

	if (childpid < 0) {
                sigprocmask (SIG_SETMASK, &omask, NULL);
		errval = Bonobo_GeneralError__alloc ();
		errval->description = CORBA_string_dup (_("Couldn't fork a new process"));

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_GeneralError, errval);
		return CORBA_OBJECT_NIL;
	}

	if (childpid != 0) {
                /* de-zombify */
                while (waitpid (childpid, &status, 0) == -1 && errno == EINTR)
                        ;
                sigprocmask (SIG_SETMASK, &omask, NULL);
                
		if (!WIFEXITED (status)) {
			Bonobo_GeneralError *errval;
			char cbuf[512];
                        
			errval = Bonobo_GeneralError__alloc ();

			if (WIFSIGNALED (status))
				g_snprintf (cbuf, sizeof (cbuf),
					    _("Child received signal %u (%s)"),
					    WTERMSIG (status),
					    g_strsignal (WTERMSIG
                                                         (status)));
			else
				g_snprintf (cbuf, sizeof (cbuf),
					    _("Unknown non-exit error (status is %u)"),
					    status);
                        errval->description = CORBA_string_dup (cbuf);

			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_GeneralError, errval);
			return CORBA_OBJECT_NIL;
		}
#ifdef BONOBO_ACTIVATION_DEBUG
		ai.do_srv_output = getenv ("BONOBO_ACTIVATION_DEBUG_EXERUN");
                
		if (ai.do_srv_output)
			print_exit_status (status);
#endif
                
		close (iopipes[1]);
		ai.fh = iorfh = fdopen (iopipes[0], "r");
                
		ai.iorbuf[0] = '\0';
		ai.mloop = g_main_new (FALSE);
		gioc = g_io_channel_unix_new (iopipes[0]);
		watchid = g_io_add_watch (gioc,
                                          G_IO_IN | G_IO_HUP | G_IO_NVAL |
                                          G_IO_ERR, (GIOFunc) & handle_exepipe,
                                          &ai);
		g_io_channel_unref (gioc);
		g_main_run (ai.mloop);
		g_main_destroy (ai.mloop);
		fclose (iorfh);

		g_strstrip (ai.iorbuf);
		if (!strncmp (ai.iorbuf, "IOR:", 4)) {
			retval = CORBA_ORB_string_to_object (bonobo_activation_orb_get (),
                                                             ai.iorbuf, ev);
			if (ev->_major != CORBA_NO_EXCEPTION)
				retval = CORBA_OBJECT_NIL;
#ifdef BONOBO_ACTIVATION_DEBUG
			if (ai.do_srv_output)
				g_message ("Did string_to_object on %s = '%p' (%s)",
					   ai.iorbuf, retval,
                                           ev->_major == CORBA_NO_EXCEPTION?
                                           "no-exception" : ev->_id);
#endif
		} else {
			Bonobo_GeneralError *errval;

#ifdef BONOBO_ACTIVATION_DEBUG
			if (ai.do_srv_output)
				g_message ("string doesn't match IOR:");
#endif

			errval = Bonobo_GeneralError__alloc ();

                        if (*ai.iorbuf == '\0')
                                errval->description =
                                        CORBA_string_dup (_("Child process did not give an error message, unknown failure occurred"));
                        else
                                errval->description = CORBA_string_dup (ai.iorbuf);
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_GeneralError, errval);
			retval = CORBA_OBJECT_NIL;
		}
	} else if ((childpid = fork ())) {
		_exit (0);	/* de-zombifier process, just exit */
	} else {
                if (display)
		  bonobo_activation_setenv ("DISPLAY", display);
		if (od_iorstr)
		  bonobo_activation_setenv ("BONOBO_ACTIVATION_OD_IOR", od_iorstr);
                

		close (iopipes[0]);
                
                if (fd_arg != 0) {
                        cmd[fd_arg] = g_strdup_printf (cmd[fd_arg], iopipes[1]);
                }

		memset (&sa, 0, sizeof (sa));
		sa.sa_handler = SIG_IGN;
		sigaction (SIGPIPE, &sa, 0);

                if (set_process_group) {
                        if (setpgid (getpid (), parent_pid) < 0) {
                                g_print (_("bonobo-activation failed to set process group of %s: %s\n"),
                                         cmd[0], g_strerror (errno));
                                _exit (1);
                        }
                } else {
                        setsid ();
                }
                
		execvp (cmd[0], (char **) cmd);
		if (iopipes[1] != 1)
			dup2 (iopipes[1], 1);
		g_print (_("Failed to execute %s: %d (%s)\n"),
                         cmd[0],
                         errno, g_strerror (errno));
		_exit (1);
	}

	return retval;
}
