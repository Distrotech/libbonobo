/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Modified Bonobo Unknown interface base implementation
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999,2000 Helix Code, Inc.
 */
#ifndef _BONOBO_M_OBJECT_H_
#define _BONOBO_M_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#undef BONOBO_M_OBJECT_DEBUG
 
#define BONOBO_M_OBJECT_TYPE        (bonobo_m_object_get_type ())
#define BONOBO_M_OBJECT(o)          (GTK_CHECK_CAST ((o), BONOBO_M_OBJECT_TYPE, BonoboMObject))
#define BONOBO_M_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_M_OBJECT_TYPE, BonoboMObjectClass))
#define BONOBO_MIS_OBJECT(o)       (GTK_CHECK_TYPE ((o), BONOBO_M_OBJECT_TYPE))
#define BONOBO_MIS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_M_OBJECT_TYPE))

#define BONOBO_M_OBJECT_HEADER_SIZE (sizeof (BonoboObject))

/*
 * Macros to convert between types.
 *  - foolproof versions to follow.
 */
#define BONOBO_M_OBJECT_GET_CORBA(o)   ((CORBA_Object)&(o)->object)
#define BONOBO_M_OBJECT_GET_SERVANT(o) ((PortableServer_Servant)&(o)->servant)
#define BONOBO_M_CORBA_GET_OBJECT(o)   ((BonoboMObject *)((guchar *)(o)				\
					     - BONOBO_M_OBJECT_HEADER_SIZE))
#define BONOBO_M_CORBA_GET_SERVANT(o)  ((BonoboMObject *)((guchar *)(o)				\
					     + sizeof (struct CORBA_Object_struct)	\
					     + sizeof (gpointer) * 4))
#define BONOBO_M_SERVANT_GET_CORBA(o)  ((BonoboMObject *)((guchar *)(o)				\
					     - sizeof (struct CORBA_Object_struct)	\
					     - sizeof (gpointer) * 4))
#define BONOBO_M_SERVANT_GET_OBJECT(o) ((BonoboMObject *)((guchar *)(o)				\
					     - BONOBO_M_OBJECT_HEADER_SIZE			\
					     - sizeof (struct CORBA_Object_struct)	\
					     - sizeof (gpointer) * 4))

typedef struct _BonoboMObject BonoboMObject;

/* Detects the pointer type and returns the object reference - magic. */
BonoboMObject *bonobo_m_object (gpointer p);

struct _BonoboMObject {
	BonoboObject               base;

	/* Start: CORBA_Object */
	struct CORBA_Object_struct object;
	gpointer                   bincompat[4]; /* expansion */
	/* End:   CORBA_Object */
	
	/* Start: BonoboObjectServant */
	BonoboObjectServant        servant;
	int                        flags;        /* discriminant */
	/* End:   BonoboObjectServant */

	gpointer                   dummy;

	/* User data ... */
};

typedef void (*BonoboMObjectPOAFn) (PortableServer_Servant servant,
				    CORBA_Environment     *ev);

typedef struct {
	BonoboObjectClass          parent_class;

	BonoboMObjectPOAFn         poa_init_fn;
	BonoboMObjectPOAFn         poa_fini_fn;

	POA_Bonobo_Unknown__vepv  *vepv;

	POA_Bonobo_Unknown__epv    epv;

	/* Inherited vpev methods ... */
} BonoboMObjectClass;

GtkType        bonobo_m_object_get_type        (void);
GtkType        bonobo_m_type_unique            (GtkType            parent_type,
						BonoboMObjectPOAFn init_fn,
						BonoboMObjectPOAFn fini_fn,
						const GtkTypeInfo *info);

END_GNOME_DECLS

#endif

