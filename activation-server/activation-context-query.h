#ifndef QUERY_EXPR_H
#define QUERY_EXPR_H 1

#include "liboaf/oaf.h"

#include <glib.h>

typedef enum { EXPR_FUNCTION, EXPR_VARIABLE, EXPR_ID, EXPR_BINOP, EXPR_UNOP, EXPR_CONSTANT } QueryExprType;

typedef enum { CONST_STRING, CONST_STRINGV, CONST_NUMBER, CONST_BOOLEAN } QueryExprConstType;

typedef struct {
  QueryExprConstType type;
  union {
    char *v_string;
    char **v_stringv;
    gdouble v_number;
    gboolean v_boolean;
  } u;

  guchar value_known, needs_free;
} QueryExprConst;

typedef struct _QueryExpr QueryExpr;

struct _QueryExpr {
  QueryExprType type;

  union {
    struct {
      char *func_name;
      GSList *arguments;
    } function_value;

    char *var_value;
    char *id_value;

    struct {
      enum { OP_EQ, OP_NEQ, OP_LEQ, OP_GEQ, OP_LT, OP_GT, OP_OR,
	     OP_AND, OP_MULTIPLY, OP_DIVIDE, OP_ADD, OP_SUBTRACT, OP_XOR } type;

      QueryExpr *op1, *op2;
    } binop_value;

    struct {
      enum { OP_NOT, OP_NEGATE } type;
      QueryExpr *op;
    } unop_value;

    QueryExprConst constant_value;
  } u;

  QueryExprConst cached_value;
  guchar has_fields, have_cached_value;
};

QueryExpr *qexp_binop_new(QueryExpr *op1, int operand, QueryExpr *op2);
QueryExpr *qexp_unop_new(int operand, QueryExpr *op);
QueryExpr *qexp_function_new(char *name, GSList *exprlist);
QueryExpr *qexp_variable_new(char *name);
QueryExpr *qexp_id_new(char *name);
QueryExpr *qexp_constant_new(QueryExprConst setme);

const char *qexp_parse( const char *_code, QueryExpr **retme ); /* Return value is a string describing any errors */
void qexp_free(QueryExpr *qexp);

/* For debugging purposes */
void qexp_dump(QueryExpr *exp);
void qexp_constant_dump(QueryExprConst *c);

typedef struct {
  OAF_ServerInfo **sil;

  int nservers;

  CORBA_Context cctx;
} QueryContext;

QueryExprConst qexp_evaluate(OAF_ServerInfo *si, QueryExpr *e, QueryContext *qctx);
gboolean qexp_matches(OAF_ServerInfo *si, QueryExpr *e, QueryContext *qctx);
void qexp_sort(OAF_ServerInfo **servers, int nservers, QueryExpr **sexps, int nexps, QueryContext *qctx);

#endif
