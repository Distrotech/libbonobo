/**
 * bonobo-moniker-util.c
 *
 * Copyright (C) 2000  Helix Code, Inc.
 *
 * Authors:
 *	Michael Meeks    (michael@helixcode.com)
 *	Ettore Perazzoli (ettore@helixcode.com)
 */

#include "bonobo.h"
#include <liboaf/liboaf.h>

struct {
	char *prefix;
	char *oafiid;
} fast_prefix [] = {
	{ "file:",   "OAFIID:Bonobo_Moniker_File"  },
	{ "query:(", "OAFIID:Bonobo_Moniker_Query" },
	{ "!",       "OAFIID:Bonobo_Moniker_Item"  },
	{ "OAFIID:", "OAFIID:Bonobo_Moniker_Oaf"   },
	{ "OAFAID:", "OAFIID:Bonobo_Moniker_Oaf"   },
	{ "new:",    "OAFIID:Bonobo_Moniker_New"   },
	{ "http:",   "OAFIID:Bonobo_Moniker_HTTP"  },
/*
	{ "queue:", "" },
*/
	{ NULL, NULL }
};

static char *
moniker_id_from_nickname (const CORBA_char *name)
{
	int i;

	for (i = 0; fast_prefix [i].prefix; i++) {
		int len = strlen (fast_prefix [i].prefix);

		if (!g_strncasecmp (fast_prefix [i].prefix, name, len)) {

			return fast_prefix [i].oafiid;
		}
	}

	return NULL;
}

