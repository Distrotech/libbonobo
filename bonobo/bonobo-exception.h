/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_EXCEPTION_H_
#define _BONOBO_EXCEPTION_H_

#include <glib.h>
#include <bonobo/Bonobo.h>

#define BONOBO_EX(ev) ((ev) && (ev)->_major != CORBA_NO_EXCEPTION)

#define BONOBO_RET_EX(ev)		\
	G_STMT_START			\
		if (BONOBO_EX (ev))	\
			return;		\
	G_STMT_END

#define BONOBO_RET_VAL_EX(ev,v)		\
	G_STMT_START			\
		if (BONOBO_EX (ev))	\
			return (v);	\
	G_STMT_END

typedef char *(*BonoboExceptionFn)     (CORBA_Environment *ev, gpointer user_data);

char *bonobo_exception_get_text        (CORBA_Environment *ev);

void  bonobo_exception_add_handler_str (const char *repo_id,
					const char *str);

void  bonobo_exception_add_handler_fn  (const char *repo_id,
					BonoboExceptionFn fn,
					gpointer          user_data,
					GDestroyNotify    destroy_fn);

#endif /* _BONOBO_EXCEPTION_H_ */
