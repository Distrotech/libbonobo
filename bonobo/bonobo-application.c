/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#include "config.h"
#include "bonobo-application.h"
#include "bonobo-app-client.h"
#include <bonobo-exception.h>
#include "bonobo-marshal.h"
#include "bonobo-arg.h"
#include "bonobo-main.h"
#include "bonobo-i18n.h"
#include "bonobo-types.h"
#include <string.h>


enum SIGNALS {
	MESSAGE,
	NEW_INSTANCE,
	LAST_SIGNAL
};

enum PROPERTIES {
	PROP_0,
	PROP_NAME,
};

static guint signals [LAST_SIGNAL] = { 0 };

typedef struct {
	BonoboAppHookFunc func;
	gpointer          data;
} BonoboAppHook;


typedef struct {
	GClosure *closure;
	GType     return_type;
} BonoboAppMessageDesc;

static GArray *bonobo_application_hooks = NULL;

/*
 * A pointer to our parent object class
 */
static gpointer parent_class;


/* ------------ Forward function declarations ------------ */
static void     bonobo_application_invoke_hooks (BonoboApplication *app);


static void bonobo_app_message_desc_free (BonoboAppMessageDesc *msgdesc)
{
	if (msgdesc->closure)
		g_closure_unref (msgdesc->closure);
	g_free (msgdesc);
}


static gboolean
_bonobo_application_message_accumulator(GSignalInvocationHint *ihint, 
					GValue                *return_accu, 
					const GValue          *handler_return, 
					gpointer               dummy)
{
	gboolean null_gvalue;

	null_gvalue = (G_VALUE_HOLDS (handler_return, G_TYPE_VALUE) &&
		       (g_value_peek_pointer (handler_return) == NULL));

	if (!null_gvalue) {
		g_value_copy (handler_return, return_accu);
		return FALSE;	/* stop emission */
	}
	return TRUE;		/* continue emission */
}



static void
bonobo_application_finalize (GObject *object)
{
	BonoboApplication *self = BONOBO_APPLICATION (object);

	if (self->message_list) {
		g_slist_foreach (self->message_list, (GFunc) CORBA_free, NULL);
		g_slist_free (self->message_list);
		self->message_list = NULL;
	}

	if (self->name) {
		g_free (self->name);
		self->name = NULL;
	}

	if (self->closure_hash) {
		g_hash_table_destroy (self->closure_hash);
		self->closure_hash = NULL;
	}

	G_OBJECT_CLASS (parent_class)->finalize (object);
}


static CORBA_any *
impl_Bonobo_Application_message (PortableServer_Servant            servant,
				 const CORBA_char                 *msg,
				 const Bonobo_Application_ArgList *args,
				 CORBA_Environment                *ev)
{
	BonoboApplication  *app = BONOBO_APPLICATION (bonobo_object (servant));
	GValue             *signal_return = NULL;
	GValueArray        *signal_args;
	int                 i;
	CORBA_any          *rv;
	GValue              value;

	signal_args = g_value_array_new (args->_length);
	memset (&value, 0, sizeof (value));
	for (i = 0; i < args->_length; ++i) {
		if (bonobo_arg_to_gvalue_alloc (&args->_buffer [i], &value)) {
			g_value_array_append (signal_args, &value);
			g_value_unset (&value);
		} else {
			g_warning ("Failed to convert type '%s' to GValue",
				   args->_buffer[i]._type->name);
		}
	}

	g_signal_emit (app, signals [MESSAGE],
		       g_quark_from_string (msg),
		       msg, signal_args, &signal_return);

	g_value_array_free (signal_args);
	rv = CORBA_any__alloc ();
	if (signal_return) {
		if (!bonobo_arg_from_gvalue_alloc (rv, signal_return)) {
			g_warning ("Failed to convert type '%s' to CORBA::any",
				   g_type_name (G_VALUE_TYPE (signal_return)));
			rv->_type = TC_void;
		}
		g_value_unset (signal_return);
		g_free (signal_return);
	} else
		rv->_type = TC_void;

	return rv;
}

