/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * b-object.c: B Unknown interface base implementation
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999,2000 Helix Code, Inc.
 */

#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <bonobo/bonobo-mobject.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-main.h>
#include "bonobo-running-context.h"
#include "bonobo-object-directory.h"

/* Shared stuff with BonoboObject */
extern int bonobo_object_get_refs (BonoboObject *object);

/* Need to examine how ORBit sets up ORBit_RootObject_struct's
	ORBit_RootObject_Interface* interface; 
	guchar is_pseudo_object;
	gint refs;
*/
#define BONOBO_M_GTK_OBJ_FLAG_PATTERN 0x8000
#define BONOBO_M_GTK_FLAG_PATTERN    (GTK_FLOATING | 0x4000)

static GtkObjectClass *m_object_parent_class;

/* FIXME: cut and paste from orbit_object.c: CORBA_Object_release_fn */
static void
b_corba_object_free (BonoboMObject *object)
{
	CORBA_Object obj = BONOBO_M_OBJECT_GET_CORBA (object);

	g_assert (obj!=NULL);

	bonobo_running_context_remove_object (obj);

	ORBIT_ROOT_OBJECT_UNREF (obj);

	if (ORBIT_ROOT_OBJECT (obj)->refs <= 0) {
		g_hash_table_remove (obj->orb->objrefs, obj);
		
		if (obj->connection)
			giop_connection_unref (obj->connection);

		g_free (obj->object_id);
		ORBit_delete_profiles (obj->profile_list);
		ORBit_delete_profiles (obj->forward_locations);

		g_free (obj->vepv); /* FIXME: what ? */
	}
}

static void
m_object_finalize_real (GtkObject *object)
{
	BonoboMObject           *m_object;
	BonoboMObjectClass      *klass;
	void                    *servant;
	PortableServer_ObjectId *oid;
	CORBA_Environment        ev;
	
	m_object = BONOBO_M_OBJECT (object);
	klass = (BonoboMObjectClass *)GTK_OBJECT (object)->klass;
	servant = BONOBO_M_OBJECT_GET_SERVANT (m_object);

	CORBA_exception_init (&ev);

	b_corba_object_free (m_object);

	if (servant) {

		oid = PortableServer_POA_servant_to_id (bonobo_poa(), servant, &ev);
		PortableServer_POA_deactivate_object (bonobo_poa (), oid, &ev);

		if (klass->poa_fini_fn)
			klass->poa_fini_fn (servant, &ev);
		else /* Actualy quicker and nicer */
			PortableServer_ServantBase__fini (servant, &ev);

		CORBA_free (oid);
	}
	CORBA_exception_free (&ev);

	/* Setup the BonoboObject so nothing nasty happens when we chain */
	m_object->base.servant      = NULL;
	m_object->base.corba_objref = CORBA_OBJECT_NIL;

	/* Chain */
	m_object_parent_class->finalize (object);
}

static void
dont_release (gpointer obj, CORBA_Environment *ev)
{
	BonoboMObject *object = BONOBO_M_CORBA_GET_OBJECT (obj);

	g_warning ("Reference counting error: "
		   "Attempts to release CORBA_Object associated with "
		   "'%s' which still has a reference count of %d",
		   gtk_type_name (GTK_OBJECT (object)->klass->type),
		   bonobo_object_get_refs (BONOBO_OBJECT (object)));
}

static void
do_corba_hacks (BonoboMObject      *object,
		BonoboMObjectClass *klass)
{
	CORBA_Object obj;
	CORBA_Environment ev;
	static ORBit_RootObject_Interface ri = { dont_release };

	CORBA_exception_init (&ev);

	/* Setup the servant structure */
	object->servant.servant_placeholder._private = NULL;
	object->servant.servant_placeholder.vepv     = klass->vepv;
	object->servant.bonobo_object                = (BonoboObject *) object;

	/* Initialize the servant structure with our POA__init fn */
	if (!klass->poa_init_fn) {
		g_warning ("It looks like you used gtk_type_unique "
			   "instead of b_type_unique on type '%s'",
			   gtk_type_name (((GtkObjectClass *)klass)->type));
		return;
	}
	klass->poa_init_fn ((PortableServer_Servant) &object->servant, &ev);
	if (BONOBO_EX (&ev)) {
		g_warning ("Exception initializing servant '%s'",
			   bonobo_exception_get_text (&ev));
		return;
	}

	/* Wierdness from bonobo-object */
	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), &object->servant, &ev));

	/* Instantiate a CORBA_Object reference for the servant */
	obj = PortableServer_POA_servant_to_reference (
		bonobo_poa (), &object->servant, &ev);

	/* FIXME: Grab ownership of the CORBA_Object reference by brute force */
	{
		memcpy (&object->object, obj, sizeof (object->object));
		ORBIT_CHUNK_FREE (CORBA_Object, obj);

		/*
		 *  Override the CORBA_Object's release fn, this ensures
		 * that if people CORBA_release more than Unknown_unref
		 * we are still safe.
		 */
		object->object.parent.interface = &ri;
	}

	bonobo_running_context_add_object (BONOBO_M_OBJECT_GET_CORBA (object));

	CORBA_exception_free (&ev);
}

