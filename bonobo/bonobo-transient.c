/*
 * Bonobo transient object implementation.
 *
 * This simplifies the creation of POA managers for transient objects.
 * Objects living in this POA are created on demand and destroyed after use.
 *
 * Authors:
 *   Nat Friedman    (nat@helixcode.com)
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * I just refactored the code from the original PropertyBag, all the smart hacks
 * are from Nat -mig.
 *
 * (C) 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-transient.h>

static GtkObjectClass *parent_class = NULL;

/*
 * BonoboTransient POA and Servant Manager.
 */
typedef struct {
	POA_PortableServer_ServantLocator servant_locator;
	BonoboTransient *bonobo_transient;
} BonoboTransientServantManager;

struct _BonoboTransientPriv {
	BonoboTransientServantNew     new_servant;
	BonoboTransientServantDestroy destroy_servant;
	gpointer                      callback_data;
	PortableServer_POA            poa;	
};

/*
 * This ServantManager method is invoked before a method
 * on a BonoboProperty is called.  It creates the servant
 * for the Property and returns it.
 */
static PortableServer_Servant
bonobo_transient_servant_locator_preinvoke (PortableServer_Servant servant_manager,
					    PortableServer_ObjectId *oid,
					    PortableServer_POA adapter,
					    CORBA_Identifier op_name,
					    PortableServer_ServantLocator_Cookie *cookie,
					    CORBA_Environment *ev)
{
	BonoboTransientServantManager *sm;
	PortableServer_Servant servant = NULL;
	BonoboTransient *bt, **cookie_val;
	char *object_name;

	/*
	 * Get the TransientManager out of the servant manager.
	 */
	sm = (BonoboTransientServantManager *) servant_manager;
	bt = sm->bonobo_transient;

	/*
	 * Grab the Property name and the Property Bag.
	 */
	object_name = PortableServer_ObjectId_to_string (oid, ev);
	if (ev->_major != CORBA_NO_EXCEPTION) {
		CORBA_free (object_name);
		g_warning ("BonoboPropertyBag: Could not get property name from Object ID");
		return NULL;
	}

	/*
	 * Create a temporary servant 
	 */
	servant = bt->priv->new_servant (adapter, bt, object_name, bt->priv->callback_data);
	CORBA_free (object_name);
	if (servant == NULL) {
		g_warning ("BonoboPropertyBag: Could not create transient Property servant");
		CORBA_exception_set_system(ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return NULL;
	}

	/*
	 * The cookie is arbitrary data which will get passed to
	 * postinvoke for this Property method invocation.  We have no
	 * use for it.
	 */
	cookie_val = g_new (BonoboTransient *, 1);
	*cookie_val = bt;
	*cookie = cookie_val;

	return servant;
}

/*
 * This method is invoked after a BonoboProperty method invocation.
 * It destroys the transient Property servant.
 */
static void
bonobo_transient_servant_locator_postinvoke (PortableServer_Servant servant_manager,
					     PortableServer_ObjectId *oid,
					     PortableServer_POA adapter,
					     CORBA_Identifier op_name,
					     PortableServer_ServantLocator_Cookie cookie,
					     PortableServer_Servant servant,
					     CORBA_Environment *ev)
{
	BonoboTransient *bt = BONOBO_TRANSIENT (cookie);
	
	bt->priv->destroy_servant (servant, bt->priv->callback_data);

	g_free (cookie);
}

static PortableServer_ServantBase__epv *
bonobo_transient_get_servant_base_epv (void)
{
	PortableServer_ServantBase__epv *epv;

	epv = g_new0 (PortableServer_ServantBase__epv, 1);

	epv->default_POA = PortableServer_ServantBase__default_POA;
	epv->finalize    = PortableServer_ServantBase__fini;

	return epv;
}


static POA_PortableServer_ServantManager__epv *
bonobo_transient_get_servant_manager_epv (void)
{
	POA_PortableServer_ServantManager__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantManager__epv, 1);

	return epv;
}

static POA_PortableServer_ServantLocator__epv *
bonobo_transient_get_servant_locator_epv (void)
{
	POA_PortableServer_ServantLocator__epv *epv;

	epv = g_new0 (POA_PortableServer_ServantLocator__epv, 1);

	epv->preinvoke  = bonobo_transient_servant_locator_preinvoke;
	epv->postinvoke = bonobo_transient_servant_locator_postinvoke;

	return epv;
}