gint bonobo_application_new_instance (BonoboApplication *app,
				      gint               argc,
				      gchar             *argv[])
{
	gint rv;
	g_signal_emit (app, signals [NEW_INSTANCE], 0,
		       argc, argv, &rv);
	return rv;
}

static GValue *
bonobo_application_run_closures (BonoboApplication *self,
				 const char        *name,
				 GValueArray       *args)
{
	BonoboAppMessageDesc *desc;

	desc = g_hash_table_lookup (self->closure_hash, name);

	if (desc) {
		GValue *retval = g_new0 (GValue, 1);
		GValue *params = g_newa (GValue, args->n_values + 1);

		memset (params + 0, 0, sizeof (GValue));
		g_value_init (params + 0, G_TYPE_OBJECT);
		g_value_set_object (params + 0, self);
		memcpy (params + 1, args->values, args->n_values * sizeof (GValue));
		g_value_init (retval, desc->return_type);
		g_closure_invoke (desc->closure, retval, args->n_values + 1,
				  params, NULL /* invocation_hint */);
		g_value_unset (params + 0);
		return retval;
	}

	return NULL;
}

  /* Handle the "new-instance" standard message */
static GValue *
bonobo_application_real_message (BonoboApplication *app,
				 const char        *name,
				 GValueArray       *args)
{
	return bonobo_application_run_closures (app, name, args);
}


static CORBA_long
impl_Bonobo_Application_newInstance (PortableServer_Servant           servant,
				     Bonobo_Application_argv_t const *argv,
				     CORBA_Environment               *ev)
{
	BonoboApplication *app = BONOBO_APPLICATION (bonobo_object (servant));
	CORBA_long         retval;

	retval = bonobo_application_new_instance 
		(app, argv->_length, argv->_buffer);
	return retval;
}

static __inline__ void
message_desc_copy (Bonobo_Application_MessageDesc *dest,
		   Bonobo_Application_MessageDesc *src)
{
	dest->name           = CORBA_string_dup (src->name);
	dest->return_type    = src->return_type;
	dest->types._buffer  = src->types._buffer;
	dest->types._length  = src->types._length;
	dest->types._maximum = src->types._maximum;
	dest->types._release = CORBA_FALSE;
	dest->description    = CORBA_string_dup (src->description);
}


static Bonobo_Application_MessageList *
impl_Bonobo_Application_listMessages (PortableServer_Servant  servant,
				      CORBA_Environment      *ev)
{
	BonoboApplication *app = BONOBO_APPLICATION (bonobo_object (servant));
	int                nmessages;
	GSList            *l;
	int                i;
	Bonobo_Application_MessageList *msglist;

	nmessages = g_slist_length (app->message_list);
	msglist = Bonobo_Application_MessageList__alloc ();
	msglist->_length = msglist->_maximum = nmessages;
	msglist->_buffer = Bonobo_Application_MessageList_allocbuf (nmessages);
	for (l = app->message_list, i = 0; l; l = l->next, ++i)
		message_desc_copy (&msglist->_buffer [i],
				   (Bonobo_Application_MessageDesc *) l->data);
	CORBA_sequence_set_release (msglist, CORBA_TRUE);
	return msglist;
}

static CORBA_string
impl_Bonobo_Application_getName (PortableServer_Servant  servant,
				 CORBA_Environment      *ev)
{
	BonoboApplication *app = BONOBO_APPLICATION (bonobo_object (servant));
	return CORBA_string_dup (app->name);
}

static void
set_property (GObject      *g_object,
	      guint         prop_id,
	      const GValue *value,
	      GParamSpec   *pspec)
{
	BonoboApplication *self = (BonoboApplication *) g_object;

	switch (prop_id) {
	case PROP_NAME:
		if (self->name) g_free (self->name);
		self->name = g_strdup (g_value_get_string (value));
		break;
	default:
		break;
	}
}

