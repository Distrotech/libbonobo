/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-async.h: Code to make hand written
 * async CORBA wrappers easy to write.
 *
 * Author:
 *     Miguel de Icaza (miguel@gnu.org).
 *
 * Copyright 2000 Helix Code, Inc.
 */
#ifndef _BONOBO_ASYNC_H_
#define _BONOBO_ASYNC_H_

#include <glib.h>
#include <orb/orbit.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

typedef enum {
	BONOBO_ASYNC_IN  = 0x1,
	BONOBO_ASYNC_OUT = 0x2
} BonoboASyncFlags;

typedef struct {
	const char           *name;
	const CORBA_TypeCode  ret_type;
	const CORBA_TypeCode *arg_types;
	int                   num_args;
	const CORBA_TypeCode *exceptions;
	BonoboASyncFlags     *flags;
} BonoboASyncMethod;

typedef struct _BonoboASyncReply BonoboASyncReply;

typedef void (*BonoboASyncCallback) (BonoboASyncReply  *reply,
				     CORBA_Environment *ev,
				     gpointer           user_data);

void bonobo_async_demarshal (BonoboASyncReply        *reply,
			     gpointer                 retval,
			     gpointer                *out_args);

void bonobo_async_invoke    (const BonoboASyncMethod *method,
			     BonoboASyncCallback      cb,
			     gpointer                 user_data,
			     guint                    timeout_usec,
			     CORBA_Object             object,
			     gpointer                *args,
			     CORBA_Environment       *ev);

GIOPRecvBuffer *bonobo_async_handle_get_recv (BonoboASyncReply *reply);

END_GNOME_DECLS

#endif /* _BONOBO_ASYNC_H_ */

