/*
 * Author:
 *   Nat Friedman (nat@nat.org)
 *
 * Copyright 1999, Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/gnome-main.h>
#include <bonobo/gnome-property-bag.h>
#include <bonobo/gnome-property.h>
#include <bonobo/gnome-property-types.h>
#include <bonobo/gnome-persist-stream.h>

#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>

enum {
	VALUE_CHANGED,
	LAST_SIGNAL
};

static guint gnome_property_bag_signals [LAST_SIGNAL];

POA_GNOME_PropertyBag__vepv gnome_property_bag_vepv;


/*
 * Internal data structures.
 */
struct _GnomePropertyBagPrivate {
	PortableServer_POA	      poa;
	GHashTable		     *props;
	GHashTable		     *types;

	GnomePropertyBagPersisterFn   persister;
	GnomePropertyBagDepersisterFn depersister;
	gpointer		      persister_closure;
};

typedef struct {
	GnomePropertyBagValueMarshalerFn   marshaler;
	GnomePropertyBagValueDemarshalerFn demarshaler;
	GnomePropertyBagValueReleaserFn    releaser;
	GnomePropertyBagValueComparerFn    comparer;
	gpointer                           user_data;
} GnomePropertyType;



/*
 * GnomePropertyBag POA and Servant Manager.
 */

typedef struct {
	POA_PortableServer_ServantLocator servant_locator;
	GnomePropertyBag *property_bag;
} GnomePropertyBagServantManager;

/*
 * This ServantManager method is invoked before a method
 * on a GnomeProperty is called.  It creates the servant
 * for the Property and returns it.
 */
static PortableServer_Servant
gnome_property_servant_locator_preinvoke (PortableServer_Servant servant_manager,
					  PortableServer_ObjectId *oid,
					  PortableServer_POA adapter,
					  CORBA_Identifier op_name,
					  PortableServer_ServantLocator_Cookie *cookie,
					  CORBA_Environment *ev)
{
	GnomePropertyBagServantManager *sm;
	PortableServer_Servant servant;
	GnomePropertyBag *pb;
	char *property_name;

	/*
	 * Get the PropertyBag out of the servant manager.
	 */
	sm = (GnomePropertyBagServantManager *) servant_manager;
	pb = sm->property_bag;

	/*
	 * Grab the Property name and the Property Bag.
	 */
	property_name = PortableServer_ObjectId_to_string (oid, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomePropertyBag: Could not get property name from Object ID");
		return NULL;
	}

	/*
	 * Create a temporary servant for this Property.
	 */
	servant = gnome_property_servant_new (adapter, pb, property_name);
	if (servant == NULL) {
		g_warning ("GnomePropertyBag: Could not create transient Property servant");
		/* FIXME: Set exception */
		return NULL;
	}

	/*
	 * The cookie is arbitrary data which will get passed to
	 * postinvoke for this Property method invocation.  We have no
	 * use for it.
	 */
	*cookie = NULL;

	return servant;
}

/*
 * This method is invoked after a GnomeProperty method invocation.
 * It destroys the transient Property servant.
 */
static void
gnome_property_servant_locator_postinvoke (PortableServer_Servant servant_manager,
					   PortableServer_ObjectId *oid,
					   PortableServer_POA adapter,
					   CORBA_Identifier op_name,
					   PortableServer_ServantLocator_Cookie cookie,
					   PortableServer_Servant servant,
					   CORBA_Environment *ev)
{
	gnome_property_servant_destroy (servant);
}

static PortableServer_ServantBase__epv *
gnome_property_bag_get_servant_base_epv (void)
{
	PortableServer_ServantBase__epv *epv;

	epv = g_new0 (PortableServer_ServantBase__epv, 1);

	epv->default_POA = PortableServer_ServantBase__default_POA;
	epv->finalize    = PortableServer_ServantBase__fini;

	return epv;
}


static POA_PortableServer_ServantManager__epv *
gnome_property_bag_get_servant_manager_epv (void)
{
	POA_PortableServer_ServantManager__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantManager__epv, 1);

	return epv;
}

