/**
 * bonobo-property-bag.c: property bag object implementation.
 *
 * Authors:
 *   Nat Friedman  (nat@helixcode.com)
 *   Michael Meeks (michael@helixcode.com)
 *
 * Copyright 1999, 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-property.h>
#include <bonobo/bonobo-property-types.h>
#include <bonobo/bonobo-persist-stream.h>

POA_Bonobo_PropertyBag__vepv bonobo_property_bag_vepv;
static GtkObjectClass *parent_class = NULL;


/*
 * Internal data structures.
 */
struct _BonoboPropertyBagPrivate {
	PortableServer_POA  poa;

	GHashTable         *props;

	BonoboPropertySetFn set_prop;
	BonoboPropertyGetFn get_prop;
	gpointer            user_data;

	GSList             *listeners;
};


/*
 * BonoboPropertyBag POA and Servant Manager.
 */

typedef struct {
	POA_PortableServer_ServantLocator servant_locator;
	BonoboPropertyBag                *property_bag;
} BonoboPropertyBagServantManager;

/*
 * This ServantManager method is invoked before a method
 * on a BonoboProperty is called.  It creates the servant
 * for the Property and returns it.
 */
static PortableServer_Servant
bonobo_property_servant_locator_preinvoke (PortableServer_Servant servant_manager,
					   PortableServer_ObjectId *oid,
					   PortableServer_POA adapter,
					   CORBA_Identifier op_name,
					   PortableServer_ServantLocator_Cookie *cookie,
					   CORBA_Environment *ev)
{
	BonoboPropertyBagServantManager *sm;
	PortableServer_Servant servant;
	BonoboPropertyBag *pb;
	char *property_name;

	/*
	 * Get the PropertyBag out of the servant manager.
	 */
	sm = (BonoboPropertyBagServantManager *) servant_manager;
	pb = sm->property_bag;

	/*
	 * Grab the Property name and the Property Bag.
	 */
	property_name = PortableServer_ObjectId_to_string (oid, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBag: Could not get property name from Object ID");
		return NULL;
	}

	/*
	 * Create a temporary servant for this Property.
	 */
	servant = bonobo_property_servant_new (adapter, pb, property_name);
	if (servant == NULL) {
		g_warning ("BonoboPropertyBag: Could not create transient Property servant");
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		CORBA_free (property_name);
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
 * This method is invoked after a BonoboProperty method invocation.
 * It destroys the transient Property servant.
 */
static void
bonobo_property_servant_locator_postinvoke (PortableServer_Servant servant_manager,
					    PortableServer_ObjectId *oid,
					    PortableServer_POA adapter,
					    CORBA_Identifier op_name,
					    PortableServer_ServantLocator_Cookie cookie,
					    PortableServer_Servant servant,
					    CORBA_Environment *ev)
{
	bonobo_property_servant_destroy (servant);
}

static PortableServer_ServantBase__epv *
bonobo_property_bag_get_servant_base_epv (void)
{
	PortableServer_ServantBase__epv *epv;

	epv = g_new0 (PortableServer_ServantBase__epv, 1);

	epv->default_POA = PortableServer_ServantBase__default_POA;
	epv->finalize    = PortableServer_ServantBase__fini;

	return epv;
}


static POA_PortableServer_ServantManager__epv *
bonobo_property_bag_get_servant_manager_epv (void)
{
	POA_PortableServer_ServantManager__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantManager__epv, 1);

	return epv;
}

static POA_PortableServer_ServantLocator__epv *
bonobo_property_bag_get_servant_locator_epv (void)
{
	POA_PortableServer_ServantLocator__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantLocator__epv, 1);

	epv->preinvoke  = bonobo_property_servant_locator_preinvoke;
	epv->postinvoke = bonobo_property_servant_locator_postinvoke;

	return epv;
}

static POA_PortableServer_ServantLocator__vepv *
bonobo_property_bag_get_servant_locator_vepv (void)
{
	static POA_PortableServer_ServantLocator__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_PortableServer_ServantLocator__vepv, 1);

	vepv->_base_epv				= bonobo_property_bag_get_servant_base_epv ();
	vepv->PortableServer_ServantManager_epv = bonobo_property_bag_get_servant_manager_epv ();
	vepv->PortableServer_ServantLocator_epv = bonobo_property_bag_get_servant_locator_epv ();

	return vepv;
}

