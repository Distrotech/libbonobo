#include "config.h"

#include "oafd.h"
#include "ac-query-expr.h"
#include <popt.h>

static char *od_source_dir = OAFINFODIR, *ac_evaluate = NULL;

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

  CORBA_exception_init(&ev);

  ctx = poptGetContext("oafd", argc, argv, options, 0);
  while(poptGetNextOpt(ctx) >= 0) /**/;

  poptFreeContext(ctx);

  orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);
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
  
  return 0;
}