static POA_PortableServer_ServantLocator__epv *
gnome_property_bag_get_servant_locator_epv (void)
{
	POA_PortableServer_ServantLocator__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantLocator__epv, 1);

	epv->preinvoke  = gnome_property_servant_locator_preinvoke;
	epv->postinvoke = gnome_property_servant_locator_postinvoke;

	return epv;
}

static POA_PortableServer_ServantLocator__vepv *
gnome_property_bag_get_servant_locator_vepv (void)
{
	static POA_PortableServer_ServantLocator__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_PortableServer_ServantLocator__vepv, 1);

	vepv->_base_epv				= gnome_property_bag_get_servant_base_epv ();
	vepv->PortableServer_ServantManager_epv = gnome_property_bag_get_servant_manager_epv ();
	vepv->PortableServer_ServantLocator_epv = gnome_property_bag_get_servant_locator_epv ();

	return vepv;
}

/*
 * Creates the POA and ServantManager which will handle
 * GnomeProperty requests.
 */
static gboolean
gnome_property_bag_create_poa (GnomePropertyBag *pb)
{
        PortableServer_POA                 property_poa = NULL;
	CORBA_PolicyList		  *policies;
	GnomePropertyBagServantManager    *sm;
	CORBA_Environment		   ev;
	char				  *poa_name;

	CORBA_exception_init (&ev);

	/*
	 * Create a new custom POA which will manage the
	 * GnomeProperty objects.  We need a custom POA because there
	 * may be many, many properties and we want to avoid
	 * instantiating a servant for each one of them from the
	 * outset (which is what the default POA will require).
	 *
	 * Our new POA will have a special Policy set --
	 * USE_SERVANT_MANAGER -- which means that, when a request
	 * comes in for one of the objects (properties) managed by
	 * this POA, the ORB will dispatch to our special
	 * ServantManager.  The ServantManager will then incarnate the
	 * Servant for the Property which is being operated on.  So we
	 * are creating a ServantManager which will incarnate Property
	 * servants as-needed, and a POA which knows how to dispatch
	 * to our special ServantManager.
	 *
	 * Repetition is probably the only way to get this across, so
	 * allow me to rephrase: When a request comes in for a particular
	 * object, the POA uses the servant manager to get the servant
	 * for the specified object reference.
	 *
	 * This is just on-demand object creation, mired in a bunch of
	 * fucking CORBA jargon.
	 *
	 * The take home message: Each Bonobo Property CORBA object is
	 * not created until someone actually invokes one of its
	 * methods.
	 */
	
	/*
	 * Create a list of CORBA policies which we will apply to our
	 * new POA.
	 */
	policies = g_new0 (CORBA_PolicyList, 1);
	policies->_maximum = 2;
	policies->_length  = 2;
	policies->_buffer = g_new0 (CORBA_Policy,
				    policies->_length);
	policies->_release = CORBA_FALSE;
	
	/*
	 * Create a new CORBA Policy object which specifies that we
	 * will be using a ServantManager, thank you very much.
	 */
	policies->_buffer [0] = (CORBA_Policy)
		PortableServer_POA_create_request_processing_policy (
			bonobo_poa (),			     /* This argument is ignored. */
			PortableServer_USE_SERVANT_MANAGER,
			&ev);
	
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Could not create request processing policy for GnomeProperty POA");
		g_free (policies->_buffer);
		g_free (policies);
		CORBA_exception_free (&ev);
		return FALSE;
	}
	
	/*
	 * Now, to add a touch more complexity to the whole
	 * system, we go further than just creating Property
	 * servants on-demand; we make them completely transient.
	 * What this means is that, when a Property servant has
	 * finished processing a request on the property object,
	 * it disappears.  So we only use resources on property
	 * servants while a property method invocation is being
	 * processed.  (Now I'm just showing off)
	 *
	 * This is actually important because, with Controls,
	 * properties are used to manipulate many highly-variant
	 * run-time attributes (not just crap like font size).  The
	 * Microsoft ActiveX web controls, for example, use properties
	 * to allow the user (the parent application) to get/set the
	 * current URL being displayed.
	 *
	 * Accordingly, the following CORBA Policy specifies that
	 * servants should not be retained.
	 */
	policies->_buffer [1] = (CORBA_Policy)
		PortableServer_POA_create_servant_retention_policy (
			bonobo_poa (),
			PortableServer_NON_RETAIN,
			&ev);
	
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Could not create servant retention policy for GnomeProperty POA");
		g_free (policies->_buffer);
		g_free (policies);
		CORBA_exception_free (&ev);
		return FALSE;
	}
	
	/*
	 * Create the GnomeProperty POA as a child of the root
	 * Bonobo POA.
	 */
	poa_name = g_strdup_printf ("GnomePropertyBag %p", pb);
	pb->priv->poa =
		PortableServer_POA_create_POA (bonobo_poa (),
					       poa_name,
					       bonobo_poa_manager (),
					       policies,
					       &ev);
	g_free (poa_name);
	
	
	g_free (policies->_buffer);
	g_free (policies);
	
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomePropertyBag: Could not create GnomePropertyBag POA");
		CORBA_exception_free (&ev);
		return FALSE;
	}
	
	property_poa = pb->priv->poa;
	
	/*
	 * Create our ServantManager.
	 */
	sm = g_new0 (GnomePropertyBagServantManager, 1);
	sm->property_bag = pb;

	((POA_PortableServer_ServantLocator *) sm)->vepv = gnome_property_bag_get_servant_locator_vepv ();

	POA_PortableServer_ServantLocator__init (((PortableServer_ServantLocator *) sm), &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomePropertyBag: Could not initialize ServantLocator");
		CORBA_exception_free (&ev);
		g_free (sm);
		return FALSE;
		
	}

	PortableServer_POA_set_servant_manager (pb->priv->poa, (PortableServer_ServantManager) sm, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("GnomePropertyBag: Could not set POA servant manager");
		CORBA_exception_free (&ev);
		g_free (sm);
		return FALSE;
	}

	return TRUE;
}