/*
 * Creates the POA and ServantManager which will handle
 * BonoboProperty requests.
 */
static gboolean
bonobo_property_bag_create_poa (BonoboPropertyBag *pb)
{
        PortableServer_POA                 property_poa = NULL;
	CORBA_PolicyList		  *policies;
	BonoboPropertyBagServantManager    *sm;
	CORBA_Environment		   ev;
	char				  *poa_name;

	CORBA_exception_init (&ev);

	/*
	 * Create a new custom POA which will manage the
	 * BonoboProperty objects.  We need a custom POA because there
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
	 * CORBA jargon.
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
	policies->_maximum = 3;
	policies->_length  = 3;
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
		g_warning ("Could not create request processing policy for BonoboProperty POA");
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
		g_warning ("Could not create servant retention policy for BonoboProperty POA");
		g_free (policies->_buffer);
		g_free (policies);
		CORBA_exception_free (&ev);
		return FALSE;
	}

	/*
	 * Set the threading model to SINGLE_THREAD_MODEL, otherwise
	 * an ORB could use the default ORB_CTRL_MODEL, which is to
	 * let the ORB make threads for requests as it likes.
	 */
	policies->_buffer [2] = (CORBA_Policy)
		PortableServer_POA_create_thread_policy (
			bonobo_poa (),
			PortableServer_SINGLE_THREAD_MODEL,
			&ev);
	
	if (ev._major != CORBA_NO_EXCEPTION){
		g_warning ("Could not create threading policy for BonoboProperty POA");
		g_free (policies->_buffer);
		g_free (policies);
		CORBA_exception_free (&ev);
		return FALSE;
	}   

	/*
	 * Create the BonoboProperty POA as a child of the root
	 * Bonobo POA.
	 */
	poa_name = g_strdup_printf ("BonoboPropertyBag %p", pb);
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
		g_warning ("BonoboPropertyBag: Could not create BonoboPropertyBag POA");
		CORBA_exception_free (&ev);
		return FALSE;
	}
	
	property_poa = pb->priv->poa;
	
	/*
	 * Create our ServantManager.
	 */
	sm = g_new0 (BonoboPropertyBagServantManager, 1);
	sm->property_bag = pb;

	((POA_PortableServer_ServantLocator *) sm)->vepv = bonobo_property_bag_get_servant_locator_vepv ();
		
	POA_PortableServer_ServantLocator__init (((PortableServer_ServantLocator *) sm), &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBag: Could not initialize ServantLocator");
		CORBA_exception_free (&ev);
		g_free (sm);
		return FALSE;
		
	}

	PortableServer_POA_set_servant_manager (pb->priv->poa, (PortableServer_ServantManager) sm, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboPropertyBag: Could not set POA servant manager");
		CORBA_exception_free (&ev);
		g_free (sm);
		return FALSE;
	}

	return TRUE;
}


/*
 * BonoboPropertyBag CORBA methods.
 */

static void
bonobo_property_bag_foreach_create_list (gpointer key, gpointer value,
					gpointer data)
{
	GList **l = (GList **) data;

	*l = g_list_prepend (*l, value);
}


/**
 * bonobo_property_bag_get_prop_list:
 * @pb: A #BonoboPropertyBag.
 *
 * Returns a #GList of #BonoboProperty structures.  This function is
 * private and should only be used internally, or in a PropertyBag
 * persistence implementation.  You should not touch the
 * #BonoboProperty structure unless you know what you're doing.
 */
GList *
bonobo_property_bag_get_prop_list (BonoboPropertyBag *pb)
{
	GList *l;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL);

	l = NULL;

	g_hash_table_foreach (pb->priv->props,
			      bonobo_property_bag_foreach_create_list,
			      &l);

	return l;
}

