/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include <bonobo/bonobo-foreign-object.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-running-context.h>

static void
bonobo_foreign_object_class_init (BonoboForeignObjectClass *klass)
{
}

static void
bonobo_foreign_object_instance_init (GObject    *g_object,
				     GTypeClass *klass)
{
}

GType
bonobo_foreign_object_get_type (void)
{
	static GType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (BonoboForeignObjectClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) bonobo_foreign_object_class_init,
			NULL, /* class_finalize */
			NULL, /* class_data */
			sizeof (BonoboForeignObject),
			0, /* n_preallocs */
			(GInstanceInitFunc) bonobo_foreign_object_instance_init
		};
		
		type = g_type_register_static (BONOBO_TYPE_OBJECT, "BonoboForeignObject",
					       &info, 0);
	}

	return type;
}


BonoboObject *
bonobo_foreign_object_new (CORBA_Object corba_objref)
{
	BonoboObject *object;
#if 0
	CORBA_Environment ev;
#endif

	g_return_val_if_fail (corba_objref != CORBA_OBJECT_NIL, NULL);

	  /* The following type check is deactivated because
	   * CORBA_Object_is_a doesn't seem to work correctly,
	   * considering that it returns TRUE only if the object is
	   * exactly of type Bonobo::Unknown, and FALSE if it is of a
	   * derived type, while the CORBA spec says it should return
	   * TRUE in both cases.  Perhaps there is a bug in ORBit?.. */
#if 0
	CORBA_exception_init (&ev);
	if (!CORBA_Object_is_a (corba_objref, "IDL:Bonobo/Unknown:1.0", &ev)) {
		if (ev._major != CORBA_NO_EXCEPTION)
			g_warning ("CORBA_Object_is_a: %s",
				   bonobo_exception_get_text (&ev));
		else
			g_warning ("bonobo_foreign_object_new: corba_objref"
				   " doesn't have interface Bonobo::Unknown");
		CORBA_exception_free (&ev);
		return NULL;
	}
	CORBA_exception_free (&ev);
#endif
	object = BONOBO_OBJECT (g_object_new (BONOBO_TYPE_FOREIGN_OBJECT, NULL));
	object->corba_objref = CORBA_Object_duplicate (corba_objref, NULL);
	bonobo_running_context_add_object_T (object->corba_objref);
	return object;
}