/*
 * GnomePropertyBag CORBA methods.
 */

static void
gnome_property_bag_foreach_create_list (gpointer key, gpointer value,
					gpointer data)
{
	GList **l = (GList **) data;

	*l = g_list_prepend (*l, value);
}


/**
 * gnome_property_bag_get_prop_list:
 * @pb: A #GnomePropertyBag.
 *
 * Returns a #GList of #GnomeProperty structures.  This function is
 * private and should only be used internally, or in a PropertyBag
 * persistence implementation.  You should not touch the
 * #GnomeProperty structure unless you know what you're doing.
 */
GList *
gnome_property_bag_get_prop_list (GnomePropertyBag *pb)
{
	GList *l;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);

	l = NULL;

	g_hash_table_foreach (pb->priv->props,
			      gnome_property_bag_foreach_create_list,
			      &l);

	return l;
}

static GNOME_Property
gnome_property_bag_create_objref (GnomePropertyBag *pb, char *name,
				  GNOME_Property *obj, CORBA_Environment *ev)
{
	PortableServer_ObjectId *oid;

	oid = PortableServer_string_to_ObjectId (name, ev);

	*obj = (GNOME_Property) PortableServer_POA_create_reference_with_id (
		pb->priv->poa, oid, "IDL:Bonobo/Property:1.0", ev);

	return *obj;
}

