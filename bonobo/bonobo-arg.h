#ifndef __BONOBO_ARG_H__
#define __BONOBO_ARG_H__

#include <bonobo/Bonobo.h>

BEGIN_GNOME_DECLS

typedef CORBA_any      BonoboArg;
typedef CORBA_TypeCode BonoboArgType;

#define BONOBO_ARG_NULL    TC_null
#define BONOBO_ARG_BOOLEAN TC_boolean
#define BONOBO_ARG_INT     TC_long
#define BONOBO_ARG_LONG    TC_long
#define BONOBO_ARG_FLOAT   TC_float
#define BONOBO_ARG_DOUBLE  TC_double
#define BONOBO_ARG_STRING  TC_string

#define BONOBO_ARG_GET_GENERAL(a,t)   (g_assert ((a)->_type->kind == CORBA_tk##t),	\
				       *((CORBA##t *)(a->_value)))
#define BONOBO_ARG_SET_GENERAL(a,v,t) (g_assert ((a)->_type->kind == CORBA_tk##t),	\
				       *((CORBA##t *)(a->_value)) = (CORBA##t)(v))

#define BONOBO_ARG_GET_BOOLEAN(a)   (BONOBO_ARG_GET_GENERAL (a, _boolean))
#define BONOBO_ARG_SET_BOOLEAN(a,v) (BONOBO_ARG_SET_GENERAL (a, v, _boolean))

#define BONOBO_ARG_GET_INT(a)       (BONOBO_ARG_GET_GENERAL (a, _long))
#define BONOBO_ARG_SET_INT(a,v)     (BONOBO_ARG_SET_GENERAL (a, v, _long))
#define BONOBO_ARG_GET_LONG(a)      (BONOBO_ARG_GET_GENERAL (a, _long))
#define BONOBO_ARG_SET_LONG(a,v)    (BONOBO_ARG_SET_GENERAL (a, v, _long))

#define BONOBO_ARG_GET_FLOAT(a)     (BONOBO_ARG_GET_GENERAL (a, _float))
#define BONOBO_ARG_SET_FLOAT(a,v)   (BONOBO_ARG_SET_GENERAL (a, v, _float))

#define BONOBO_ARG_GET_DOUBLE(a)    (BONOBO_ARG_GET_GENERAL (a, _double))
#define BONOBO_ARG_SET_DOUBLE(a,v)  (BONOBO_ARG_SET_GENERAL (a, v, _double))

#define BONOBO_ARG_GET_STRING(a)    (g_assert ((a)->_type->kind == CORBA_tk_string),	\
				     *((CORBA_char **)(a->_value)))
#define BONOBO_ARG_SET_STRING(a,v)  (g_assert ((a)->_type->kind == CORBA_tk_string),	\
				     *((CORBA_char **)(a->_value)) = CORBA_string_dup ((v)?(v):""))

BonoboArg    *bonobo_arg_new           (BonoboArgType t);
void          bonobo_arg_init_default  (BonoboArg    *arg);
void          bonobo_arg_release       (BonoboArg    *arg);
BonoboArg    *bonobo_arg_copy          (BonoboArg    *arg);

void          bonobo_arg_from_gtk      (BonoboArg    *a, const GtkArg       *arg);
BonoboArgType bonobo_arg_type_from_gtk (GtkType       t);
void          bonobo_arg_to_gtk        (GtkArg       *a, const BonoboArg    *arg);

END_GNOME_DECLS

#endif /* ! __BONOBO_ARG_H__ */
