#include "config.h"

#include "liboaf/liboaf.h"

#include "oafd.h"
#include "ac-query-expr.h"
#include <popt.h>

#if 0
/* Production version */
static char *od_source_dir = OAFINFODIR;
#else
static char *od_source_dir = ".";
#endif

static char *ac_evaluate = NULL;

static struct poptOption options[] = {
  {"od-source-dir", '\0', POPT_ARG_STRING, &od_source_dir, 0, "Directory to read .oafinfo files from", "DIRECTORY"},
  {"evaluate", '\0', POPT_ARG_STRING, &ac_evaluate, 0, "Query expression to evaluate", "EXPRESSION"},
  POPT_AUTOHELP
  {NULL}
};

int main(int argc, char *argv[])
{
  QueryExpr *exp;
  const char *err;
  QueryContext tmpctx = {NULL, 0, CORBA_OBJECT_NIL};
  CORBA_ORB orb;
  PortableServer_POA root_poa;
  CORBA_Environment ev;
  OAF_ObjectDirectory od;
  OAF_ActivationContext ac;
  poptContext ctx;
  GMainLoop *ml;
  char *ior;

  CORBA_exception_init(&ev);

  ctx = poptGetContext("oafd", argc, argv, options, 0);
  while(poptGetNextOpt(ctx) >= 0) /**/;

  poptFreeContext(ctx);

  ml = g_main_new(FALSE);

  orb = oaf_orb_init(&argc, argv);
  root_poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, "RootPOA", &ev);
  od = OAF_ObjectDirectory_create(root_poa, "user", od_source_dir, &ev);
  ac = OAF_ActivationContext_create(root_poa, &ev);
  OAF_ActivationContext_add_directory(ac, od, &ev);

  if(ac_evaluate) {
    err = qexp_parse(ac_evaluate, &exp);
    if(err) {
      g_print("Parse error: %s\n", err);
    } else {
      QueryExprConst res;

      qexp_dump(exp);
      g_print("\n");
      g_print("Evaluation with no server record is: ");
      res = qexp_evaluate(NULL, exp, &tmpctx);
      qexp_constant_dump(&res);
      g_print("\n");
    }
  }

  ior = CORBA_ORB_object_to_string(orb, ac, &ev);
  fprintf(stdout, "%s\n", ior);
  fflush(stdout);
  CORBA_free(ior);

  PortableServer_POAManager_activate(PortableServer_POA__get_the_POAManager(root_poa, &ev), &ev);
  g_main_run(ml);
  
  return 0;
}