static GNOME_PropertyList *
impl_GNOME_PropertyBag_get_properties (PortableServer_Servant servant,
				       CORBA_Environment *ev)
{
	GnomePropertyBag   *pb = GNOME_PROPERTY_BAG (gnome_object_from_servant (servant));
	GNOME_PropertyList *prop_list;
	GList		   *props;
	GList		   *curr;
	int		    len;
	int		    i;

	/*
	 * Create the PropertyList and allocate space for the
	 * properties.
	 */
	len = g_hash_table_size (pb->priv->props);

	prop_list = GNOME_PropertyList__alloc ();
	prop_list->_length = len;

	if (len == 0)
		return prop_list;

	prop_list->_buffer = CORBA_sequence_GNOME_Property_allocbuf (len);

	/*
	 * Create a list of Object references for the properties.
	 */
	props = gnome_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		GnomeProperty *prop = curr->data;
		GNOME_Property objref;

		objref = gnome_property_bag_create_objref (
			pb, prop->name,
			& (prop_list->_buffer [i]), ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			g_warning ("GnomePropertyBag: Could not create property objref!\n");
			g_list_free (props);
			CORBA_free (prop_list);
			return CORBA_OBJECT_NIL;
		}

		i++;
		
	}

	g_list_free (props);

	return prop_list;
}

static GNOME_Property
impl_GNOME_PropertyBag_get_property (PortableServer_Servant servant,
				     CORBA_char *name,
				     CORBA_Environment *ev)
{
	GnomePropertyBag *pb = GNOME_PROPERTY_BAG (gnome_object_from_servant (servant));
	GNOME_Property    prop;

	if (g_hash_table_lookup (pb->priv->props, name) == NULL) {

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_PropertyBag_PropertyNotFound,
				     NULL);

		return CORBA_OBJECT_NIL;
	}

	gnome_property_bag_create_objref (pb, name, &prop, ev);

	return prop;
}

static GNOME_PropertyNames *
impl_GNOME_PropertyBag_get_property_names (PortableServer_Servant servant,
					   CORBA_Environment *ev)
{
	GnomePropertyBag        *pb = GNOME_PROPERTY_BAG (gnome_object_from_servant (servant));
	GNOME_PropertyNames	*name_list;
	GList			*props;
	GList			*curr;
	int                      len;
	int			 i;

	/*
	 * Create the PropertyNames list and allocate space for the
	 * names.
	 */
	len = g_hash_table_size (pb->priv->props);

	name_list = GNOME_PropertyNames__alloc ();
	name_list->_length = len;

	if (len == 0)
		return name_list;

	name_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);

	/*
	 * Create the list of property names.
	 */
	props = gnome_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		GnomeProperty *prop = curr->data;

		name_list->_buffer [i] = CORBA_string_dup (prop->name);
		i ++;
	}

	g_list_free (props);

	return name_list;
}


/*
 * Property streaming hooks.
 */
void
gnome_property_bag_set_persister (GnomePropertyBag              *pb,
				  GnomePropertyBagPersisterFn    persister,
				  GnomePropertyBagDepersisterFn  depersister,
				  gpointer			 user_data)
{
	g_return_if_fail (pb != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG (pb));

	pb->priv->persister         = persister;
	pb->priv->depersister       = depersister;
	pb->priv->persister_closure = user_data;
}

static int
gnome_property_bag_persist_save (GnomePersistStream *ps,
				 const GNOME_Stream stream,
				 void *closure)
{
	GnomePropertyBag *pb = closure;

	if (pb->priv->persister != NULL) {
		if (! (pb->priv->persister) (pb, stream,
					     pb->priv->persister_closure))
			return FALSE;
		else
			return TRUE;
	}

	return TRUE;
}

static int
gnome_property_bag_persist_load (GnomePersistStream *ps,
				 const GNOME_Stream stream,
				 void *closure)
{
	GnomePropertyBag *pb = closure;

	if (pb->priv->depersister != NULL) {
		if (! (pb->priv->depersister) (pb, stream,
					       pb->priv->persister_closure))
			return FALSE;
		else
			return TRUE;
	}

	return TRUE;
}



/*
 * GnomePropertyBag construction/deconstruction functions. 
 */
static GnomePropertyBag *
gnome_property_bag_construct (GnomePropertyBag *pb,
			      CORBA_Object corba_pb)
{
	GnomePersistStream *pstream;

	gnome_object_construct (GNOME_OBJECT (pb), corba_pb);

	if (! gnome_property_bag_create_poa (pb)) {
		g_free (pb);
		return NULL;
	}

	/*
	 * Create the stream which we use to persist/depersist properties.
	 */
	pstream = gnome_persist_stream_new (gnome_property_bag_persist_load,
					    gnome_property_bag_persist_save,
					    pb);

	gnome_object_add_interface (GNOME_OBJECT (pb),
				    GNOME_OBJECT (pstream));


	return pb;
}