static POA_PortableServer_ServantLocator__vepv *
bonobo_transient_get_servant_locator_vepv (void)
{
	static POA_PortableServer_ServantLocator__vepv *vepv = NULL;

	if (vepv != NULL)
		return vepv;

	vepv = g_new0 (POA_PortableServer_ServantLocator__vepv, 1);

	vepv->_base_epv				= bonobo_transient_get_servant_base_epv ();
	vepv->PortableServer_ServantManager_epv = bonobo_transient_get_servant_manager_epv ();
	vepv->PortableServer_ServantLocator_epv = bonobo_transient_get_servant_locator_epv ();

	return vepv;
}

static BonoboTransient *
bonobo_transient_construct (BonoboTransient *bt,
			    BonoboTransientServantNew new_servant,
			    BonoboTransientServantDestroy destroy_servant,
			    gpointer data)
{
	CORBA_PolicyList		*policies;
	BonoboTransientServantManager   *sm;
	CORBA_Environment		ev;
	char				*poa_name;

	bt->priv->new_servant = new_servant;
	bt->priv->destroy_servant = destroy_servant;
	bt->priv->callback_data = data;
	
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
		g_warning ("Could not create request processing policy for BonoboTransient POA");
		g_free (policies->_buffer);
		g_free (policies);
		CORBA_exception_free (&ev);
		return NULL;
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
		g_warning ("Could not create servant retention policy for BonoboTransient POA");
		g_free (policies->_buffer);
		g_free (policies);
		CORBA_exception_free (&ev);
		return NULL;
	}
	
	/*
	 * Create the BonoboProperty POA as a child of the root
	 * Bonobo POA.
	 */
	poa_name = g_strdup_printf ("BonoboTransient %p", bt);
	bt->poa = PortableServer_POA_create_POA (
		bonobo_poa (), poa_name, bonobo_poa_manager (),
		policies, &ev);
	
	g_free (poa_name);
	
	
	g_free (policies->_buffer);
	g_free (policies);
	
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboTransient: Could not create BonoboTransient POA");
		CORBA_exception_free (&ev);
		return NULL;
	}
	
	/*
	 * Create our ServantManager.
	 */
	sm = g_new0 (BonoboTransientServantManager, 1);
	sm->bonobo_transient = bt;

	((POA_PortableServer_ServantLocator *) sm)->vepv = bonobo_transient_get_servant_locator_vepv ();
		
	POA_PortableServer_ServantLocator__init (((PortableServer_ServantLocator *) sm), &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboTransient: Could not initialize ServantLocator");
		CORBA_exception_free (&ev);
		g_free (sm);
		return NULL;
		
	}

	PortableServer_POA_set_servant_manager (bt->priv->poa, (PortableServer_ServantManager) sm, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("BonoboTransient: Could not set POA servant manager");
		CORBA_exception_free (&ev);
		g_free (sm);
		return NULL;
	}

	return bt;
}

static void
bonobo_transient_destroy (GtkObject *object)
{
	BonoboTransient *bt = BONOBO_TRANSIENT (object);
	CORBA_Environment ev;
	
	/* Destroy the POA. */
	CORBA_exception_init (&ev);
	PortableServer_POA_destroy (bt->poa, TRUE, TRUE, &ev);
	if (ev._major != CORBA_NO_EXCEPTION)
		g_warning ("bonobo_transient_destroy: Could not destroy POA.");
	CORBA_exception_free (&ev);

	g_free (bt->priv);
	
	parent_class->destroy (object);
}

static void
bonobo_transient_class_init (BonoboTransientClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->destroy = bonobo_transient_destroy;
}

static void
bonobo_transient_init (BonoboTransient *bt)
{
	bt->priv = g_new0 (BonoboTransientPriv, 1);
}

/**
 * bonobo_transient_get_type:
 *
 * Returns: The GtkType corresponding to the BonoboTransient class.
 */
GtkType
bonobo_transient_get_type (void)
{
	static GtkType type = 0;

	if (! type) {
		GtkTypeInfo info = {
			"BonoboTransient",
			sizeof (BonoboTransient),
			sizeof (BonoboTransientClass),
			(GtkClassInitFunc) bonobo_transient_class_init,
			(GtkObjectInitFunc) bonobo_transient_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

BonoboTransient *
bonobo_transient_new (BonoboTransientServantNew new_servant,
		      BonoboTransientServantDestroy destroy_servant,
		      gpointer data)
{
	BonoboTransient *bt;

	bt = gtk_type_new (BONOBO_TRANSIENT_TYPE);
	if (bonobo_transient_construct (bt, new_servant, destroy_servant, data) == NULL)
		gtk_object_destroy (GTK_OBJECT (bt));

	return bt;
}
