/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * activation-context.h: Context based activation
 *
 * Author:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2003 Ximian, Inc.
 */
#ifndef _ACTIVATION_CONTEXT_H_
#define _ACTIVATION_CONTEXT_H_

#include <bonobo/bonobo-object.h>
#include <bonobo-activation/Bonobo_ActivationContext.h>

G_BEGIN_DECLS

typedef struct _ActivationContext        ActivationContext;
typedef struct _ActivationContextPrivate ActivationContextPrivate;

#define ACTIVATION_TYPE_CONTEXT        (activation_context_get_type ())
#define ACTIVATION_CONTEXT(o)          (G_TYPE_CHECK_INSTANCE_CAST ((bonobo_object (o)), ACTIVATION_TYPE_CONTEXT, ActivationContext))
#define ACTIVATION_CONTEXT_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), ACTIVATION_TYPE_CONTEXT, ActivationContextClass))
#define ACTIVATION_IS_CONTEXT(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), ACTIVATION_TYPE_CONTEXT))
#define ACTIVATION_IS_CONTEXT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), ACTIVATION_TYPE_CONTEXT))

typedef struct _ChildODInfo ChildODInfo;

struct _ActivationContext {
	BonoboObject parent;

	Bonobo_ObjectDirectory obj; /* to die in time */

	int          total_servers;
	Bonobo_ServerInfoList *list;
	Bonobo_CacheTime       time_list_pulled;
	GHashTable            *by_iid;

	/* It is assumed that accesses to this
	 * hash table are atomic - i.e. a CORBA 
	 * call cannot come in while
	 * checking a value in this table */
	GHashTable              *active_servers;
	Bonobo_ServerStateCache *active_server_list;
	Bonobo_CacheTime         time_active_pulled;

	gint         refs; /* nasty re-enterancy guard */
};

typedef struct {
	BonoboObjectClass parent_class;

	POA_Bonobo_ActivationContext__epv epv;
} ActivationContextClass;

GType activation_context_get_type (void) G_GNUC_CONST;
void  activation_context_setup    (PortableServer_POA     poa,
				   Bonobo_ObjectDirectory dir,
				   CORBA_Environment     *ev);
void  activation_context_shutdown (void);

G_END_DECLS

#endif /* _ACTIVATION_CONTEXT_H_ */