static CORBA_Object
gnome_property_bag_create_corba_object (GnomeObject *object)
{
	POA_GNOME_PropertyBag *servant;
	CORBA_Environment      ev;

	servant = (POA_GNOME_PropertyBag *)g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_property_bag_vepv;

	CORBA_exception_init (&ev);

	POA_GNOME_PropertyBag__init ( (PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
		CORBA_exception_free (&ev);
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return gnome_object_activate_servant (object, servant);
}

/**
 * gnome_property_bag_new:
 *
 * Returns: A new #GnomePropertyBag object.
 */
GnomePropertyBag *
gnome_property_bag_new (void)
{
	GnomePropertyBag *pb;
	CORBA_Object      corba_pb;


	pb = gtk_type_new (gnome_property_bag_get_type ());

	corba_pb = gnome_property_bag_create_corba_object (GNOME_OBJECT (pb));
	if (corba_pb == CORBA_OBJECT_NIL) {
		gtk_object_destroy (GTK_OBJECT (pb));
		return NULL;
	}

	return gnome_property_bag_construct (pb, corba_pb);
}

static gboolean
gnome_property_bag_foreach_remove_type (gpointer key, gpointer value,
					gpointer user_data)
{
	GnomePropertyType *type = value;
	char              *type_name = key;

	g_free (type);
	g_free (type_name);

	return TRUE;
}

static gboolean
gnome_property_bag_foreach_remove_prop (gpointer key, gpointer value,
					gpointer user_data)
{
	GnomePropertyBag  *pb        = user_data;
	GnomeProperty     *prop      = value;
	GnomePropertyType *type;
	char              *prop_name = key;

	type = g_hash_table_lookup (pb->priv->types, prop->type);
	g_return_val_if_fail (type != NULL, TRUE);

	(type->releaser) (prop->type, prop->value, type->user_data);
	(type->releaser) (prop->type, prop->default_value, type->user_data);

	g_free (prop_name);
	g_free (prop->docstring);
	g_free (prop->type);

	g_free (prop);

	return TRUE;
}

static void
gnome_property_bag_destroy (GtkObject *object)
{
	GnomePropertyBag *pb = GNOME_PROPERTY_BAG (object);
	CORBA_Environment ev;


	/* Destroy the POA. */
	CORBA_exception_init (&ev);
	PortableServer_POA_destroy (pb->priv->poa, TRUE, TRUE, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("gnome_property_bag_destroy: Could not destroy POA.\n");
	}
	CORBA_exception_free (&ev);

	/* Destroy all properties. */
	g_hash_table_foreach_remove (pb->priv->props,
				     gnome_property_bag_foreach_remove_prop,
				     NULL);
	g_hash_table_destroy (pb->priv->props);

	/* Destroy all the types. */
	g_hash_table_foreach_remove (pb->priv->types,
				     gnome_property_bag_foreach_remove_type,
				     NULL);
	g_hash_table_destroy (pb->priv->types);

	g_free (pb->priv);
	gnome_object_destroy (GNOME_OBJECT (pb));
}

/**
 * gnome_property_bag_get_epv:
 */
POA_GNOME_PropertyBag__epv *
gnome_property_bag_get_epv (void)
{
	POA_GNOME_PropertyBag__epv *epv;

	epv = g_new0 (POA_GNOME_PropertyBag__epv, 1);

	epv->get_properties     = impl_GNOME_PropertyBag_get_properties;
	epv->get_property       = impl_GNOME_PropertyBag_get_property;
	epv->get_property_names = impl_GNOME_PropertyBag_get_property_names;

	return epv;
}



/*
 * GnomePropertyBag property manipulation API.
 */

static gboolean
gnome_property_bag_set_prop_type (GnomePropertyBag *pb, GnomeProperty *prop,
				  const char *type)
{
	if (g_hash_table_lookup (pb->priv->types, type) == NULL) {
		g_warning ("gnome_property_bag_set_prop_type: Type \"%s\" not found\n", type);
		return FALSE;
	}

	prop->type = g_strdup (type);

	return TRUE;
}

/**
 * gnome_property_bag_add:
 */
void
gnome_property_bag_add (GnomePropertyBag *pb, const char *name, const char *type, gpointer value,
			gpointer default_value, const char *docstring, GnomePropertyFlags flags)
{
	GnomeProperty *prop;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG (pb));
	g_return_if_fail (name != NULL);
	g_return_if_fail (type != NULL);

	/*
	 * Check to see if there's already a property with this name.
	 */
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) == NULL);

	/*
	 * Create the new property.
	 */
	prop = g_new0 (GnomeProperty, 1);

	/* Set the property's type. */
	if (! gnome_property_bag_set_prop_type (pb, prop, type)) {
		g_free (prop);
		return;
	}

	/* Set the other fields. */
	prop->name = g_strdup (name);
	prop->value = value;
	prop->default_value = default_value;
	prop->flags = flags;
	
	if (docstring != NULL)
		prop->docstring = g_strdup (docstring);

	/* Stash the property in the property table. */
	g_hash_table_insert (pb->priv->props, prop->name, prop);
}

