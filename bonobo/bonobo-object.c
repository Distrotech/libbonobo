/**
 * GNOME Unknown interface base implementation
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <libgnorba/gnorba.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-unknown.h>
#include "bonobo.h"

struct _GnomeAggregateObject {
	GList *objs;
};

enum {
	QUERY_INTERFACE,
	LAST_SIGNAL
};

static guint gnome_unknown_signals [LAST_SIGNAL];
static GtkObjectClass *gnome_unknown_parent_class;

/* Assumptions made: sizeof(POA_interfacename) does not change between interfaces */

/**
 * gnome_unknown_from_servant:
 * @servant: Your servant.
 *
 * CORBA method implementations receive a parameter of type
 * PortableServer_Servant which is a pointer to the servant that was
 * used to create this specific CORBA object instance
 *
 * This routine allows the user to get the GnomeUnknown (ie, Gtk object
 * wrapper) from the servant.
 * 
 * Returns: the GnomeUnknown wrapper associated to this servant.
 */
GnomeUnknown *
gnome_unknown_from_servant (PortableServer_Servant servant)
{
	g_return_val_if_fail (servant != NULL, NULL);

	return GNOME_UNKNOWN(((GnomeUnknownServant *)servant)->gnome_unknown);
}

/**
 * gnome_unknown_bind_to_servant:
 * @object: the GnomeUnknown to bind to the servant.
 * @servant: A PortableServer_Servant to bind to the GnomeUnknown
 *
 * This routine binds the @object to the @servant.  It establishes a
 * one to one mapping between the @object and the @servant.  Utility
 * routines are provides to go back and forth.
 *
 * This routine is used internally by the
 * gnome_unknown_activate_servant().
 */
void
gnome_unknown_bind_to_servant (GnomeUnknown *object, void *servant)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (servant != NULL);
	g_return_if_fail (GNOME_IS_UNKNOWN (object));

	object->servant = servant;
	((GnomeUnknownServant *)servant)->gnome_unknown = object;
}

static void
impl_GNOME_Unknown__destroy (PortableServer_Servant servant, CORBA_Environment *ev)
{
	POA_GNOME_Unknown__fini ((POA_GNOME_Unknown *)servant, ev);
	g_free (servant);
}

static void
impl_GNOME_Unknown_ref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeUnknown *object;

	object = gnome_unknown_from_servant (servant);
	gtk_object_ref (GTK_OBJECT (object));
	gtk_object_sink (GTK_OBJECT (object));
}

static void
impl_GNOME_Unknown_unref (PortableServer_Servant servant, CORBA_Environment *ev)
{
	GnomeUnknown *object;

	object = gnome_unknown_from_servant (servant);
	gtk_object_unref (GTK_OBJECT (object));
}

static CORBA_Object
impl_GNOME_Unknown_query_interface (PortableServer_Servant servant,
				const CORBA_char *repoid,
				CORBA_Environment *ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	GnomeUnknown *object;
	GtkType type;
	GList *l;
	
	object = gnome_unknown_from_servant (servant);

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);

	gtk_signal_emit (
		GTK_OBJECT (object), gnome_unknown_signals [QUERY_INTERFACE],
		repoid, &retval);

	if (CORBA_Object_is_nil (retval, ev)){
		type = gtk_type_from_name (repoid);
		
		/* Try looking at the gtk types */
		for (l = object->ao->objs; l; l = l->next){
                       GnomeUnknown *tryme = l->data;

                       if ((type && gtk_type_is_a(GTK_OBJECT(tryme)->klass->type, type)) ||
			   !strcmp(tryme->corba_objref->object_id, repoid)){
                               retval = CORBA_Object_duplicate (tryme->corba_objref, ev);
                               break;
                       }
               }
	}
	return retval;
}

PortableServer_ServantBase__epv gnome_unknown_base_epv =
{
	NULL,			/* _private data */
	&impl_GNOME_Unknown__destroy,	/* finalize routine */
	NULL,			/* default_POA routine */
};

POA_GNOME_Unknown__epv gnome_unknown_epv =
{
	NULL,			/* _private */
	&impl_GNOME_Unknown_ref,
	&impl_GNOME_Unknown_unref,
	&impl_GNOME_Unknown_query_interface,
};

POA_GNOME_Unknown__vepv gnome_unknown_vepv = {
	&gnome_unknown_base_epv,
	&gnome_unknown_epv
};

static void
gnome_unknown_destroy (GtkObject *object)
{
	GnomeUnknown *gnome_unknown = GNOME_UNKNOWN (object);
	void *servant = gnome_unknown->servant;
	
	if (gnome_unknown->corba_objref != CORBA_OBJECT_NIL){
		CORBA_Object_release (gnome_unknown->corba_objref, &gnome_unknown->ev);
		PortableServer_POA_deactivate_object (
			bonobo_poa (), servant, &gnome_unknown->ev);
	}
	CORBA_exception_free (&gnome_unknown->ev);

	gnome_unknown->ao->objs = g_list_remove (gnome_unknown->ao->objs, object);
	if (!gnome_unknown->ao->objs)
		g_free (gnome_unknown->ao);
	
	gnome_unknown_parent_class->destroy (object);
}

