/**
 * GNOME Moniker
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
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
		     const GNOME_BindOptions * bind_context,
		     const GNOME_Moniker left_moniker,
		     const CORBA_char *requested_interface,
		     CORBA_Environment *ev)
{
	GnomeMonikerClass *class;
	GnomeMoniker *moniker;
	CORBA_Object bound_object_rtn;
	
	moniker = GNOME_MONIKER (gnome_unknown_from_servant (servant));

	return (*moniker->bind_function)(moniker, bind_context, NULL);
}


static CORBA_Object
impl_bind_to_storage (PortableServer_Servant servant,
		      const GNOME_BindOptions * bind_context,
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
		   CORBA_Environment * ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}

static GNOME_Moniker_MonikerList *
impl_enum_pieces (PortableServer_Servant servant,
		  const GNOME_Moniker composite_moniker,
		  CORBA_Environment * ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}


static CORBA_char *
impl_get_display_name (PortableServer_Servant servant,
		       const GNOME_BindOptions * bind_context,
		       const GNOME_Moniker left,
		       CORBA_Environment * ev)
{
	g_error ("not implemented");
	return CORBA_OBJECT_NIL;
}

static GNOME_Moniker
impl_parse_display_name (PortableServer_Servant servant,
			 const GNOME_BindOptions * bind_context,
			 const GNOME_Moniker left,
			 const CORBA_char * display_name,
			 CORBA_short * display_name_bytes_parsed,
			 CORBA_Environment * ev)
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
	gnome_moniker_vepv.GNOME_Unknown_epv = &gnome_unknown_epv;
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

CORBA_Object
find_moniker_in_naming_service (gchar *name, gchar *kind)
{
	CosNaming_NameComponent nc[3] = {{"GNOME", "subcontext"},
					 {"Monikers", "subcontext"}};
	CosNaming_Name          nom;
	CORBA_Object name_server;
	CORBA_Object object_in_name_server;
	CORBA_Environment ev;

	g_assert (name);
	nom._maximum = 0;
	nom._length = 3;
	nom._buffer = nc;
	nom._release = CORBA_FALSE;

	CORBA_exception_init (&ev);

	nc[2].id   = (char *)name;
	nc[2].kind = (char *)kind;

	name_server = gnome_name_service_get();

	g_assert(name_server != CORBA_OBJECT_NIL);

	object_in_name_server = CosNaming_NamingContext_resolve(name_server, &nom, &ev);

	if(ev._major == CORBA_NO_EXCEPTION
	   || (ev._major == CORBA_USER_EXCEPTION
	       && strcmp(CORBA_exception_id(&ev),
			 ex_CosNaming_NamingContext_NotFound))) {

		/* found it! */

		CORBA_Object_release(name_server, &ev);

		CORBA_exception_free(&ev);
		return object_in_name_server;
	}

	/* didn't find it, return nothing */
	CORBA_Object_release(name_server, &ev);
	CORBA_exception_free(&ev);

	return CORBA_OBJECT_NIL;

} /* find_moniker_in_naming_service */




GnomeMoniker *
gnome_moniker_construct (GnomeMoniker *moniker, GNOME_Moniker corba_moniker,
			 GnomeMonikerBindFn bind_function)
{
	g_return_val_if_fail (moniker != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_MONIKER (moniker), NULL);
	g_return_val_if_fail (corba_moniker != CORBA_OBJECT_NIL, NULL);
	g_return_val_if_fail (bind_function != NULL, NULL);
	
	gnome_persist_stream_construct (GNOME_PERSIST_STREAM (moniker),
					(GNOME_Moniker) corba_moniker,
					NULL, NULL, NULL);

	moniker->bind_function = bind_function;
	
	return moniker;
}

static void
gnome_moniker_init (GnomeUnknown *object)
{
}

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