/**
 * gnome_property_bag_set_value:
 */
void
gnome_property_bag_set_value (GnomePropertyBag *pb, const char *name,
			      gpointer value)
{
	GnomePropertyType *ptype;
	GnomeProperty     *prop;
	gpointer           old_value;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	/* Set the new value. */
	old_value = prop->value;
	prop->value = value;

	/* Notify the user. */
	gtk_signal_emit (GTK_OBJECT (pb), gnome_property_bag_signals [VALUE_CHANGED],
			 prop->name, prop->type, old_value, prop->value);

	/* Release the old value. */
	ptype = g_hash_table_lookup (pb->priv->types, prop->type);
	g_return_if_fail (ptype != NULL);

	(ptype->releaser) (prop->type, old_value, ptype->user_data);
}

/**
 * gnome_property_bag_set_default:
 */
void
gnome_property_bag_set_default (GnomePropertyBag *pb, const char *name,
				gpointer default_value)
{
	GnomeProperty *prop;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	prop->default_value = default_value;
}

/**
 * gnome_property_bag_set_docstring:
 */
void
gnome_property_bag_set_docstring (GnomePropertyBag *pb, const char *name,
				  char *docstring)
{
	GnomeProperty *prop;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (docstring != NULL);
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	g_free (prop->docstring);
	prop->docstring = g_strdup (docstring);
}

/**
 * gnome_property_bag_set_flags:
 */
void
gnome_property_bag_set_flags (GnomePropertyBag *pb, const char *name,
			      GnomePropertyFlags flags)
{
	GnomeProperty *prop;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	prop->flags = flags;
}

/**
 * gnome_property_bag_get_prop_type:
 */
const char *
gnome_property_bag_get_prop_type (GnomePropertyBag *pb, const char *name)
{
	GnomeProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->type;
}

/**
 * gnome_property_bag_get_value:
 */
const gpointer
gnome_property_bag_get_value (GnomePropertyBag *pb, const char *name)
{
	GnomeProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->value;
}

/**
 * gnome_property_bag_get_default:
 */
const gpointer
gnome_property_bag_get_default (GnomePropertyBag *pb, const char *name)
{
	GnomeProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->default_value;
}

/**
 * gnome_property_bag_get_docstring:
 */
const char *
gnome_property_bag_get_docstring (GnomePropertyBag *pb, const char *name)
{
	GnomeProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->docstring;
}

/**
 * gnome_property_bag_get_flags:
 */
const GnomePropertyFlags
gnome_property_bag_get_flags (GnomePropertyBag *pb, const char *name)
{
	GnomeProperty *prop;

	g_return_val_if_fail (pb != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), 0);
	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, 0);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->flags;
}