static void
m_object_instance_init (GtkObject    *gtk_object,
			GtkTypeClass *klass)
{
	BonoboMObject *object = BONOBO_M_OBJECT (gtk_object);

	GTK_OBJECT_UNSET_FLAGS (GTK_OBJECT (object), GTK_FLOATING);

	g_warning ("m_object_instance init '%s' '%s'",
		   gtk_type_name (gtk_object->klass->type),
		   gtk_type_name (klass->type));

	do_corba_hacks (object, BONOBO_M_OBJECT_CLASS (klass));

	object->base.corba_objref = BONOBO_M_OBJECT_GET_CORBA   (object);
	object->base.servant      = BONOBO_M_OBJECT_GET_SERVANT (object);
}

static void
m_object_class_init (BonoboMObjectClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;

	m_object_parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->finalize = m_object_finalize_real;
}

/**
 * m_object_get_type:
 *
 * Returns: the GtkType associated with the base BonoboMObject class type.
 */
GtkType
bonobo_m_object_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMObject",
			sizeof (BonoboMObject),
			sizeof (BonoboMObjectClass),
			(GtkClassInitFunc) m_object_class_init,
			(GtkObjectInitFunc) m_object_instance_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (
			bonobo_object_get_type (), &info);
	}

	return type;
}

static gboolean
setup_type (GtkType type, BonoboMObjectPOAFn init_fn, BonoboMObjectPOAFn fini_fn)
{
	GtkType       p, b_type;
	int           depth;
	BonoboMObjectClass *klass;
	gpointer     *vepv;

	b_type = bonobo_m_object_get_type ();

	/* How far down the tree are we ? */
	for (depth = 0, p = type; p && p != b_type;
	     p = gtk_type_parent (p))
		depth++;

	if (!type) {
		g_warning ("Trying to inherit '%s' from a BonoboMObject, but "
			   "no BonoboMObject in the ancestory",
			   gtk_type_name (type));
		return FALSE;
	}

	g_warning ("We are at depth %d with type '%s'",
		   depth, gtk_type_name (type));

	klass = gtk_type_class (type);

	klass->poa_init_fn = init_fn;
	klass->poa_fini_fn = fini_fn;

	{
		POA_Bonobo_Unknown__epv *epv = bonobo_object_get_epv ();
		klass->epv = *epv;
		g_free (epv);
		klass->epv._private = NULL;
	}
	
	vepv = g_new0 (gpointer, depth + 2);
	klass->vepv = (POA_Bonobo_Unknown__vepv *) vepv;
	klass->vepv->_base_epv = NULL;
	klass->vepv->Bonobo_Unknown_epv = &klass->epv;

	/* Build our EPV */
	if (depth > 0) {
		int i;

		p = type;
		for (i = depth; i > 0; i--) {
			GtkTypeQuery *info;

			p = gtk_type_parent (p);
			info = gtk_type_query (p);
			g_return_val_if_fail (info != NULL, FALSE);

			vepv [i + 1] = ((guchar *)klass) + info->class_size;

			g_free (info);
		}
	}

	return TRUE;
}

GtkType
bonobo_m_type_unique (GtkType            parent_type,
		      BonoboMObjectPOAFn init_fn,
		      BonoboMObjectPOAFn fini_fn,
		      const GtkTypeInfo *info)
{
	GtkType       type;

	type = gtk_type_unique (parent_type, info);
	if (!type)
		return 0;

	if (setup_type (type, init_fn, fini_fn))
		return type;
	else
		return 0;
}

BonoboMObject *
bonobo_m_object (gpointer p)
{
	if (!p)
		return NULL;

	/* ... do clever stuff here */

	return NULL;
}