static void
gnome_unknown_class_init (GnomeUnknownClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_unknown_parent_class = gtk_type_class (gtk_object_get_type ());

	gnome_unknown_signals [QUERY_INTERFACE] =
		gtk_signal_new ("query_interface",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(GnomeUnknownClass,query_interface), 
				gtk_marshal_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 2, GTK_TYPE_POINTER, GTK_TYPE_POINTER); 
	gtk_object_class_add_signals (object_class, gnome_unknown_signals, LAST_SIGNAL);

	object_class->destroy = gnome_unknown_destroy;
}

static void
gnome_unknown_instance_init (GtkObject *gtk_object)
{
	GnomeUnknown *object = GNOME_UNKNOWN (gtk_object);
	
	CORBA_exception_init (&object->ev);
	object->ao = g_new0 (GnomeAggregateObject, 1);
	object->ao->objs = g_list_append (object->ao->objs, object);
}

/**
 * gnome_unknown_get_type:
 *
 * Returns the GtkType associated for the base GnomeUnknown type
 */
GtkType
gnome_unknown_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/obj:1.0",
			sizeof (GnomeUnknown),
			sizeof (GnomeUnknownClass),
			(GtkClassInitFunc) gnome_unknown_class_init,
			(GtkObjectInitFunc) gnome_unknown_instance_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

/** 
 * gnome_unknown_activate_servant:
 * @object: a GnomeUnknown
 * @servant: The servant to activate.
 *
 * This routine activates the @servant which is wrapped inside the
 * @object on the bonobo_poa (which is the default POA).
 *
 * Returns: The CORBA_Object that is wrapped by @object and whose
 * servant is @servant.  Might return CORBA_OBJECT_NIL on failure. 
 */
CORBA_Object
gnome_unknown_activate_servant (GnomeUnknown *object, void *servant)
{
	CORBA_Object o;

	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (GNOME_IS_UNKNOWN (object), CORBA_OBJECT_NIL);
	g_return_val_if_fail (servant != NULL, CORBA_OBJECT_NIL);
	
	CORBA_free (PortableServer_POA_activate_object (
		bonobo_poa (), servant, &object->ev));

	o = PortableServer_POA_servant_to_reference (
		bonobo_poa(), servant, &object->ev);

	if (o){
		gnome_unknown_bind_to_servant (object, servant);
		return o;
	} else
		return CORBA_OBJECT_NIL;
	
}

/**
 * gnome_unknown_construct:
 * @object; The GTK object server wrapper for the CORBA service.
 * @corba_object: the reference to the real CORBA object.
 *
 * Constructs a GnomeUnknown. This method is usually invoked from the
 * construct method for other Gtk-based CORBA wrappers that derive
 * from the GNOME::obj interface
 *
 *
 * This returns a constructed GnomeUnknown. 
 *
 */
GnomeUnknown *
gnome_unknown_construct (GnomeUnknown *object, CORBA_Object corba_object)
{
	g_return_val_if_fail (object != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_UNKNOWN (object), NULL);
	g_return_val_if_fail (corba_object != CORBA_OBJECT_NIL, NULL);

	
/* * This routine assumes ownership of the corba_object that is passed in. */
	object->corba_objref = CORBA_Object_duplicate (corba_object, &object->ev);

	return object;
}

void
gnome_unknown_add_interface (GnomeUnknown *object, GnomeUnknown *newobj)
{
       GnomeAggregateObject *oldao;
       GList *l;

        if (object->ao == newobj->ao)
               return;

       oldao = newobj->ao;

       /* Merge the two AggregateObject lists */
       for (l = newobj->ao->objs; l; l = l->next){
               if (!g_list_find (object->ao->objs, l->data)){
                       object->ao->objs = g_list_prepend (object->ao->objs, l->data);
                       ((GnomeUnknown *)l->data)->ao = object->ao;
               }
       }

       g_list_free (oldao->objs);
       g_free (oldao);
}

CORBA_Object
gnome_unknown_query_interface (GnomeUnknown *object, const char *repo_id)
{
       CORBA_Environment ev;
       CORBA_Object retval;

       CORBA_exception_init(&ev);
       retval = GNOME_Unknown_query_interface (object->corba_objref, (CORBA_char *)repo_id, &ev);
       if(ev._major != CORBA_NO_EXCEPTION)
               retval = CORBA_OBJECT_NIL;
       CORBA_exception_free (&ev);
       return retval;
}

CORBA_Object
gnome_unknown_corba_objref (GnomeUnknown *object)
{
	g_return_val_if_fail (object != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail (GNOME_IS_UNKNOWN (object), NULL);
	
	return object->corba_objref;
}