/**
 * gnome_property_bag_has_property:
 */
gboolean
gnome_property_bag_has_property (GnomePropertyBag *pb, const char *name)
{
	g_return_val_if_fail (pb != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), FALSE);
	g_return_val_if_fail (name != NULL, FALSE);

	if (g_hash_table_lookup (pb->priv->props, name) == NULL)
		return FALSE;

	return TRUE;
}

/*
 * Property types.
 */

/**
 * gnome_property_bag_create_type:
 */
void
gnome_property_bag_create_type (GnomePropertyBag *pb, char *type_name,
				GnomePropertyBagValueMarshalerFn   marshaler,
				GnomePropertyBagValueDemarshalerFn demarshaler,
				GnomePropertyBagValueComparerFn    comparer,
				GnomePropertyBagValueReleaserFn    releaser,
				gpointer                           user_data)
{
	GnomePropertyType *ptype;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG (pb));
	g_return_if_fail (type_name != NULL);
	g_return_if_fail (marshaler != NULL);
	g_return_if_fail (demarshaler != NULL);

	/* Make sure this type doesn't already exist. */
	g_return_if_fail (g_hash_table_lookup (pb->priv->types, type_name) == NULL);

	ptype = g_new0 (GnomePropertyType, 1);
	ptype->marshaler   = marshaler;
	ptype->demarshaler = demarshaler;
	ptype->releaser    = releaser;
	ptype->comparer	   = comparer;
	ptype->user_data   = user_data;

	g_hash_table_insert (pb->priv->types, type_name, ptype);
}

/**
 * gnome_property_bag_get_prop_any:
 */
CORBA_any *
gnome_property_bag_value_to_any (GnomePropertyBag *pb, const char *type,
				 const gpointer value)
{
	GnomePropertyBagValueMarshalerFn  value_marshaler;
	GnomePropertyType                *ptype;
	CORBA_any                        *any;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (type != NULL, NULL);

	ptype = g_hash_table_lookup (pb->priv->types, type);
	g_return_val_if_fail (ptype != NULL, NULL);

	value_marshaler = ptype->marshaler;
	g_return_val_if_fail (value_marshaler != NULL, NULL);

	any = (value_marshaler) (type, value, ptype->user_data);

	return any;
}

/**
 * gnome_property_bag_any_to_value:
 */
gpointer
gnome_property_bag_any_to_value (GnomePropertyBag *pb, const char *type,
				 const CORBA_any *any)
{
	GnomePropertyBagValueDemarshalerFn  value_demarshaler;
	GnomePropertyType                  *ptype;
	gpointer                            value;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (type != NULL, NULL);

	ptype = g_hash_table_lookup (pb->priv->types, type);
	g_return_val_if_fail (ptype != NULL, NULL);

	value_demarshaler = ptype->demarshaler;
	g_return_val_if_fail (value_demarshaler != NULL, NULL);

	value = (value_demarshaler) (type, any, ptype->user_data);

	return value;
}

/**
 * gnome_property_bag_compare_values:
 */
gboolean
gnome_property_bag_compare_values (GnomePropertyBag *pb, const char *type,
				   gpointer value1, gpointer value2)
{
	GnomePropertyBagValueComparerFn  value_comparer;
	GnomePropertyType		*ptype;

	g_return_val_if_fail (pb != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), FALSE);
	g_return_val_if_fail (type != NULL, FALSE);

	ptype = g_hash_table_lookup (pb->priv->types, type);
	g_return_val_if_fail (ptype != NULL, FALSE);

	value_comparer = ptype->comparer;
	g_return_val_if_fail (value_comparer != NULL, FALSE);

	return (value_comparer) (type, value1, value2, ptype->user_data);
}


/*
 * Class/object initialization functions.
 */

typedef void (*GtkSignal_NONE__POINTER_POINTER_POINTER_POINTER) (GtkObject * object,
								 gpointer arg1,
								 gpointer arg2,
								 gpointer arg3,
								 gpointer arg4,
								 gpointer user_data);