static void
get_property (GObject    *object,
	      guint       prop_id,
	      GValue     *value,
	      GParamSpec *pspec)
{
	BonoboApplication *self = (BonoboApplication *) object;

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string (value, self->name);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static GObject *
bonobo_application_constructor (GType                  type,
				guint                  n_construct_properties,
				GObjectConstructParam *construct_properties)
{
	GObject           *object;
	BonoboApplication *self;

	object = G_OBJECT_CLASS (parent_class)->constructor
		(type, n_construct_properties, construct_properties);
	self = BONOBO_APPLICATION (object);
	bonobo_application_invoke_hooks (self);
	return object;
}

static void
bonobo_application_class_init (BonoboApplicationClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	POA_Bonobo_Application__epv *epv = &klass->epv;

	parent_class = g_type_class_peek_parent (klass);

	object_class->finalize     = bonobo_application_finalize;
	object_class->constructor  = bonobo_application_constructor;
	object_class->set_property = set_property;
	object_class->get_property = get_property;

	signals [MESSAGE] = g_signal_new (
		"message", BONOBO_TYPE_APPLICATION,
		G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
		G_STRUCT_OFFSET (BonoboApplicationClass, message),
		_bonobo_application_message_accumulator, NULL,
		bonobo_marshal_BOXED__STRING_BOXED,
		G_TYPE_VALUE, 2, /* return_type, nparams */
		G_TYPE_STRING | G_SIGNAL_TYPE_STATIC_SCOPE,
		G_TYPE_VALUE_ARRAY);

	signals [NEW_INSTANCE] = g_signal_new (
		"new-instance", BONOBO_TYPE_APPLICATION, G_SIGNAL_RUN_LAST,
		G_STRUCT_OFFSET (BonoboApplicationClass, new_instance),
		NULL, NULL,	/* accumulator and accumulator data */
		bonobo_marshal_INT__INT_POINTER,
		G_TYPE_INT, 2, /* return_type, nparams */
		G_TYPE_INT, G_TYPE_POINTER);

	g_object_class_install_property
		(object_class, PROP_NAME,
		 g_param_spec_string
		 ("name", _("Name"), _("Application unique name"), NULL,
		  G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	klass->message = bonobo_application_real_message;

	epv->message      = impl_Bonobo_Application_message;
	epv->listMessages = impl_Bonobo_Application_listMessages;
	epv->newInstance  = impl_Bonobo_Application_newInstance;
	epv->getName      = impl_Bonobo_Application_getName;
}

static void
bonobo_application_init (BonoboApplication *self)
{
	self->closure_hash = g_hash_table_new_full
		(g_str_hash, g_str_equal,
		 g_free,(GDestroyNotify) bonobo_app_message_desc_free);
}

BONOBO_TYPE_FUNC_FULL (BonoboApplication,
		       Bonobo_Application,
		       BONOBO_TYPE_OBJECT,
		       bonobo_application);


BonoboApplication *
bonobo_application_new (const char *name)
{
	GObject           *obj;
	BonoboApplication *app;

	obj = g_object_new (BONOBO_TYPE_APPLICATION,
			    "poa", bonobo_poa_get_threaded (ORBIT_THREAD_HINT_ALL_AT_IDLE),
			    "name", name,
			    NULL);
	app = (BonoboApplication *) obj;
	return app;
}

static __inline__ CORBA_TypeCode
_gtype_to_typecode (GType gtype)
{
	static GHashTable *hash = NULL;

	if (!hash) {
		hash = g_hash_table_new (g_direct_hash, g_direct_equal);
#define mapping(gtype_, corba_type)\
		g_hash_table_insert (hash, GUINT_TO_POINTER (gtype_), corba_type);
		
		mapping (G_TYPE_NONE,    TC_void);
		mapping (G_TYPE_BOOLEAN, TC_CORBA_boolean);
		mapping (G_TYPE_INT,     TC_CORBA_long);
		mapping (G_TYPE_UINT,    TC_CORBA_unsigned_long);
		mapping (G_TYPE_LONG,    TC_CORBA_long);
		mapping (G_TYPE_ULONG,   TC_CORBA_unsigned_long);
		mapping (G_TYPE_FLOAT,   TC_CORBA_float);
		mapping (G_TYPE_DOUBLE,  TC_CORBA_double);
		mapping (G_TYPE_STRING,  TC_CORBA_string);
		
		mapping (BONOBO_TYPE_CORBA_ANY,  TC_CORBA_any);
#undef mapping
	}
	return (CORBA_TypeCode) g_hash_table_lookup (hash, GUINT_TO_POINTER (G_TYPE_INT));
}

void
bonobo_application_register_message_v (BonoboApplication *app,
					const gchar       *name,
					const gchar       *description,
					GClosure          *opt_closure,
					GType              return_type,
					GType const        arg_types[])
{
	Bonobo_Application_MessageDesc *msgdesc;
	int i, arg_types_len;

	for (arg_types_len = -1; arg_types[++arg_types_len] != G_TYPE_NONE;);

	msgdesc = Bonobo_Application_MessageDesc__alloc ();

	msgdesc->return_type = _gtype_to_typecode (return_type);
	msgdesc->name        = CORBA_string_dup (name);
	msgdesc->description = CORBA_string_dup (description);

	msgdesc->types._length = msgdesc->types._maximum = arg_types_len;
	msgdesc->types._buffer =
		CORBA_sequence_CORBA_TypeCode_allocbuf (arg_types_len);

	for (i = 0; arg_types[i] != G_TYPE_NONE; ++i)
		msgdesc->types._buffer[i] = _gtype_to_typecode (arg_types[i]);

	app->message_list = g_slist_prepend (app->message_list, msgdesc);

	if (opt_closure) {
		BonoboAppMessageDesc *desc = g_new0 (BonoboAppMessageDesc, 1);
		g_closure_ref (opt_closure);
		g_closure_sink (opt_closure);
		desc->closure = opt_closure;
		desc->return_type = return_type;
		g_hash_table_insert (app->closure_hash, g_strdup (name), desc);
	}
}


void
bonobo_application_register_message_va (BonoboApplication *app,
					const gchar       *name,
					const gchar       *description,
					GClosure          *opt_closure,
					GType              return_type,
					GType              first_arg_type,
					va_list            var_args)
{
	GArray *arg_types;
	GType   gtype;

	arg_types = g_array_new (FALSE, FALSE, sizeof(GType));
	if (first_arg_type != G_TYPE_NONE) {
		g_array_append_val (arg_types, first_arg_type);
		while ((gtype = va_arg (var_args, GType)) != G_TYPE_NONE)
			g_array_append_val (arg_types, gtype);
	}
	gtype = G_TYPE_NONE; g_array_append_val (arg_types, gtype);

	bonobo_application_register_message_v (app, name, description,
					       opt_closure, return_type,
					       (const GType *) arg_types->data);

	g_array_free (arg_types, TRUE);
}

void
bonobo_application_register_message (BonoboApplication *app,
				     const gchar       *name,
				     const gchar       *description,
				     GClosure          *opt_closure,
				     GType              return_type,
				     GType              first_arg_type,
				     ...)
{
	va_list var_args;

	va_start (var_args, first_arg_type);
	bonobo_application_register_message_va (app, name, description,
						opt_closure, return_type,
						first_arg_type, var_args);
	va_end (var_args);
}


/**
 * bonobo_application_register_unique:
 * @app: a #BonoboApplication instance
 * 
 * Try to register the running application, or check for an existing
 * application already registered and get a reference to it.
 * Applications already running but on different environments (that
 * usually means different displays, or whatever is set with
 * bonobo_activation_registration_env_set) than this one are ignored
 * and do not interfere.
 * 
 * Return value: %NULL if this process was the first to register, or a
 * BonoboAppClient which acts as client to another process that has
 * registered earlier with the same name.  If the return value is
 * non-%NULL, the programmer is responsible for freeing the
 * #BonoboApplication with bonobo_object_unref().
 **/
BonoboAppClient *
bonobo_application_register_unique (BonoboApplication *app)
{
	Bonobo_RegistrationResult  reg_res;
	gchar                     *iid, *description;
	CORBA_Object               remote_obj = CORBA_OBJECT_NIL;

	iid     = g_strdup_printf ("OAFIID:%s", app->name);
	description = g_strdup_printf (
		"<oaf_info>\n"
		"  <oaf_server iid=\"%s\" location=\"unknown\" type=\"runtime\">\n"
		"    <oaf_attribute name=\"repo_ids\" type=\"stringv\">\n"
		"       <item value=\"IDL:Bonobo/Unknown:1.0\"/>\n"
		"       <item value=\"IDL:Bonobo/Application:1.0\"/>\n"
		"    </oaf_attribute>\n"
		"    <oaf_attribute name=\"name\" type=\"string\" value=\"%s\"/>\n"
		"    <oaf_attribute name=\"description\" type=\"string\" "
		" value=\"%s application instance\"/>\n"
		"  </oaf_server>\n"
		"</oaf_info>",
		iid, app->name, app->name);
	reg_res = bonobo_activation_register_active_server_ext
		(iid, bonobo_object_corba_objref (BONOBO_OBJECT (app)), NULL,
		 Bonobo_REGISTRATION_FLAG_NO_SERVERINFO, &remote_obj,
		 description);
	g_free (iid);
	g_free (description);
	if (reg_res == Bonobo_ACTIVATION_REG_SUCCESS)
		return NULL;
	else if (reg_res == Bonobo_ACTIVATION_REG_ALREADY_ACTIVE) {
		return bonobo_app_client_new ((Bonobo_Application) remote_obj);
	} else {
		g_warning ("bonobo_register_unique_application failed"
			   " with error code %i", reg_res);
		return NULL;
	}
}


/**
 * bonobo_application_add_hook:
 * @func: hook function
 * @data: user data
 * 
 * Add a hook function to be called whenever #BonoboApplication is
 * instantiated.
 **/
void bonobo_application_add_hook (BonoboAppHookFunc func, gpointer data)
{
	BonoboAppHook hook;

	if (bonobo_application_hooks == NULL)
		bonobo_application_hooks = g_array_new (FALSE, FALSE, sizeof (BonoboAppHook));
	
	hook.func = func;
	hook.data = data;
	g_array_append_val (bonobo_application_hooks, hook);
}


/**
 * bonobo_application_remove_hook:
 * @func: hook function
 * @data: user data
 * 
 * Removes a hook function previously set with bonobo_application_add_hook().
 **/
void bonobo_application_remove_hook (BonoboAppHookFunc func, gpointer data)
{
	BonoboAppHook *hook;
	int            i;

	g_return_if_fail (bonobo_application_hooks);

	for (i = 0; i < bonobo_application_hooks->len; ++i) {
		hook = &g_array_index (bonobo_application_hooks, BonoboAppHook, i);
		if (hook->func == func && hook->data == data) {
			g_array_remove_index (bonobo_application_hooks, i);
			return;
		}
	}

	g_warning ("bonobo_application_remove_hook: "
		   "(func, data) == (%p, %p) not found.", func, data);
}


static void
bonobo_application_invoke_hooks (BonoboApplication *app)
{
	BonoboAppHook *hook;
	int            i;

	if (!bonobo_application_hooks)
		return;

	for (i = 0; i < bonobo_application_hooks->len; ++i) {
		hook = &g_array_index (bonobo_application_hooks, BonoboAppHook, i);
		hook->func (app, hook->data);
	}
}
