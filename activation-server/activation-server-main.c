/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "config.h"

#include <popt.h>
#include <signal.h>
#include <stdlib.h>

#include "liboaf/liboaf.h"

#include <ORBitservices/CosNaming.h>
#include <ORBitservices/CosNaming_impl.h>

#include "oafd.h"
#include "ac-query-expr.h"

#ifdef OAF_DEBUG
static void debug_queries (void);
#endif

/* Option values */
static char *od_source_dir = NULL;
static char *ac_evaluate = NULL, *od_domain = "session";
static int server_ac = 0, ior_fd = -1, server_ns = 1;

static struct poptOption options[] = {

	{"od-source-dir", '\0', POPT_ARG_STRING, &od_source_dir, 0,
	 "Directory to read .oafinfo files from", "DIRECTORY"},
	{"od-domain", '\0', POPT_ARG_STRING, &od_domain, 0,
	 "Domain of ObjectDirectory", "DOMAIN"},

	{"ac-activate", '\0', POPT_ARG_NONE, &server_ac, 0,
	 "Serve as an ActivationContext (default is as an ObjectDirectory only)",
	 NULL},


	{"ior-output-fd", '\0', POPT_ARG_INT, &ior_fd, 0,
	 "File descriptor to write IOR to", "FD"},

#ifdef OAF_DEBUG

	{"evaluate", '\0', POPT_ARG_STRING, &ac_evaluate, 0,
	 "Query expression to evaluate", "EXPRESSION"},
#endif

	POPT_AUTOHELP {NULL}
};

int
main (int argc, char *argv[])
{
	GMainLoop *ml;
	CORBA_ORB orb;
	PortableServer_POA root_poa;
	CORBA_Environment ev;
	OAF_ObjectDirectory od;
	OAF_ActivationContext ac;
	CORBA_Object primary_server;
	poptContext ctx;
	char *ior;
	FILE *fh;
	struct sigaction sa;

	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = SIG_IGN;
	sigaction (SIGPIPE, &sa, NULL);

	CORBA_exception_init (&ev);

	ctx = poptGetContext ("oafd", argc, argv, options, 0);
	while (poptGetNextOpt (ctx) >= 0)
		/**/;

	poptFreeContext (ctx);

	ml = g_main_new (FALSE);

	orb = oaf_init (argc, argv);
	root_poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (orb, "RootPOA", &ev);
	{
		char *env_od_source_dir;
		char *gnome_env_od_source_dir;
		GString *real_od_source_dir;

		real_od_source_dir = g_string_new (OAFINFODIR);
		env_od_source_dir = getenv ("OAF_INFO_PATH");

		if (od_source_dir) {
			g_string_append_c (real_od_source_dir, ':');
			g_string_append (real_od_source_dir, od_source_dir);
		}
		if (env_od_source_dir) {
			g_string_append_c (real_od_source_dir, ':');
			g_string_append (real_od_source_dir,
					 env_od_source_dir);
		}

		od = OAF_ObjectDirectory_create (root_poa, od_domain,
                                                 real_od_source_dir->str,
                                                 &ev);
		if (server_ns) {
			CORBA_Object naming_service;

			naming_service =
				impl_CosNaming_NamingContext__create
				(root_poa, &ev);
			OAF_ObjectDirectory_register_new 
                                (od,
                                 "OAFIID:oaf_naming_service:7e2b90ef-eaf0-4239-bb7c-812606fcd80d",
                                 naming_service,
                                 &ev);
		}

		g_string_free (real_od_source_dir, TRUE);
	}
	if (server_ac) {
		primary_server = ac =
			OAF_ActivationContext_create (root_poa, &ev);
		OAF_ActivationContext_add_directory (ac, od, &ev);
	} else
		primary_server = od;

	ior = CORBA_ORB_object_to_string (orb, primary_server, &ev);

	fh = NULL;
	if (ior_fd >= 0)
		fh = fdopen (ior_fd, "w");
	if (fh) {
		fprintf (fh, "%s\n", ior);
		fclose (fh);
	} else {
		fprintf (stdout, "%s\n", ior);
		fflush (stdout);
	}
	CORBA_free (ior);

#ifdef OAF_DEBUG
	debug_queries ();
#endif

	PortableServer_POAManager_activate
		(PortableServer_POA__get_the_POAManager (root_poa, &ev), &ev);
	g_main_run (ml);

	return 0;
}

#ifdef OAF_DEBUG
static void
debug_queries (void)
{
	if (ac_evaluate) {
		QueryExpr *exp;
		const char *err;
		QueryContext tmpctx = { NULL, 0, CORBA_OBJECT_NIL };

		err = qexp_parse (ac_evaluate, &exp);
		if (err) {
			g_print ("Parse error: %s\n", err);
		} else {
			QueryExprConst res;

			qexp_dump (exp);
			g_print ("\n");
			g_print ("Evaluation with no server record is: ");
			res = qexp_evaluate (NULL, exp, &tmpctx);
			qexp_constant_dump (&res);
			g_print ("\n");
		}
	}
}
#endif