static void
gtk_marshal_NONE__POINTER_POINTER_POINTER_POINTER (GtkObject *object,
						   GtkSignalFunc func,
						   gpointer func_data,
						   GtkArg *args)
{
	GtkSignal_NONE__POINTER_POINTER_POINTER_POINTER rfunc;
	rfunc = (GtkSignal_NONE__POINTER_POINTER_POINTER_POINTER) func;

	(*rfunc) (object,
		  GTK_VALUE_POINTER (args[0]),
		  GTK_VALUE_POINTER (args[1]),
		  GTK_VALUE_POINTER (args[2]),
		  GTK_VALUE_POINTER (args[3]),
		  func_data);
}

static void
gnome_property_bag_init_corba_class (void)
{
	gnome_property_bag_vepv.GNOME_Unknown_epv     = gnome_object_get_epv ();
	gnome_property_bag_vepv.GNOME_PropertyBag_epv = gnome_property_bag_get_epv ();
}

static void
gnome_property_bag_class_init (GnomePropertyBagClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	object_class->destroy = gnome_property_bag_destroy;

	gnome_property_bag_signals [VALUE_CHANGED] =
		gtk_signal_new ("value_changed",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET (GnomePropertyBagClass, value_changed),
				gtk_marshal_NONE__POINTER_POINTER_POINTER_POINTER,
				GTK_TYPE_NONE, 4,
				GTK_TYPE_POINTER, GTK_TYPE_POINTER,
				GTK_TYPE_POINTER, GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, gnome_property_bag_signals, LAST_SIGNAL);

	gnome_property_bag_init_corba_class ();
}

static void
gnome_property_bag_init (GnomePropertyBag *pb)
{
	pb->priv = g_new0 (GnomePropertyBagPrivate, 1);

	/*
	 * This hash table will hold GnomeProperty structs.
	 */
	pb->priv->props = g_hash_table_new (g_str_hash, g_str_equal);

	/*
	 * This hash table will map type names to type marshaler
	 * functions.  These functions convert a GnomeProperty
	 * 'value' pointer to a CORBA_any.
	 */
	pb->priv->types = g_hash_table_new (g_str_hash, g_str_equal);

	/* Prime the table with some default types. */
	gnome_property_bag_create_type (pb, "boolean",
					gnome_property_marshal_boolean,
					gnome_property_demarshal_boolean,
					gnome_property_compare_boolean,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "string",
					gnome_property_marshal_string,
					gnome_property_demarshal_string,
					gnome_property_compare_string,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "short",
					gnome_property_marshal_short,
					gnome_property_demarshal_short,
					gnome_property_compare_short,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "ushort",
					gnome_property_marshal_ushort,
					gnome_property_demarshal_ushort,
					gnome_property_compare_ushort,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "long",
					gnome_property_marshal_long,
					gnome_property_demarshal_long,
					gnome_property_compare_long,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "ulong",
					gnome_property_marshal_ulong,
					gnome_property_demarshal_ulong,
					gnome_property_compare_ulong,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "float",
					gnome_property_marshal_float,
					gnome_property_demarshal_float,
					gnome_property_compare_float,
					gnome_property_generic_releaser,
					NULL);
	gnome_property_bag_create_type (pb, "double",
					gnome_property_marshal_double,
					gnome_property_demarshal_double,
					gnome_property_compare_double,
					gnome_property_generic_releaser,
					NULL);
}

/**
 * gnome_property_bag_get_type:
 *
 * Returns: The GtkType corresponding to the GnomePropertyBag class.
 */
GtkType
gnome_property_bag_get_type (void)
{
	static GtkType type = 0;

	if (! type) {
		GtkTypeInfo info = {
			"GnomePropertyBag",
			sizeof (GnomePropertyBag),
			sizeof (GnomePropertyBagClass),
			(GtkClassInitFunc) gnome_property_bag_class_init,
			(GtkObjectInitFunc) gnome_property_bag_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_object_get_type (), &info);
	}

	return type;
}
