#include "ac-query-expr.h"

int main(int argc, char *argv[])
{
  QueryExpr *exp;
  const char *err;
  QueryContext tmpctx = {NULL, 0, CORBA_OBJECT_NIL};

  if(argc < 2)
    return 1;

  err = qexp_parse(argv[1], &exp);
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
  
  return 0;
}
