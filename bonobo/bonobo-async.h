/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-async.h: Code to make async invocations
 * of synchronous methods easier to perform.
 *
 * Author:
 *     Michael Meeks (michael@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#ifndef _BONOBO_ASYNC_H_
#define _BONOBO_ASYNC_H_

#include <glib.h>
#include <orbit/orbit.h>

G_BEGIN_DECLS

#if 0
typedef void (*BonoboAsyncCallback) (BonoboAsyncReply  *reply,
				     gpointer           retval,
				     gpointer          *out_args,
				     CORBA_Environment *ev,
				     gpointer           user_data);

void bonobo_async_invoke    (const ORBit_IMethod       *method,
			     BonoboAsyncCallback        cb,
			     gpointer                   user_data,
			     guint                      timeout_msec,
			     CORBA_Object               object,
			     gpointer                  *args,
			     CORBA_Environment         *ev);
#endif
G_END_DECLS

#endif /* _BONOBO_ASYNC_H_ */

