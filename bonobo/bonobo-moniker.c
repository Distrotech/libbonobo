/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * GNOME Moniker
 *
 * Authors:
 *   Miguel de Icaza (miguel@kernel.org)
 *   Nat Friedman (nat@gnome-support.com)
 */
#include <config.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <bonobo/gnome-moniker.h>

/* Parent GTK object class */
static GnomePersistStreamClass *gnome_moniker_parent_class;

/* The CORBA entry point vectors */
static POA_GNOME_Moniker__epv gnome_moniker_epv;

/* The CORBA vepv for the class */
POA_GNOME_Moniker__vepv gnome_moniker_vepv;

static CORBA_Object
impl_bind_to_object (PortableServer_Servant servant,
		     const GNOME_BindOptions *bind_context,
		     const GNOME_Moniker left_moniker,
		     const CORBA_char *requested_interface,
		     CORBA_Environment *ev)
{
	GnomeMonikerClass *class;
	GnomeMoniker *moniker;
	CORBA_Object bound_object_rtn;
	
	moniker = GNOME_MONIKER (gnome_object_from_servant (servant));

	return (*moniker->bind_function)(moniker, bind_context, moniker->bind_function_closure);
}


static CORBA_Object
impl_bind_to_storage (PortableServer_Servant servant,
		      const GNOME_BindOptions *bind_context,
		      const GNOME_Moniker left_moniker,
		      const CORBA_char *persistent_interface_name,
		      CORBA_Environment *ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}


static GNOME_Moniker
impl_compose_with (PortableServer_Servant servant,
		   const GNOME_Moniker right,
		   const CORBA_boolean only_if_exists,
		   CORBA_Environment *ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}

static GNOME_Moniker_MonikerList *
impl_enum_pieces (PortableServer_Servant servant,
		  const GNOME_Moniker composite_moniker,
		  CORBA_Environment *ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}


static CORBA_char *
impl_get_display_name (PortableServer_Servant servant,
		       const GNOME_BindOptions *bind_context,
		       const GNOME_Moniker left,
		       CORBA_Environment *ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}

static GNOME_Moniker
impl_parse_display_name (PortableServer_Servant servant,
			 const GNOME_BindOptions *bind_context,
			 const GNOME_Moniker left,
			 const CORBA_char *display_name,
			 CORBA_short * display_name_bytes_parsed,
			 CORBA_Environment *ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}

static void
init_moniker_corba_class (void)
{
	/* Setup the GNOME::Moniker methods */
	gnome_moniker_epv.bind_to_object = impl_bind_to_object;
	gnome_moniker_epv.bind_to_storage = impl_bind_to_storage;
	gnome_moniker_epv.compose_with = impl_compose_with;
	gnome_moniker_epv.enum_pieces = impl_enum_pieces;
	gnome_moniker_epv.get_display_name = impl_get_display_name;
	gnome_moniker_epv.parse_display_name = impl_parse_display_name;
		
	/* Now the Vepv */
	gnome_moniker_vepv.GNOME_Unknown_epv = &gnome_object_epv;
	gnome_moniker_vepv.GNOME_Persist_epv = &gnome_persist_epv;
	gnome_moniker_vepv.GNOME_PersistStream_epv = &gnome_persist_stream_epv;
	gnome_moniker_vepv.GNOME_Moniker_epv = &gnome_moniker_epv;
}

static void
gnome_moniker_class_init (GnomeMonikerClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	GnomePersistStreamClass *ps_class = (GnomePersistStreamClass *) class;
	
	gnome_moniker_parent_class = gtk_type_class (gnome_persist_stream_get_type ());

	init_moniker_corba_class ();
}

static CORBA_Object
create_gnome_moniker (GnomeObject *object)
{
	POA_GNOME_Moniker *servant;
	
	servant = (POA_GNOME_Moniker *)g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_moniker_vepv;

	POA_GNOME_Moniker__init ((PortableServer_Servant) servant, &object->ev);
	if (object->ev._major != CORBA_NO_EXCEPTION) {
		g_free (servant);
		return CORBA_OBJECT_NIL;
	}

	return gnome_object_activate_servant (object, servant);
}

/**
 * gnome_moniker_construct:
 * @moniker: The GnomeMoniker object to be initialized.
 * @corba_moniker: The CORBA object supporting GNOME::Moniker.
 * @bind_function: The function which is called when the
 * bind_to_object() method is invoked on the #GnomeMoniker object.
 * @bind_function_closure: The closure pointer passed to @bind_function.
 *
 * Initializes the @moniker GnomeMoniker object with the specified
 * @bind_function and CORBA object @corba_moniker.  The @corba_moniker
 * object must support the GNOME_Moniker interface.
 *
 * Returns: The initialized GnomeMoniker object.
 */
GnomeMoniker *
gnome_moniker_construct (GnomeMoniker *moniker, GNOME_Moniker corba_moniker,
			 const char *moniker_goad_id,
			 GnomeMonikerBindFn bind_function,
			 void *bind_function_closure)
{
	g_return_val_if_fail (moniker != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_MONIKER (moniker), NULL);
	g_return_val_if_fail (corba_moniker != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (bind_function != NULL, NULL);
	
	gnome_persist_stream_construct (GNOME_PERSIST_STREAM (moniker),
					(GNOME_Moniker) corba_moniker,
					moniker_goad_id,
					NULL, NULL, NULL);

	moniker->bind_function = bind_function;
	moniker->bind_function_closure = bind_function_closure;
	
	return moniker;
}

/**
 * gnome_moniker_new:
 * @moniker_goad_id: The GOAD ID registered by this moniker.
 * @bind_function: The routine which is invoked for the bind_to_object() interface method.
 * @bind_function_closure: The closure data passed to @bind_function when it is called.
 *
 * Creates a new GnomeMoniker object and the corresponding CORBA interface using the specified
 * bind_to_object() callback, @bind_function.
 *
 * Returns: The newly-constructed GnomeMoniker object.
 */
GnomeMoniker *
gnome_moniker_new (const char *moniker_goad_id,
		   GnomeMonikerBindFn bind_function,
		   void *bind_function_closure)
{
	GNOME_Moniker corba_moniker;
	GnomeMoniker *moniker;

	g_return_val_if_fail (bind_function != NULL, NULL);

	moniker = gtk_type_new (gnome_moniker_get_type ());
	corba_moniker = create_gnome_moniker (GNOME_OBJECT (moniker));
	if (corba_moniker == CORBA_OBJECT_NIL) {
		gtk_object_destroy (GTK_OBJECT (moniker));
		return NULL;
	}

	return gnome_moniker_construct (
		moniker, moniker_goad_id,
		corba_moniker, bind_function, bind_function_closure);
}

static void
gnome_moniker_init (GnomeObject *object)
{
}

/**
 * gnome_moniker_get_type:
 *
 * Returns: The GtkType of the GnomeMoniker class.
 */
GtkType
gnome_moniker_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/Moniker:1.0",
			sizeof (GnomeMoniker),
			sizeof (GnomeMonikerClass),
			(GtkClassInitFunc) gnome_moniker_class_init,
			(GtkObjectInitFunc) gnome_moniker_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_persist_stream_get_type (), &info);
	}

	return type;
}


