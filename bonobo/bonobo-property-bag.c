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

#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>

enum {
	VALUE_CHANGED,
	LAST_SIGNAL
};

static guint gnome_property_bag_signals [LAST_SIGNAL];

POA_GNOME_PropertyBag__vepv gnome_property_bag_vepv;

struct _GnomePropertyBagPrivate {
	PortableServer_POA	 poa;
	GHashTable		*props;
	GHashTable		*types;
};

typedef struct {
	GnomePropertyBagValueMarshalerFn   marshaler;
	GnomePropertyBagValueDemarshalerFn demarshaler;
	GnomePropertyBagValueReleaserFn    releaser;
} GnomePropertyTypeDefinition;



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
	static PortableServer_POA	   property_poa = NULL;
	CORBA_PolicyList		  *policies;     /* The list of policies used to create the GnomeProperty POA. */
	GnomePropertyBagServantManager   *sm;		 /* Our special servant manager. */
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

	if (property_poa)
		pb->priv->poa = property_poa;
	else {
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
	}

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

struct pb_objref_closure {
	GnomePropertyBag *pb;
	GNOME_PropertyList *prop_list;
	int curr_index;
	CORBA_Environment *ev;
};

static void
gnome_property_bag_foreach_create_objrefs (gpointer key, gpointer value,
					   gpointer data)
{
	struct pb_objref_closure *closure = data;
	char			 *name = key;
	GNOME_Property		  prop;

	
	prop = gnome_property_bag_create_objref (closure->pb, name,
				  & (closure->prop_list->_buffer [closure->curr_index]),
						  closure->ev);

	closure->curr_index ++;
}

static GNOME_PropertyList *
impl_GNOME_PropertyBag_get_properties (PortableServer_Servant servant,
				       CORBA_Environment *ev)
{
	GnomePropertyBag *pb = GNOME_PROPERTY_BAG (gnome_object_from_servant (servant));
	GNOME_PropertyList *prop_list;
	struct pb_objref_closure *closure;
	int len;

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
	closure = g_new0 (struct pb_objref_closure, 1);
	closure->pb = pb;
	closure->prop_list = prop_list;
	closure->ev = ev;

	g_hash_table_foreach (pb->priv->props,
			      gnome_property_bag_foreach_create_objrefs,
			      closure);

	g_free (closure);

	return prop_list;
}

static GNOME_Property
impl_GNOME_PropertyBag_get_property (PortableServer_Servant servant,
				     CORBA_char *name,
				     CORBA_Environment *ev)
{
	GnomePropertyBag	*pb = GNOME_PROPERTY_BAG (gnome_object_from_servant (servant));
	GNOME_Property		 prop;

	if (g_hash_table_lookup (pb->priv->props, name) == NULL) {
		GNOME_PropertyBag_PropertyNotFound *exception;

		exception = g_new0 (GNOME_PropertyBag_PropertyNotFound, 1);
		if (exception == NULL) {
			CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY,
						    CORBA_COMPLETED_NO);
			return CORBA_OBJECT_NIL;
		}

		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_GNOME_PropertyBag_PropertyNotFound,
				     exception);

		return CORBA_OBJECT_NIL;
	}

	gnome_property_bag_create_objref (pb, name, &prop, ev);

	return prop;
}

struct pb_names_closure {
	GNOME_PropertyNames *name_list;
	int curr_index;
};

static void
gnome_property_bag_foreach_create_names (gpointer key, gpointer value,
					 gpointer data)
{
	struct pb_names_closure  *closure = data;
	char			 *name = key;

	closure->name_list->_buffer [closure->curr_index] = CORBA_string_dup (name);
	closure->curr_index ++;
}

static GNOME_PropertyNames *
impl_GNOME_PropertyBag_get_property_names (PortableServer_Servant servant,
					   CORBA_Environment *ev)
{
	GnomePropertyBag *pb = GNOME_PROPERTY_BAG (gnome_object_from_servant (servant));
	GNOME_PropertyNames *name_list;
	struct pb_names_closure *closure;
	int len;

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
	closure = g_new0 (struct pb_names_closure, 1);
	closure->name_list = name_list;

	g_hash_table_foreach (pb->priv->props,
			      gnome_property_bag_foreach_create_names,
			      closure);

	g_free (closure);

	return name_list;
}