static Bonobo_Property
bonobo_property_bag_create_objref (BonoboPropertyBag  *pb,
				   const char        *name,
				   Bonobo_Property    *obj,
				   CORBA_Environment *ev)
{
	PortableServer_ObjectId *oid;

	oid = PortableServer_string_to_ObjectId ((char *) name, ev);

	*obj = (Bonobo_Property) PortableServer_POA_create_reference_with_id (
		pb->priv->poa, oid, "IDL:Bonobo/Property:1.0", ev);

	return *obj;
}

static Bonobo_PropertyList *
impl_Bonobo_PropertyBag_getProperties (PortableServer_Servant  servant,
				       CORBA_Environment      *ev)
{
	BonoboPropertyBag   *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_PropertyList *prop_list;
	GList		   *props;
	GList		   *curr;
	int		    len;
	int		    i;

	/*
	 * Create the PropertyList and allocate space for the
	 * properties.
	 */
	len = g_hash_table_size (pb->priv->props);

	prop_list = Bonobo_PropertyList__alloc ();
	prop_list->_length = len;

	if (len == 0)
		return prop_list;

	prop_list->_buffer = CORBA_sequence_Bonobo_Property_allocbuf (len);

	/*
	 * Create a list of Object references for the properties.
	 */
	props = bonobo_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		BonoboProperty *prop = curr->data;
		Bonobo_Property objref;

		objref = bonobo_property_bag_create_objref (
			pb, prop->name,
			& (prop_list->_buffer [i]), ev);

		if (ev->_major != CORBA_NO_EXCEPTION) {
			g_warning ("BonoboPropertyBag: Could not create property objref!");
			g_list_free (props);
			CORBA_free (prop_list);
			return CORBA_OBJECT_NIL;
		}

		i++;
		
	}

	g_list_free (props);

	return prop_list;
}

static Bonobo_Property
impl_Bonobo_PropertyBag_getPropertyByName (PortableServer_Servant servant,
					   const CORBA_char      *name,
					   CORBA_Environment     *ev)
{
	BonoboPropertyBag *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_Property    prop;

	if (g_hash_table_lookup (pb->priv->props, name) == NULL) {

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyBag_NotFound,
				     NULL);

		return CORBA_OBJECT_NIL;
	}

	bonobo_property_bag_create_objref (pb, name, &prop, ev);

	return prop;
}

static Bonobo_PropertyNames *
impl_Bonobo_PropertyBag_getPropertyNames (PortableServer_Servant servant,
					  CORBA_Environment     *ev)
{
	BonoboPropertyBag        *pb = BONOBO_PROPERTY_BAG (bonobo_object_from_servant (servant));
	Bonobo_PropertyNames	*name_list;
	GList			*props;
	GList			*curr;
	int                      len;
	int			 i;

	/*
	 * Create the PropertyNames list and allocate space for the
	 * names.
	 */
	len = g_hash_table_size (pb->priv->props);

	name_list = Bonobo_PropertyNames__alloc ();
	name_list->_length = len;

	if (len == 0)
		return name_list;

	name_list->_buffer = CORBA_sequence_CORBA_string_allocbuf (len);

	/*
	 * Create the list of property names.
	 */
	props = bonobo_property_bag_get_prop_list (pb);

	i = 0;
	for (curr = props; curr != NULL; curr = curr->next) {
		BonoboProperty *prop = curr->data;

		name_list->_buffer [i] = CORBA_string_dup (prop->name);
		i ++;
	}

	g_list_free (props);

	return name_list;
}

static void
impl_Bonobo_PropertyBag_addChangeListener (PortableServer_Servant        servant,
					   const CORBA_char             *name,
					   const Bonobo_PropertyListener listener,
					   CORBA_Environment            *ev)
{
	BonoboPropertyBag *pb = BONOBO_PROPERTY_BAG (
		bonobo_object_from_servant (servant));

	bonobo_property_bag_add_listener (pb, name, listener, ev);
}
						     
static void
impl_Bonobo_PropertyBag_removeChangeListener (PortableServer_Servant        servant,
					      const CORBA_char             *name,
					      const Bonobo_PropertyListener listener,
					      CORBA_Environment            *ev)
{
	BonoboPropertyBag *pb = BONOBO_PROPERTY_BAG (
		bonobo_object_from_servant (servant));

	bonobo_property_bag_remove_listener (pb, name, listener, ev);
}


