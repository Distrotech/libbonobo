/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

#include "liboaf/liboaf-private.h"

extern CORBA_Object oaf_server_activate_shlib (OAF_ActivationResult * sh,
					       CORBA_Environment * ev);
/**
 * oaf_query: 
 * @requirements: query string.
 * @selection_order: sort criterion for returned list.
 * @ev: a %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation.
 *
 * Executes the @requirements query on the OAF daemon.
 * The result is sorted according to @selection_order. 
 * @selection_order can safely be NULL.
 * The returned list has to be freed with CORBA_free.
 *
 * Return value: the list of servers matching the requirements.
 */

OAF_ServerInfoList *
oaf_query (const char *requirements, char *const *selection_order,
	   CORBA_Environment * ev)
{
	GNOME_stringlist selorder;
	OAF_ServerInfoList *res;
	int i;
	CORBA_Environment myev;
	OAF_ActivationContext ac;

	g_return_val_if_fail (requirements, CORBA_OBJECT_NIL);
	ac = oaf_activation_context_get ();
	g_return_val_if_fail (ac, CORBA_OBJECT_NIL);

	if (!ev) {
		ev = &myev;
		CORBA_exception_init (&myev);
	}

	if (selection_order) {
		for (i = 0; selection_order[i]; i++)
			/**/;

		selorder._length = i;
		selorder._buffer = (char **) selection_order;
		CORBA_sequence_set_release (&selorder, CORBA_FALSE);
	} else
		memset (&selorder, 0, sizeof (selorder));

	res = OAF_ActivationContext_query (ac, (char *) requirements,
					   &selorder, oaf_context_get (), ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		res = NULL;

	if (ev == &myev)
		CORBA_exception_free (&myev);

	return res;
}


/**
 * oaf_activate:
 * @requirements: query string.
 * @selection_order: sort criterion for returned list.
 * @flags: how to activate the object.
 * @ret_aid: AID of the activated object.
 * @ev: %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation. 
 *
 * Activates a given object. @ret_aid can be safely NULLed.
 *
 * Return value: the CORBA object reference of the activated object.
 * This value can be CORBA_OBJECT_NIL: you are supposed to check @ev 
 * for success.
 */
CORBA_Object
oaf_activate (const char *requirements, char *const *selection_order,
	      OAF_ActivationFlags flags, OAF_ActivationID * ret_aid,
	      CORBA_Environment * ev)
{
	GNOME_stringlist selorder;
	CORBA_Object retval = CORBA_OBJECT_NIL;
	OAF_ActivationResult *res;
	int i;
	CORBA_Environment myev;
	OAF_ActivationContext ac;

	g_return_val_if_fail (requirements, CORBA_OBJECT_NIL);
	ac = oaf_activation_context_get ();
	g_return_val_if_fail (ac, CORBA_OBJECT_NIL);

	if (!ev) {
		ev = &myev;
		CORBA_exception_init (&myev);
	}

	if (selection_order) {
		for (i = 0; selection_order[i]; i++)
			/**/;

		selorder._length = i;
		selorder._buffer = (char **) selection_order;
		CORBA_sequence_set_release (&selorder, CORBA_FALSE);
	} else
		memset (&selorder, 0, sizeof (selorder));

	res = OAF_ActivationContext_activate (ac, (char *) requirements,
					      &selorder, flags,
					      oaf_context_get (), ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		goto out;

	switch (res->res._d) {
	case OAF_RESULT_SHLIB:
		retval = oaf_server_activate_shlib (res, ev);
		break;
	case OAF_RESULT_OBJECT:
		retval = CORBA_Object_duplicate (res->res._u.res_object, ev);
		break;
	case OAF_RESULT_NONE:
	default:
		break;
	}

	if (ret_aid) {
		*ret_aid = NULL;
		if (*res->aid)
			*ret_aid = g_strdup (res->aid);
	}

	CORBA_free (res);

      out:
	if (ev == &myev)
		CORBA_exception_free (&myev);

	return retval;
}

/**
 * oaf_activate_from_id
 * @aid: AID or IID of the object to activate.
 * @flags: activation flag.
 * @ret_aid: AID of the activated server.
 * @ev: %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation. 
 *
 * Activates the server corresponding to @aid. @ret_aid can be safely NULLed.
 *
 * Return value: a CORBA object reference to the newly activated server.
 */

CORBA_Object
oaf_activate_from_id (const OAF_ActivationID aid, OAF_ActivationFlags flags,
		      OAF_ActivationID * ret_aid, CORBA_Environment * ev)
{
	CORBA_Object retval = CORBA_OBJECT_NIL;
	OAF_ActivationResult *res;
	CORBA_Environment myev;
	OAF_ActivationContext ac;
	OAFActivationInfo *ai;

	g_return_val_if_fail (aid, CORBA_OBJECT_NIL);
	ac = oaf_activation_context_get ();
	g_return_val_if_fail (ac, CORBA_OBJECT_NIL);

	ai = oaf_actid_parse (aid);

	if (ai) {		/* This is so that using an AID in an unactivated OD will work nicely */
		OAFRegistrationCategory regcat;

		memset (&regcat, 0, sizeof (regcat));
		regcat.name = "IDL:OAF/ObjectDirectory:1.0";
		regcat.session_name = oaf_session_name_get ();
		regcat.username = ai->user;
		regcat.hostname = ai->host;
		regcat.domain = ai->domain;

		oaf_service_get (&regcat);

		oaf_actinfo_free (ai);
	}

	if (!ev) {
		ev = &myev;
		CORBA_exception_init (&myev);
	}

	res =
		OAF_ActivationContext_activate_from_id (ac, aid, flags,
							oaf_context_get (),
							ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		goto out;

	switch (res->res._d) {
	case OAF_RESULT_SHLIB:
                retval = oaf_server_activate_shlib (res, ev);
		break;
	case OAF_RESULT_OBJECT:
		retval = CORBA_Object_duplicate (res->res._u.res_object, ev);
		break;
	case OAF_RESULT_NONE:
	default:
		break;
	}

	if (ret_aid) {
		*ret_aid = NULL;
		if (*res->aid)
			*ret_aid = g_strdup (res->aid);
	}

	CORBA_free (res);

      out:
	if (ev == &myev)
		CORBA_exception_free (&myev);

	return retval;
}

/**
 * oaf_name_service_get:
 * @ev: %CORBA_Environment structure which will contain 
 *      the CORBA exception status of the operation. 
 *
 * Returns the name server of OAF.
 *
 * Return value: the name server of OAF.
 */
CORBA_Object oaf_name_service_get (CORBA_Environment * ev)
{

	return oaf_activate_from_id ("OAFIID:oaf_naming_service:7e2b90ef-eaf0-4239-bb7c-812606fcd80d",
				     0, NULL, ev);
}