Bonobo_Moniker
bonobo_moniker_util_new_from_name_full (Bonobo_Moniker     parent,
					const CORBA_char  *name,
					CORBA_Environment *ev)
{
	Bonobo_Unknown   object;
	Bonobo_Moniker   toplevel, moniker;
	const char       *iid;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (name != NULL, NULL);

	if (name [0] == '#')
		name++;

	iid = moniker_id_from_nickname (name);

	if (!iid) { /* Do an oaf-query for a handler */
		char *prefix, *query;
		int   len;

		for (len = 0; name [len]; len++) {
			if (name [len] == ':')
				break;
		}

		prefix = g_strndup (name, len);
		
		query = g_strdup_printf (
			"repo_ids.has ('IDL:Bonobo/Moniker:1.0') AND "
			"bonobo:moniker == '%s'", prefix);
		g_free (prefix);
		
		object = oaf_activate (query, NULL, 0, NULL, ev);
		g_free (query);
		
		if (ev->_major != CORBA_NO_EXCEPTION)
			return CORBA_OBJECT_NIL;

		if (object == CORBA_OBJECT_NIL) {
			CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
					     ex_Bonobo_Moniker_UnknownPrefix, NULL);
			return CORBA_OBJECT_NIL;
		}
	} else {
		object = oaf_activate_from_id ((gchar *) iid, 0, NULL, ev);

		if (ev->_major != CORBA_NO_EXCEPTION)
			return CORBA_OBJECT_NIL;
		
		if (object == CORBA_OBJECT_NIL) {
			g_warning ("Activating '%s' returned nothing", iid);
			return CORBA_OBJECT_NIL;
		}
	}

	toplevel = Bonobo_Unknown_queryInterface (
		object, "IDL:Bonobo/Moniker:1.0", ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Moniker object '%s' doesn't implement "
			   "the Moniker interface", iid);
		return CORBA_OBJECT_NIL;
	}

	moniker = Bonobo_Moniker_parseDisplayName (toplevel, parent,
						   name, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	bonobo_object_release_unref (toplevel, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	return moniker;
}

CORBA_char *
bonobo_moniker_util_get_parent_name (Bonobo_Moniker     moniker,
				     CORBA_Environment *ev)
{
	Bonobo_Moniker parent;
	CORBA_char    *name;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, NULL);

	parent = Bonobo_Moniker__get_parent (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION ||
	    parent == CORBA_OBJECT_NIL)
		return NULL;
	
	name = Bonobo_Moniker_getDisplayName (parent, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		name = NULL;

	bonobo_object_release_unref (parent, ev);

	return name;
}

Bonobo_Unknown
bonobo_moniker_util_qi_return (Bonobo_Unknown     object,
			       const CORBA_char  *requested_interface,
			       CORBA_Environment *ev)
{
	Bonobo_Unknown retval = CORBA_OBJECT_NIL;

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	if (object == CORBA_OBJECT_NIL) {
		g_warning ("Object is NIL");
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		return CORBA_OBJECT_NIL;
	}

	retval = Bonobo_Unknown_queryInterface (
		object, requested_interface, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		goto release_unref_object;
	
	if (retval == CORBA_OBJECT_NIL) {
		g_warning ("No interface '%s' on object", requested_interface);
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_Bonobo_Moniker_InterfaceNotFound, NULL);
		goto release_unref_object;
	}

 release_unref_object:	
	bonobo_object_release_unref (object, ev);

	if (retval != CORBA_OBJECT_NIL)
		return CORBA_Object_duplicate (retval, ev);
	else
		return CORBA_OBJECT_NIL;
}

int
bonobo_moniker_util_seek_std_separator (const CORBA_char *str,
					int               min_idx)
{
	int i;

	g_return_val_if_fail (str != NULL, 0);
	g_return_val_if_fail (min_idx >= 0, 0);

	for (i = 0; i < min_idx; i++) {
		if (!str [i]) {
			g_warning ("Serious separator error, seeking in '%s' < %d",
				   str, min_idx);
			return i;
		}
	}

#warning We need escaping support here 

	for (; str [i]; i++) {

		if (str [i] == '!' ||
		    str [i] == '#')
			break;
	}
	
	return i;
}

Bonobo_Moniker
bonobo_moniker_client_new_from_name (const CORBA_char  *name,
				     CORBA_Environment *ev)
{
	return bonobo_moniker_util_new_from_name_full (
		CORBA_OBJECT_NIL, name, ev);
}

CORBA_char *
bonobo_moniker_client_get_name (Bonobo_Moniker     moniker,
				CORBA_Environment *ev)
{
	CORBA_char *name;

	g_return_val_if_fail (ev != NULL, NULL);
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, NULL);

	name = Bonobo_Moniker_getDisplayName (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return NULL;

	return name;
}

static void
init_default_resolve_options (Bonobo_ResolveOptions *options)
{
	options->flags = 0;
	options->timeout = -1;
}

Bonobo_Unknown
bonobo_moniker_client_resolve_default (Bonobo_Moniker     moniker,
				       const char        *interface_name,
				       CORBA_Environment *ev)
{
	Bonobo_ResolveOptions options;
	
	g_return_val_if_fail (moniker != CORBA_OBJECT_NIL, CORBA_OBJECT_NIL);

	init_default_resolve_options (&options);

	return Bonobo_Moniker_resolve (moniker, &options, interface_name, ev);
}

BonoboObjectClient *
bonobo_moniker_client_resolve_client_default (Bonobo_Moniker     moniker,
					      const char        *interface_name,
					      CORBA_Environment *ev)
{
	Bonobo_Unknown unknown;

	g_return_val_if_fail (ev != NULL, NULL);

	unknown = bonobo_moniker_client_resolve_default (
		moniker, interface_name, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return NULL;

	if (unknown == CORBA_OBJECT_NIL)
		return NULL;

	return bonobo_object_client_from_corba (unknown);
}

Bonobo_Unknown
bonobo_get_object (const CORBA_char *name,
		   const char        *interface_name,
		   CORBA_Environment *ev)
{
	Bonobo_Moniker moniker;
	Bonobo_Unknown retval;

	moniker = bonobo_moniker_client_new_from_name (name, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;

	retval = bonobo_moniker_client_resolve_default (
		moniker, interface_name, ev);

	bonobo_object_release_unref (moniker, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		return CORBA_OBJECT_NIL;
	
	return retval;
}

typedef void (*ASyncMarshal)   (BonoboMonikerASyncHandle *handle);

typedef void (*ASyncDeMarshal) (GIOChannel               *source,
				GIOCondition              condition,
				BonoboMonikerASyncHandle *handle);

struct _BonoboMonikerASyncHandle {
	/* Exposed stuff */
	Bonobo_Unknown        object;
	CORBA_Environment     ev;

	/* Private stuff */
	CORBA_Environment    *send_ev;

	BonoboMonikerCallback cb;
	gpointer              user_data;

	GIOPConnection       *request_cnx;

	GIOChannel           *channel;
	GIOP_unsigned_long    request_id;

	/* Used for dealing with forwarded connections */
	ASyncMarshal          marshal_fn;
	ASyncDeMarshal        demarshal_fn;
};

static BonoboMonikerASyncHandle *
bonobo_moniker_async_handle_construct (BonoboMonikerASyncHandle *handle,
				       BonoboMonikerCallback     cb,
				       gpointer                  user_data,
				       ASyncMarshal              marshal_fn,
				       ASyncDeMarshal            demarshal_fn,
				       CORBA_Environment        *ev)
{
	handle->object    = CORBA_OBJECT_NIL;
	handle->cb        = cb;
	handle->user_data = user_data;
	handle->send_ev   = ev;

	handle->marshal_fn   = marshal_fn;
	handle->demarshal_fn = demarshal_fn;
	handle->request_id   = GPOINTER_TO_UINT (handle);

	CORBA_exception_init (&handle->ev);

	return handle;
}

void
bonobo_moniker_async_handle_release (BonoboMonikerASyncHandle *handle)
{
	g_assert (handle->channel == NULL);
	CORBA_exception_free (&handle->ev);
	g_free (handle);
}

static void
bonobo_moniker_async_handle_set_connection (BonoboMonikerASyncHandle *handle,
					    GIOPConnection           *request_cnx)
{
	g_return_if_fail (handle != NULL);
	g_return_if_fail (handle->demarshal_fn != NULL);

	handle->request_cnx = request_cnx;

	g_assert (request_cnx->fd > 0);
	g_assert (handle->channel == NULL);
	handle->channel = g_io_channel_unix_new (request_cnx->fd);
	g_assert (handle->channel);

	g_io_add_watch (handle->channel, G_IO_IN,
			(GIOFunc) handle->demarshal_fn, handle);
}

static void
bonobo_moniker_async_forward (BonoboMonikerASyncHandle *handle, 
			      GIOPConnection           *request_cnx)
{
	handle->send_ev = &handle->ev;
	bonobo_moniker_async_handle_set_connection (handle, request_cnx);
}

static void
bonobo_moniker_async_marshal (BonoboMonikerASyncHandle *handle)
{
	handle->marshal_fn (handle);
}

/**
 * unhook_io_channel:
 * @handle: 
 * 
 *   Vital to call this as we enter the demarshal function
 * so that we don't get unwanted re-enterancy.
 **/
static void
unhook_io_channel (BonoboMonikerASyncHandle *handle)
{
	g_assert (handle->channel != NULL);
	g_io_channel_unref (handle->channel);
	handle->channel = NULL;
}

static void
bonobo_moniker_invoke_callback (BonoboMonikerASyncHandle *handle)
{
	/* Should have been unhooked at entry to demarshal */
	g_assert (handle->channel == NULL);

	handle->cb (handle, handle->user_data);
}

Bonobo_Moniker
bonobo_moniker_async_handle_get_moniker (BonoboMonikerASyncHandle *handle)
{
	g_return_val_if_fail (handle != NULL, NULL);

	return handle->object;
}

Bonobo_Unknown
bonobo_moniker_async_handle_get_object (BonoboMonikerASyncHandle *handle)
{
	g_return_val_if_fail (handle != NULL, NULL);

	return handle->object;
}

CORBA_Environment *
bonobo_moniker_async_handle_get_environment (BonoboMonikerASyncHandle *handle)
{
	g_return_val_if_fail (handle != NULL, NULL);

	return &handle->ev;
}

void
bonobo_moniker_client_new_from_name_async (const CORBA_char        *name,
					   CORBA_Environment       *ev,
					   BonoboMonikerCallback    cb,
					   gpointer                 user_data)
{
	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (name != CORBA_OBJECT_NIL);

	g_warning ("Unimplemented as yet");
}

typedef struct {
	BonoboMonikerASyncHandle h;
	Bonobo_Moniker           moniker;
	Bonobo_ResolveOptions   *options;
	char                    *interface_name;
} ResolveHandle;

static void
resolve_marshal (BonoboMonikerASyncHandle *handle)
{
	Bonobo_Moniker _obj           = ((ResolveHandle *) handle)->moniker;
	Bonobo_ResolveOptions *options = ((ResolveHandle *) handle)->options;
	CORBA_char *requested_interface = ((ResolveHandle *) handle)->interface_name;

	register GIOP_unsigned_long _ORBIT_system_exception_minor;
	register CORBA_completion_status _ORBIT_completion_status;
	register GIOPSendBuffer *_ORBIT_send_buffer;
	register GIOPConnection *_cnx  = handle->request_cnx;

	g_warning ("Resolve marshal");

	_ORBIT_send_buffer = NULL;
	_ORBIT_completion_status = CORBA_COMPLETED_NO;

	{ /* marshalling */
		static const struct
		{
			CORBA_unsigned_long len;
			char opname[8];
		}
		_ORBIT_operation_name_data =
		{
			8, "resolve"};
		static const struct iovec _ORBIT_operation_vec =
		{ (gpointer) & _ORBIT_operation_name_data, 12 };
		register CORBA_unsigned_long _ORBIT_tmpvar_0;
		CORBA_unsigned_long _ORBIT_tmpvar_1;

		_ORBIT_send_buffer =
			giop_send_request_buffer_use(_cnx, NULL, handle->request_id,
						     CORBA_TRUE,
						     &(_obj->active_profile->object_key_vec),
						     &_ORBIT_operation_vec,
						     &ORBit_default_principal_iovec);

		_ORBIT_system_exception_minor = ex_CORBA_COMM_FAILURE;
		if (!_ORBIT_send_buffer)
			goto _ORBIT_system_exception;
		giop_message_buffer_do_alignment(GIOP_MESSAGE_BUFFER
						 (_ORBIT_send_buffer), 4);
		giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer),
					       &((*options)), sizeof((*options)));
		_ORBIT_tmpvar_1 = strlen(requested_interface) + 1;
		{
			guchar *_ORBIT_t;

			_ORBIT_t = alloca(sizeof(_ORBIT_tmpvar_1));
			memcpy(_ORBIT_t, &(_ORBIT_tmpvar_1), sizeof(_ORBIT_tmpvar_1));
			giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER
						       (_ORBIT_send_buffer), (_ORBIT_t),
						       sizeof(_ORBIT_tmpvar_1));
		}
		giop_message_buffer_append_mem(GIOP_MESSAGE_BUFFER(_ORBIT_send_buffer),
					       (requested_interface),
					       sizeof(requested_interface
						      [_ORBIT_tmpvar_0]) *
					       _ORBIT_tmpvar_1);
		giop_send_buffer_write(_ORBIT_send_buffer);
		_ORBIT_completion_status = CORBA_COMPLETED_MAYBE;
		giop_send_buffer_unuse(_ORBIT_send_buffer);
		_ORBIT_send_buffer = NULL;
		return;
	}

 _ORBIT_system_exception:
	unhook_io_channel (handle);
	CORBA_exception_set_system(handle->send_ev, _ORBIT_system_exception_minor,
				   _ORBIT_completion_status);
	giop_send_buffer_unuse(_ORBIT_send_buffer);
}

static void
resolve_demarshal (GIOChannel               *source,
		   GIOCondition              condition,
		   BonoboMonikerASyncHandle *handle)
{
	/* demarshalling */
	Bonobo_Moniker _obj = ((ResolveHandle *) handle)->moniker;
	register guchar *_ORBIT_curptr;
	register GIOPRecvBuffer *_ORBIT_recv_buffer;
	register GIOPConnection *_cnx = handle->request_cnx;
	register CORBA_completion_status _ORBIT_completion_status;
	register GIOP_unsigned_long _ORBIT_system_exception_minor;
	static const ORBit_exception_demarshal_info _ORBIT_user_exceptions[] =
	{ {(const CORBA_TypeCode) &TC_Bonobo_Moniker_InterfaceNotFound_struct,
	   (gpointer) _ORBIT_Bonobo_Moniker_InterfaceNotFound_demarshal},
	  {(const CORBA_TypeCode) &TC_Bonobo_Moniker_TimeOut_struct,
	   (gpointer) _ORBIT_Bonobo_Moniker_TimeOut_demarshal}, {CORBA_OBJECT_NIL,
								 NULL} };

	/* Not our request */
	if (!(_ORBIT_recv_buffer =
	      giop_recv_reply_buffer_use_2(_cnx, handle->request_id, FALSE)))
		return;

	unhook_io_channel (handle);

	_ORBIT_system_exception_minor = ex_CORBA_COMM_FAILURE;
	_ORBIT_completion_status = CORBA_COMPLETED_NO;
	if (!_ORBIT_recv_buffer)
		goto _ORBIT_system_exception;
	_ORBIT_completion_status = CORBA_COMPLETED_YES;
	if (_ORBIT_recv_buffer->message.u.reply.reply_status !=
	    GIOP_NO_EXCEPTION) goto _ORBIT_msg_exception;
	_ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;
	if (giop_msg_conversion_needed(GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer))) {
		GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur = _ORBIT_curptr;
		handle->object =
			ORBit_demarshal_object(_ORBIT_recv_buffer,
					       GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)->
					       connection->orb_data);
		_ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;
	} else {
		GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur = _ORBIT_curptr;
		handle->object =
			ORBit_demarshal_object(_ORBIT_recv_buffer,
					       GIOP_MESSAGE_BUFFER(_ORBIT_recv_buffer)->
					       connection->orb_data);
		_ORBIT_curptr = GIOP_RECV_BUFFER(_ORBIT_recv_buffer)->cur;
	}
	giop_recv_buffer_unuse(_ORBIT_recv_buffer);
	goto do_return;

 _ORBIT_system_exception:
	CORBA_exception_set_system(&handle->ev, _ORBIT_system_exception_minor,
				   _ORBIT_completion_status);
	giop_recv_buffer_unuse(_ORBIT_recv_buffer);
	goto do_return;

 _ORBIT_msg_exception:
	if (_ORBIT_recv_buffer->message.u.reply.reply_status ==
	    GIOP_LOCATION_FORWARD) {
		if (_obj->forward_locations != NULL)
			ORBit_delete_profiles(_obj->forward_locations);
		_obj->forward_locations = ORBit_demarshal_IOR(_ORBIT_recv_buffer);
		bonobo_moniker_async_forward (handle, 
			ORBit_object_get_forwarded_connection(_obj));
		giop_recv_buffer_unuse(_ORBIT_recv_buffer);
		
		bonobo_moniker_async_marshal (handle);
		return;
	} else {
		ORBit_handle_exception(_ORBIT_recv_buffer, &handle->ev,
				       _ORBIT_user_exceptions, _obj->orb);
		giop_recv_buffer_unuse(_ORBIT_recv_buffer);
		goto do_return;
	}
 do_return:
	bonobo_moniker_invoke_callback (handle);
}