/*
 * GnomePropertyBag construction/deconstruction functions. 
 */

static GnomePropertyBag *
gnome_property_bag_construct (GnomePropertyBag *pb,
			      CORBA_Object corba_pb)
{
	gnome_object_construct (GNOME_OBJECT (pb), corba_pb);

	if (! gnome_property_bag_create_poa (pb)) {
		g_free (pb);
		return NULL;
	}

	return pb;
}

static CORBA_Object
gnome_property_bag_create_corba_object (GnomeObject *object)
{
	POA_GNOME_PropertyBag *servant;
	CORBA_Environment ev;

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
	CORBA_Object corba_pb;


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
	GnomePropertyTypeDefinition *type = value;
	char *type_name = key;

	g_free (type);
	g_free (type_name);

	return TRUE;
}

static gboolean
gnome_property_bag_foreach_remove_prop (gpointer key, gpointer value,
					gpointer user_data)
{
	GnomePropertyBag *pb = user_data;
	GnomePropertyTypeDefinition *type;
	GnomeProperty *prop = value;
	char *prop_name = key;

	type = g_hash_table_lookup (pb->priv->types, prop->type);
	g_return_val_if_fail (type != NULL, TRUE);

	(type->releaser) (pb, prop->type, prop->value);
	(type->releaser) (pb, prop->type, prop->default_value);

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
				   char *type)
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
gnome_property_bag_add (GnomePropertyBag *pb, char *name,
			char *type, gpointer value,
			gpointer default_value, char *docstring,
			GnomePropertyFlags flags)
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
gnome_property_bag_set_value (GnomePropertyBag *pb, char *name,
			      gpointer value)
{
	GnomePropertyTypeDefinition *ptype;
	GnomeProperty *prop;
	gpointer old_value;

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

	(ptype->releaser) (pb, prop->type, old_value);
}

/**
 * gnome_property_bag_set_default:
 */
