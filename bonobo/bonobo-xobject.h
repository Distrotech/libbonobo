/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-xobject.h: Modified Bonobo Unknown interface base implementation
 *
 * Authors:
 *   Michael Meeks (michael@ximian.com)
 *
 * Copyright 2000 Ximian, Inc.
 */
#ifndef _BONOBO_X_OBJECT_H_
#define _BONOBO_X_OBJECT_H_

#include <libgnomebase/gnome-defs.h>
#include <gobject/gobject.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_X_OBJECT_TYPE        (bonobo_x_object_get_type ())
#define BONOBO_X_OBJECT(o)          (G_TYPE_CHECK_INSTANCE_CAST ((o), BONOBO_X_OBJECT_TYPE, BonoboXObject))
#define BONOBO_X_OBJECT_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST((k), BONOBO_X_OBJECT_TYPE, BonoboXObjectClass))
#define BONOBO_IS_X_OBJECT(o)       (G_TYPE_CHECK_INSTANCE_TYPE ((o), BONOBO_X_OBJECT_TYPE))
#define BONOBO_IS_X_OBJECT_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), BONOBO_X_OBJECT_TYPE))

#define BONOBO_X_OBJECT_HEADER_SIZE (sizeof (BonoboObject))

/*
 * Macros to convert between types.
 *  - foolproof versions to follow.
 */
#define BONOBO_X_OBJECT_GET_SERVANT(o) ((PortableServer_Servant)&(o)->servant)
#define BONOBO_X_SERVANT_GET_OBJECT(o) ((BonoboXObject *)((guchar *)(o)				\
					     - BONOBO_X_OBJECT_HEADER_SIZE			\
					     - sizeof (struct CORBA_Object_struct)	\
					     - sizeof (gpointer) * 4))

typedef struct _BonoboXObject BonoboXObject;

/* Detects the pointer type and returns the object reference - magic. */
BonoboXObject *bonobo_x_object (gpointer p);

struct _BonoboXObject {
	BonoboObject               base;
	
	/* Start: BonoboObjectServant */
	BonoboObjectServant        servant;
	int                        flags;        /* discriminant */
	/* End:   BonoboObjectServant */

	gpointer                   dummy;

	/* User data ... */
};

typedef void (*BonoboXObjectPOAFn) (PortableServer_Servant servant,
				    CORBA_Environment     *ev);

typedef struct {
	BonoboObjectClass          parent_class;

	BonoboXObjectPOAFn         poa_init_fn;
	BonoboXObjectPOAFn         poa_fini_fn;

	POA_Bonobo_Unknown__vepv  *vepv;

	/* The offset of this class' additional epv */
	int                        epv_struct_offset;

	POA_Bonobo_Unknown__epv    epv;
} BonoboXObjectClass;

GType          bonobo_x_object_get_type        (void);

/* Use G_STRUCT_OFFSET to calc. epv_struct_offset */
GType          bonobo_x_type_unique            (GType              parent_type,
						BonoboXObjectPOAFn init_fn,
						BonoboXObjectPOAFn fini_fn,
						int                epv_struct_offset,
						const GTypeInfo   *info,
						const gchar       *type_name);

gboolean       bonobo_x_type_setup             (GType              type,
						BonoboXObjectPOAFn init_fn,
						BonoboXObjectPOAFn fini_fn,
						int                epv_struct_offset);
						

#define BONOBO_X_TYPE_FUNC_FULL(class_name, corba_name, parent, prefix)       \
GType                                                                         \
prefix##_get_type (void)                                                      \
{                                                                             \
	GType ptype;                                                          \
	static GType type = 0;                                                \
                                                                              \
	if (type == 0) {                                                      \
		static GTypeInfo info = {                                     \
			sizeof (class_name##Class),                           \
			(GBaseInitFunc) NULL,                                 \
			(GBaseFinalizeFunc) NULL,                             \
			(GClassInitFunc) prefix##_class_init,                 \
			NULL, NULL,                                           \
			sizeof (class_name),                                  \
			0,                                                    \
			(GInstanceInitFunc) prefix##_init                     \
		};                                                            \
		ptype = (parent);                                             \
		type = bonobo_x_type_unique (ptype,                           \
			POA_##corba_name##__init, POA_##corba_name##__fini,   \
			G_STRUCT_OFFSET (class_name##Class, epv),             \
			&info, #class_name);                                  \
	}                                                                     \
	return type;                                                          \
}
 
#define BONOBO_X_TYPE_FUNC(class_name, parent, prefix)                        \
GType                                                                         \
prefix##_get_type (void)                                                      \
{                                                                             \
	GType ptype;                                                          \
	static GType type = 0;                                                \
                                                                              \
	if (type == 0) {                                                      \
		static GTypeInfo info = {                                     \
			sizeof (class_name##Class),                           \
			(GBaseInitFunc) NULL,                                 \
			(GBaseFinalizeFunc) NULL,                             \
			(GClassInitFunc) prefix##_class_init,                 \
			NULL, NULL,                                           \
			sizeof (class_name),                                  \
			0,                                                    \
			(GInstanceInitFunc) prefix##_init                     \
		};                                                            \
		ptype = (parent);                                             \
		type = bonobo_x_type_unique (ptype, NULL, NULL, 0,            \
					     &info, #class_name);             \
	}                                                                     \
	return type;                                                          \
}

END_GNOME_DECLS

#endif