void
bonobo_moniker_resolve_async (Bonobo_Moniker           moniker,
			      Bonobo_ResolveOptions   *options,
			      const char              *interface_name,
			      CORBA_Environment       *ev,
			      BonoboMonikerCallback    cb,
			      gpointer                 user_data)
{
	ResolveHandle *rh = g_new0 (ResolveHandle, 1);
	
	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (options != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);


	rh->moniker = moniker;
	rh->options = options;
	rh->interface_name = (char *) interface_name;

	bonobo_moniker_async_handle_construct (
		&rh->h, cb, user_data, resolve_marshal, resolve_demarshal, ev);

	/* FIXME: should we be doing this really */
	if (moniker->servant && moniker->vepv && Bonobo_Moniker__classid) {
		rh->h.object = ((POA_Bonobo_Moniker__epv *)
				moniker->vepv [Bonobo_Moniker__classid])->resolve (
					  moniker->servant, options, interface_name, ev);
		bonobo_moniker_invoke_callback (&rh->h);
	}

	bonobo_moniker_async_handle_set_connection (
		&rh->h, ORBit_object_get_connection (moniker));

	bonobo_moniker_async_marshal (&rh->h);
}

void
bonobo_moniker_resolve_async_default (Bonobo_Moniker           moniker,
				      const char              *interface_name,
				      CORBA_Environment       *ev,
				      BonoboMonikerCallback    cb,
				      gpointer                 user_data)
{
	Bonobo_ResolveOptions options;


	g_return_if_fail (ev != NULL);
	g_return_if_fail (cb != NULL);
	g_return_if_fail (moniker != CORBA_OBJECT_NIL);
	g_return_if_fail (interface_name != CORBA_OBJECT_NIL);

	init_default_resolve_options (&options);

	bonobo_moniker_resolve_async (moniker, &options, interface_name,
				      ev, cb, user_data);
}