void
gnome_property_bag_set_default (GnomePropertyBag *pb, char *name,
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
gnome_property_bag_set_docstring (GnomePropertyBag *pb, char *name,
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
gnome_property_bag_set_flags (GnomePropertyBag *pb, char *name,
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
 * Properties and the marshaling/demarshaling engine.
 */
void
gnome_property_bag_create_type (GnomePropertyBag *pb, char *type_name,
				GnomePropertyBagValueMarshalerFn   marshaler,
				GnomePropertyBagValueDemarshalerFn demarshaler,
				GnomePropertyBagValueReleaserFn    releaser)
{
	GnomePropertyTypeDefinition *ptype;

	g_return_if_fail (pb != NULL);
	g_return_if_fail (GNOME_IS_PROPERTY_BAG (pb));
	g_return_if_fail (type_name != NULL);
	g_return_if_fail (marshaler != NULL);
	g_return_if_fail (demarshaler != NULL);

	/* Make sure this type doesn't already exist. */
	g_return_if_fail (g_hash_table_lookup (pb->priv->types, type_name) == NULL);

	ptype = g_new0 (GnomePropertyTypeDefinition, 1);
	ptype->marshaler   = marshaler;
	ptype->demarshaler = demarshaler;
	ptype->releaser    = releaser;

	g_hash_table_insert (pb->priv->types, type_name, ptype);
}

/**
 * gnome_property_bag_get_prop_any:
 */
CORBA_any *
gnome_property_bag_value_to_any (GnomePropertyBag *pb, const char *type,
				 const gpointer value)
{
	GnomePropertyBagValueMarshalerFn value_marshaler;
	GnomePropertyTypeDefinition *ptype;
	CORBA_any *any;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (type != NULL, NULL);

	ptype = g_hash_table_lookup (pb->priv->types, type);
	g_return_val_if_fail (ptype != NULL, NULL);

	value_marshaler = ptype->marshaler;
	g_return_val_if_fail (value_marshaler != NULL, NULL);

	any = (value_marshaler) (pb, type, value);

	return any;
}

/**
 * gnome_property_bag_any_to_value:
 */
gpointer
gnome_property_bag_any_to_value (GnomePropertyBag *pb, const char *type,
				 const CORBA_any *any)
{
	GnomePropertyBagValueDemarshalerFn value_demarshaler;
	GnomePropertyTypeDefinition *ptype;
	gpointer value;

	g_return_val_if_fail (pb != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_PROPERTY_BAG (pb), NULL);
	g_return_val_if_fail (type != NULL, NULL);

	ptype = g_hash_table_lookup (pb->priv->types, type);
	g_return_val_if_fail (ptype != NULL, NULL);

	value_demarshaler = ptype->demarshaler;
	g_return_val_if_fail (value_demarshaler != NULL, NULL);

	value = (value_demarshaler) (pb, type, any);

	return value;
}


/*
 * Standard type marshalers.
 */

/* Gross macros to get around ORBit incompleteness. */
#define CORBA_short__alloc() (CORBA_short *) CORBA_octet_allocbuf (sizeof (CORBA_short))
#define CORBA_unsigned_short__alloc() (CORBA_unsigned_short *) CORBA_octet_allocbuf (sizeof (CORBA_unsigned_short))
#define CORBA_long__alloc() (CORBA_long *) CORBA_octet_allocbuf (sizeof (CORBA_long))
#define CORBA_unsigned_long__alloc() (CORBA_unsigned_long *) CORBA_octet_allocbuf (sizeof (CORBA_unsigned_long))
#define CORBA_string__alloc() (CORBA_char **) CORBA_octet_allocbuf (sizeof (CORBA_char *))
#define CORBA_float__alloc() (CORBA_float *) CORBA_octet_allocbuf (sizeof (CORBA_float))
#define CORBA_double__alloc() (CORBA_double *) CORBA_octet_allocbuf (sizeof (CORBA_double))
#define CORBA_boolean__alloc() (CORBA_boolean *) CORBA_octet_allocbuf (sizeof (CORBA_boolean))

static CORBA_any *
gnome_property_bag_marshal_boolean (GnomePropertyBag *pb, const char *type,
				    const gpointer value)
{
	CORBA_any *any;
	CORBA_boolean *b;


	b = CORBA_boolean__alloc();
	*b = *(CORBA_boolean *) value;
	
	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_boolean;
	any->_value = b;
	CORBA_any_set_release (any, TRUE);

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_string (GnomePropertyBag *pb, const char *type,
				   const gpointer value)
{
	CORBA_any *any;
	CORBA_char **str;
	const char *string_value;

	if (value == NULL)
		string_value = "";
	else
		string_value = (const char *) value;

	str = CORBA_string__alloc();
	*str = CORBA_string_dup (string_value);
	
	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_string;
	any->_value = str;
	CORBA_any_set_release (any, TRUE);

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_short (GnomePropertyBag *pb, const char *type,
				  const gpointer value)
{
	CORBA_any *any;
	CORBA_short *s;

	s = CORBA_short__alloc();
	*s = *((CORBA_short *)value);

	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_short;
	any->_value = s;

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_ushort (GnomePropertyBag *pb, const char *type,
				   const gpointer value)
{
	CORBA_any *any;
	CORBA_unsigned_short *s;

	s = CORBA_unsigned_short__alloc();
	*s = *((CORBA_unsigned_short *)value);

	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_ushort;
	any->_value = s;
	CORBA_any_set_release (any, TRUE);

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_long (GnomePropertyBag *pb, const char *type,
				 const gpointer value)
{
	CORBA_any *any;
	CORBA_long *l;

	l = CORBA_long__alloc();
	*l = *((CORBA_long *) value);

	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_long;
	any->_value = l;
	CORBA_any_set_release (any, TRUE);

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_ulong (GnomePropertyBag *pb, const char *type,
				  const gpointer value)
{
	CORBA_any *any;
	CORBA_unsigned_long *l;

	l = CORBA_unsigned_long__alloc();
	*l = *((CORBA_unsigned_long *) value);

	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_ulong;
	any->_value = l;
	CORBA_any_set_release (any, TRUE);

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_float (GnomePropertyBag *pb, const char *type,
				  const gpointer value)
{
	CORBA_any *any;
	CORBA_float *f;

	f = CORBA_float__alloc();
	*f = *((CORBA_float *) value);

	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_float;
	any->_value = f;
	CORBA_any_set_release (any, TRUE);

	return any;
}

static CORBA_any *
gnome_property_bag_marshal_double (GnomePropertyBag *pb, const char *type,
				   const gpointer value)
{
	CORBA_any *any;
	CORBA_double *d;

	d = CORBA_double__alloc();
	*d = *((CORBA_double *) value);

	any = CORBA_any__alloc ();
	any->_type = (CORBA_TypeCode) TC_double;
	any->_value = d;
	CORBA_any_set_release (any, TRUE);

	return any;
}

/*
 * Standard type demarshalers.
 */
static gpointer
gnome_property_bag_demarshal_boolean (GnomePropertyBag *pb, const char *type,
				      const CORBA_any *any)
{
	CORBA_boolean *b;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_string, NULL);

	b = g_new0 (CORBA_boolean, 1);
	*b = *((CORBA_boolean *) any->_value);

	return (gpointer) b;
}

static gpointer
gnome_property_bag_demarshal_string (GnomePropertyBag *pb, const char *type,
				     const CORBA_any *any)
{
	char *s;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_string, NULL);

	s = g_strdup (any->_value);

	return (gpointer) s;
}

static gpointer
gnome_property_bag_demarshal_short (GnomePropertyBag *pb, const char *type,
				    const CORBA_any *any)
{
	CORBA_short *s;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_short, NULL);

	s = g_new0 (CORBA_short, 1);
	*s = *((CORBA_short *)any->_value);

	return (gpointer) s;
}

static gpointer
gnome_property_bag_demarshal_ushort (GnomePropertyBag *pb, const char *type,
				     const CORBA_any *any)
{
	CORBA_unsigned_short *s;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_ushort, NULL);

	s = g_new0 (CORBA_unsigned_short, 1);
	*s = *((CORBA_unsigned_short *)any->_value);

	return (gpointer) s;
}

static gpointer
gnome_property_bag_demarshal_long (GnomePropertyBag *pb, const char *type,
				   const CORBA_any *any)
{
	CORBA_long *l;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_long, NULL);

	l = g_new0 (CORBA_long, 1);
	*l = *((CORBA_long *)any->_value);

	return (gpointer) l;
}

static gpointer
gnome_property_bag_demarshal_ulong (GnomePropertyBag *pb, const char *type,
				    const CORBA_any *any)
{
	CORBA_unsigned_long *l;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_ulong, NULL);

	l = g_new0 (CORBA_unsigned_long, 1);
	*l = *((CORBA_unsigned_long *)any->_value);

	return (gpointer) l;
}

static gpointer
gnome_property_bag_demarshal_float (GnomePropertyBag *pb, const char *type,
				    const CORBA_any *any)
{
	CORBA_float *f;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_float, NULL);

	f = g_new0 (CORBA_float, 1);
	*f = *((CORBA_float *)any->_value);

	return (gpointer) f;
}

static gpointer
gnome_property_bag_demarshal_double (GnomePropertyBag *pb, const char *type,
				     const CORBA_any *any)
{
	CORBA_double *d;

	g_return_val_if_fail (any->_type->kind == CORBA_tk_double, NULL);

	d = g_new0 (CORBA_double, 1);
	*d = *((CORBA_double *)any->_value);

	return (gpointer) d;
}

static void
gnome_property_bag_generic_releaser (GnomePropertyBag *pb, const char *type,
				     gpointer value)
{
	g_free (value);
}


/*
 * Class/object initialization functions.
 */
static void
gnome_property_bag_init_corba_class (void)
{
	gnome_property_bag_vepv.GNOME_Unknown_epv = gnome_object_get_epv ();
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
				gtk_marshal_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 2,
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
					gnome_property_bag_marshal_boolean,
					gnome_property_bag_demarshal_boolean,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "string",
					gnome_property_bag_marshal_string,
					gnome_property_bag_demarshal_string,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "short",
					gnome_property_bag_marshal_short,
					gnome_property_bag_demarshal_short,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "ushort",
					gnome_property_bag_marshal_ushort,
					gnome_property_bag_demarshal_ushort,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "long",
					gnome_property_bag_marshal_long,
					gnome_property_bag_demarshal_long,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "ulong",
					gnome_property_bag_marshal_ulong,
					gnome_property_bag_demarshal_ulong,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "float",
					gnome_property_bag_marshal_float,
					gnome_property_bag_demarshal_float,
					gnome_property_bag_generic_releaser);
	gnome_property_bag_create_type (pb, "double",
					gnome_property_bag_marshal_double,
					gnome_property_bag_demarshal_double,
					gnome_property_bag_generic_releaser);
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

	if (!type){
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
