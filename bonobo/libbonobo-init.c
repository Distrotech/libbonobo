#include <config.h>
#include <libgnomebase/gnome-i18n.h>
#include <libgnomebase/gnome-program.h>

#include <bonobo/bonobo-main.h>
#include <bonobo/libbonobo-init.h>

#include <liboaf/liboaf.h>

/*****************************************************************************
 * oaf
 *****************************************************************************/

static void
gnome_oaf_pre_args_parse (GnomeProgram *program, GnomeModuleInfo *mod_info)
{
    oaf_preinit (program, mod_info);
}

static void
gnome_oaf_post_args_parse (GnomeProgram *program, GnomeModuleInfo *mod_info)
{
    int dumb_argc = 1;
    char *dumb_argv[] = {NULL};

    oaf_postinit (program, mod_info);

    dumb_argv[0] = program_invocation_name;
    (void) oaf_orb_init (&dumb_argc, dumb_argv);
}

GnomeModuleInfo gnome_oaf_module_info = {
    "gnome-oaf", VERSION, N_("GNOME OAF Support"),
    NULL,
    gnome_oaf_pre_args_parse, gnome_oaf_post_args_parse,
    oaf_popt_options
};

/*****************************************************************************
 * libbonobo
 *****************************************************************************/

static void
libbonobo_post_args_parse (GnomeProgram *program, GnomeModuleInfo *mod_info)
{
    CORBA_ORB orb;

    orb = oaf_orb_get ();

    bonobo_init (orb, CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);
}

static GnomeModuleRequirement libbonobo_requirements[] = {
    { VERSION, &gnome_oaf_module_info },
    { NULL, NULL }
};

GnomeModuleInfo libbonobo_module_info = {
    "libbonobo", VERSION, N_("Bonobo"),
    libbonobo_requirements,
    NULL, libbonobo_post_args_parse, NULL,
    NULL, NULL, NULL, NULL
};