/*
 * BonoboPropertyBag construction/deconstruction functions. 
 */
static BonoboPropertyBag *
bonobo_property_bag_construct (BonoboPropertyBag *pb,
			       CORBA_Object corba_pb)
{
	bonobo_object_construct (BONOBO_OBJECT (pb), corba_pb);

	if (! bonobo_property_bag_create_poa (pb)) {
		g_free (pb);
		return NULL;
	}

	return pb;
}

static CORBA_Object
bonobo_property_bag_create_corba_object (BonoboObject *object)
{
	POA_Bonobo_PropertyBag *servant;
	CORBA_Environment      ev;

	servant = (POA_Bonobo_PropertyBag *)g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_property_bag_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_PropertyBag__init ( (PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
		CORBA_exception_free (&ev);
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_property_bag_new:
 *
 * Returns: A new #BonoboPropertyBag object.
 */
BonoboPropertyBag *
bonobo_property_bag_new (BonoboPropertyGetFn get_prop,
			 BonoboPropertySetFn set_prop,
			 gpointer            user_data)
{
	BonoboPropertyBag *pb;
	CORBA_Object      corba_pb;

	pb = gtk_type_new (BONOBO_PROPERTY_BAG_TYPE);

	pb->priv->set_prop  = set_prop;
	pb->priv->get_prop  = get_prop;
	pb->priv->user_data = user_data;

	corba_pb = bonobo_property_bag_create_corba_object (BONOBO_OBJECT (pb));
	if (corba_pb == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (pb));
		return NULL;
	}

	return bonobo_property_bag_construct (pb, corba_pb);
}

static void
bonobo_property_destroy (BonoboProperty *prop)
{
	g_free (prop->name);
	prop->idx = -1;

	bonobo_arg_release (prop->default_value);

	/* Unref all listeners */
	while (prop->listeners) {
		Bonobo_PropertyListener listener = prop->listeners->data;
		
		prop->listeners =
			g_slist_remove (prop->listeners, listener);

		bonobo_object_release_unref (listener, NULL);
	}

	g_free (prop->docstring);

	g_free (prop);
}

static gboolean
bonobo_property_bag_foreach_remove_prop (gpointer key, gpointer value,
					 gpointer user_data)
{
	bonobo_property_destroy (value);

	return TRUE;
}

static void
bonobo_property_bag_destroy (GtkObject *object)
{
	BonoboPropertyBag *pb = BONOBO_PROPERTY_BAG (object);
	CORBA_Environment ev;

	/* Destroy the POA. */
	CORBA_exception_init (&ev);
	PortableServer_POA_destroy (pb->priv->poa, TRUE, TRUE, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		g_warning ("bonobo_property_bag_destroy: Could not destroy POA.");
	CORBA_exception_free (&ev);

	/* Destroy all properties. */
	g_hash_table_foreach_remove (pb->priv->props,
				     bonobo_property_bag_foreach_remove_prop,
				     NULL);
	g_hash_table_destroy (pb->priv->props);

	/* Unref all listeners */
	while (pb->priv->listeners) {
		Bonobo_PropertyListener listener = pb->priv->listeners->data;
		
		pb->priv->listeners =
			g_slist_remove (pb->priv->listeners, listener);

		bonobo_object_release_unref (listener, NULL);
	}

	g_free (pb->priv);

	parent_class->destroy (object);
}

/**
 * bonobo_property_bag_get_epv:
 */
POA_Bonobo_PropertyBag__epv *
bonobo_property_bag_get_epv (void)
{
	POA_Bonobo_PropertyBag__epv *epv;

	epv = g_new0 (POA_Bonobo_PropertyBag__epv, 1);

	epv->getProperties        = impl_Bonobo_PropertyBag_getProperties;
	epv->getPropertyByName    = impl_Bonobo_PropertyBag_getPropertyByName;
	epv->getPropertyNames     = impl_Bonobo_PropertyBag_getPropertyNames;
	epv->addChangeListener    = impl_Bonobo_PropertyBag_addChangeListener;
	epv->removeChangeListener = impl_Bonobo_PropertyBag_removeChangeListener;

	return epv;
}



/*
 * BonoboPropertyBag property manipulation API.
 */

void
bonobo_property_bag_add_full (BonoboPropertyBag  *pb,
			      const char         *name,
			      int                 idx,
			      BonoboArgType       type,
			      BonoboArg          *default_value,
			      const char         *docstring,
			      BonoboPropertyFlags flags,
			      BonoboPropertyGetFn get_prop,
			      BonoboPropertySetFn set_prop,
			      gpointer            user_data)
{
	BonoboProperty *prop;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (type != NULL);
	g_return_if_fail (BONOBO_IS_PROPERTY_BAG (pb));
	g_return_if_fail (g_hash_table_lookup (pb->priv->props, name) == NULL);
			    
	if (((flags & BONOBO_PROPERTY_READABLE)  && !get_prop) ||
	    ((flags & BONOBO_PROPERTY_WRITEABLE) && !set_prop)) {
		g_warning ("Serious property error, missing get/set fn. "
			   "on %s", name);
		return;
	}

	if (!(flags & BONOBO_PROPERTY_READABLE) && default_value)
		g_warning ("Assigning a default value to a non readable "
			   "property '%s'", name);

	prop = g_new0 (BonoboProperty, 1);

	prop->name          = g_strdup (name);
	prop->idx           = idx;
	prop->type          = type;
	prop->default_value = default_value;
	prop->docstring     = g_strdup (docstring);
	prop->flags         = flags;

	prop->get_prop      = get_prop;
	prop->set_prop      = set_prop;
	prop->user_data     = user_data;

	g_hash_table_insert (pb->priv->props, prop->name, prop);
}

static BonoboPropertyFlags
flags_gtk_to_bonobo (guint flags)
{
	BonoboPropertyFlags f = 0;

	if (!flags & GTK_ARG_READABLE)
		f |= BONOBO_PROPERTY_READABLE;

	if (!flags & GTK_ARG_WRITABLE)
		f |= BONOBO_PROPERTY_WRITEABLE;

	return f;
}

#define BONOBO_GTK_MAP_KEY "BonoboGtkMapKey"

static void
get_prop (BonoboPropertyBag *bag,
	  BonoboArg         *arg,
	  guint              arg_id,
	  gpointer           user_data)
{
	GtkArg *gtk_arg = user_data;
	GtkArg  new;
	GtkObject *obj;

	obj = gtk_object_get_data (GTK_OBJECT (bag),
				   BONOBO_GTK_MAP_KEY);

	g_return_if_fail (obj != NULL);
	
/*	g_warning ("Get prop ... %d: %s", arg_id, gtk_arg->name);*/

	new.type = gtk_arg->type;
	new.name = gtk_arg->name;
	gtk_object_getv (obj, 1, &new);

	bonobo_arg_from_gtk (arg, &new);

	if (new.type == GTK_TYPE_STRING &&
	    GTK_VALUE_STRING (new))
		g_free (GTK_VALUE_STRING (new));
}

static void
set_prop (BonoboPropertyBag *bag,
	  const BonoboArg   *arg,
	  guint              arg_id,
	  gpointer           user_data)
{
	GtkArg *gtk_arg = user_data;
	GtkArg  new;
	GtkObject *obj;

	obj = gtk_object_get_data (GTK_OBJECT (bag),
				   BONOBO_GTK_MAP_KEY);

	g_return_if_fail (obj != NULL);
	
/*	g_warning ("Set prop ... %d: %s", arg_id, gtk_arg->name);*/

	new.type = gtk_arg->type;
	new.name = gtk_arg->name;
	bonobo_arg_to_gtk (&new, arg);

	gtk_object_setv (obj, 1, &new);
}

void
bonobo_property_bag_add_gtk_args (BonoboPropertyBag  *pb,
				  GtkObject          *object)
{
	GtkArg  *args, *arg;
	guint32 *arg_flags;
	int      nargs = 0;
	int      i;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_OBJECT (object));

	if (gtk_object_get_data (GTK_OBJECT (pb),
				 BONOBO_GTK_MAP_KEY)) {
		g_warning ("Cannot proxy two gtk objects in the same bag yet");
		return;
	}

	gtk_object_set_data (GTK_OBJECT (pb),
			     BONOBO_GTK_MAP_KEY, object);
	/*
	 * FIXME: we should do this on a per class basis perhaps.
	 */
	args = gtk_object_query_args (GTK_OBJECT_TYPE (object),
				      &arg_flags, &nargs);
	
	if (!nargs) {
		g_warning ("Strange, no Gtk arguments to map to Bonobo");
		return;
	}

	arg = args;
	/* Setup types, and names */
	for (i = 0; i < nargs; arg++, i++) {
		BonoboPropertyFlags flags;
		BonoboArgType       type;
		char               *desc;

		type = bonobo_arg_type_from_gtk (arg->type);
		if (!type) {
			g_warning ("Can't handle type '%s' on arg '%s'",
				   gtk_type_name (arg->type),
				   arg->name);
			continue;
		}

		flags = flags_gtk_to_bonobo (arg_flags [i]);

		desc = g_strconcat (arg->name, " is a ",
				    gtk_type_name (arg->type), NULL);

		g_warning ("Mapping '%s'", desc);
		bonobo_property_bag_add_full (pb, arg->name, i, type,
					      NULL, desc, flags,
					      get_prop, set_prop, arg);
		g_free (desc);
	}

/* FIXME: leaks like a privatised water company */
/*	g_free (args);*/
	g_free (arg_flags);
}

void
bonobo_property_bag_add (BonoboPropertyBag  *pb,
			 const char         *name,
			 int                 idx,
			 BonoboArgType       type,
			 BonoboArg          *default_value,
			 const char         *docstring,
			 BonoboPropertyFlags flags)
{
	g_return_if_fail (pb != NULL);
	g_return_if_fail (pb->priv != NULL);

	return bonobo_property_bag_add_full (pb, name, idx, type,
					     default_value, docstring, flags,
					     pb->priv->get_prop,
					     pb->priv->set_prop,
					     pb->priv->user_data);
}

static void
notify_listeners (BonoboPropertyBag *pb,
		  BonoboProperty    *prop,
		  const BonoboArg   *new_value,
		  CORBA_Environment *ev)
{
	GSList *l;
	CORBA_Environment *real_ev, tmp_ev;

	if (prop->flags & BONOBO_PROPERTY_NO_LISTENING)
		return;
	
	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	/* Notify the bag listeners. */
	for (l = pb->priv->listeners; l; l = l->next) {
		Bonobo_PropertyListener_event ((Bonobo_PropertyListener) l->data, 
					       prop->name, new_value, real_ev);

		if (BONOBO_EX (real_ev))
			break;
	}

	if (BONOBO_EX (real_ev)) {
		if (!ev) {
			g_warning ("Listener exception '%s' occured.",
				   bonobo_exception_get_text (real_ev));
			CORBA_exception_free (&tmp_ev);
		}
		return;
	}

	/* Notify the property listeners. */
	for (l = prop->listeners; l; l = l->next) {
		Bonobo_PropertyListener_event ((Bonobo_PropertyListener) l->data, 
					       prop->name, new_value, real_ev);

		if (BONOBO_EX (real_ev))
			break;
	}

	if (BONOBO_EX (real_ev)) {
		if (!ev) {
			g_warning ("Listener exception '%s' occured.",
				   bonobo_exception_get_text (real_ev));
			CORBA_exception_free (&tmp_ev);
		}
		return;
	}

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

void
bonobo_property_bag_notify_listeners (BonoboPropertyBag *pb,
				      const char        *name,
				      const BonoboArg   *new_value,
				      CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;
	CORBA_Environment ev, *my_ev;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (pb->priv != NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	g_return_if_fail (prop != NULL);
	g_return_if_fail (bonobo_arg_type_is_equal (prop->type, new_value->_type, opt_ev));

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	notify_listeners (pb, prop, new_value, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);
}

void
bonobo_property_bag_set_value (BonoboPropertyBag *pb,
			       const char        *name,
			       const BonoboArg   *value,
			       CORBA_Environment *opt_ev)
{
	BonoboProperty *prop;
	CORBA_Environment ev, *my_ev;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (pb->priv != NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	g_return_if_fail (prop != NULL);
	g_return_if_fail (prop->set_prop != NULL);
	g_return_if_fail (bonobo_arg_type_is_equal (prop->type, value->_type, opt_ev));

	prop->set_prop (pb, value, prop->idx, prop->user_data);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	notify_listeners (pb, prop, value, my_ev);

	if (!opt_ev)
		CORBA_exception_free (&ev);
}

/**
 * bonobo_property_bag_get_value:
 */
BonoboArg *
bonobo_property_bag_get_value (BonoboPropertyBag *pb, const char *name)
{
	BonoboProperty *prop;
	BonoboArg      *arg;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (pb->priv != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);

	g_return_val_if_fail (prop != NULL, NULL);
	g_return_val_if_fail (prop->get_prop != NULL, NULL);

	arg = bonobo_arg_new (prop->type);

	prop->get_prop (pb, arg, prop->idx, prop->user_data);

	return arg;
}

BonoboArgType
bonobo_property_bag_get_type (BonoboPropertyBag *pb, const char *name)
{
	BonoboProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (pb->priv != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	g_return_val_if_fail (prop != NULL, NULL);

	return prop->type;
}

/**
 * bonobo_property_bag_get_default:
 */
BonoboArg *
bonobo_property_bag_get_default (BonoboPropertyBag *pb, const char *name)
{
	BonoboProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (pb->priv != NULL, NULL);
	g_return_val_if_fail (pb->priv->set_prop != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	g_return_val_if_fail (prop != NULL, NULL);

	if (prop->default_value)
		return bonobo_arg_copy (prop->default_value);
	else {
		BonoboArg *a = bonobo_arg_new (prop->type);
		bonobo_arg_init_default (a);
		return a;
	}
}

/**
 * bonobo_property_bag_has_property:
 */
gboolean
bonobo_property_bag_has_property (BonoboPropertyBag *pb, const char *name)
{
	g_return_val_if_fail (pb != NULL, FALSE);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), FALSE);
	g_return_val_if_fail (name != NULL, FALSE);

	if (g_hash_table_lookup (pb->priv->props, name) == NULL)
		return FALSE;

	return TRUE;
}

/**
 * bonobo_property_bag_get_docstring:
 */
const char *
bonobo_property_bag_get_docstring (BonoboPropertyBag *pb, const char *name)
{
	BonoboProperty *prop;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (name != NULL, NULL);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, NULL);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->docstring;
}

/**
 * bonobo_property_bag_get_flags:
 */
const BonoboPropertyFlags
bonobo_property_bag_get_flags (BonoboPropertyBag *pb, const char *name)
{
	BonoboProperty *prop;

	g_return_val_if_fail (pb != NULL, 0);
	g_return_val_if_fail (BONOBO_IS_PROPERTY_BAG (pb), 0);
	g_return_val_if_fail (name != NULL, 0);
	g_return_val_if_fail (g_hash_table_lookup (pb->priv->props, name) != NULL, 0);

	prop = g_hash_table_lookup (pb->priv->props, name);
	return prop->flags;
}


/*
 * Class/object initialization functions.
 */

static void
bonobo_property_bag_init_corba_class (void)
{
	bonobo_property_bag_vepv.Bonobo_Unknown_epv     = bonobo_object_get_epv ();
	bonobo_property_bag_vepv.Bonobo_PropertyBag_epv = bonobo_property_bag_get_epv ();
}

static void
bonobo_property_bag_class_init (BonoboPropertyBagClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	parent_class = gtk_type_class (BONOBO_OBJECT_TYPE);

	object_class->destroy = bonobo_property_bag_destroy;

	bonobo_property_bag_init_corba_class ();
}

static void
bonobo_property_bag_init (BonoboPropertyBag *pb)
{
	pb->priv = g_new0 (BonoboPropertyBagPrivate, 1);

	pb->priv->props = g_hash_table_new (g_str_hash, g_str_equal);
}

/**
 * bonobo_property_bag_get_type:
 *
 * Returns: The GtkType corresponding to the BonoboPropertyBag class.
 */
GtkType
bonobo_property_bag_get_gtk_type (void)
{
	static GtkType type = 0;

	if (! type) {
		GtkTypeInfo info = {
			"BonoboPropertyBag",
			sizeof (BonoboPropertyBag),
			sizeof (BonoboPropertyBagClass),
			(GtkClassInitFunc) bonobo_property_bag_class_init,
			(GtkObjectInitFunc) bonobo_property_bag_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_property_bag_add_bag_listener:
 * @pb: the property bag from which to remove
 * @name: the name of the listener to remove or "" for a global
 * @listener: the listener to remove
 * @ev: optional corba environment or NULL
 * 
 * Adds a listener for any changes to properties in a bag.
 */
void
bonobo_property_bag_add_listener (BonoboPropertyBag      *pb,
				  const gchar            *name, 
				  Bonobo_PropertyListener listener,
				  CORBA_Environment      *ev)
{
	CORBA_Environment *real_ev, tmp_ev;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (listener != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	bonobo_property_bag_remove_listener (pb, name, listener, real_ev);
	if (BONOBO_EX (real_ev))
		return;

	if (!strcmp (name, ""))
		pb->priv->listeners =
			g_slist_prepend (
				pb->priv->listeners, 
				bonobo_object_dup_ref (listener, real_ev));

	else if (bonobo_property_bag_has_property (pb, name)) {
		BonoboProperty *prop = g_hash_table_lookup (pb->priv->props, name);

		if (prop) {
			if (prop->flags & BONOBO_PROPERTY_NO_LISTENING)
				CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
						     ex_Bonobo_PropertyBag_NoListening, NULL);
			else
				prop->listeners =
					g_slist_prepend (
						prop->listeners,
						bonobo_object_dup_ref (listener, real_ev));
		} else
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_PropertyBag_NotFound, NULL);
	} else
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_PropertyBag_NotFound, NULL);

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Exception setting listener '%s'",
			   bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

static gint
listener_cmp (Bonobo_PropertyListener a, Bonobo_PropertyListener b) 
{
	return !g_CORBA_Object_equal (a, b);
}

/**
 * bonobo_property_bag_remove_listener:
 * @pb: the property bag from which to remove
 * @name: the name of the listener to remove or "" for a global
 * @listener: the listener to remove
 * @ev: optional corba environment or NULL
 * 
 * Removes a listener
 **/
void
bonobo_property_bag_remove_listener (BonoboPropertyBag      *pb,
				     const gchar            *name, 
				     Bonobo_PropertyListener listener,
				     CORBA_Environment      *ev)
{
	GSList *node = NULL;
	CORBA_Environment *real_ev, tmp_ev;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (name != NULL);
	g_return_if_fail (listener != CORBA_OBJECT_NIL);

	if (ev)
		real_ev = ev;
	else {
		CORBA_exception_init (&tmp_ev);
		real_ev = &tmp_ev;
	}

	if (!strcmp (name, "")) {
		node = g_slist_find_custom (pb->priv->listeners, listener,
					   (GCompareFunc) listener_cmp);
		if (node)
			pb->priv->listeners = g_slist_remove (
				pb->priv->listeners, node->data);
		
	} else if (bonobo_property_bag_has_property (pb, name)) {

		BonoboProperty *prop = g_hash_table_lookup (pb->priv->props, name);

		if (prop) {
			node = g_slist_find_custom (prop->listeners, listener,
						    (GCompareFunc) listener_cmp);
			if (node)
				prop->listeners = g_slist_remove (
					prop->listeners, node->data);
		} else
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_PropertyBag_NotFound, NULL);
	}

	if (node) {
		Bonobo_PropertyListener listener;

		listener = (Bonobo_PropertyListener) node->data; 
		bonobo_object_release_unref (listener, real_ev);
	}

	if (BONOBO_EX (real_ev) && !ev)
		g_warning ("Exception setting listener '%s'",
			   bonobo_exception_get_text (real_ev));

	if (!ev)
		CORBA_exception_free (&tmp_ev);
}

