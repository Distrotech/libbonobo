/*
 * Bonobo argument support:
 *
 *  A thin wrapper of CORBA_any's with macros
 * to assist in handling values safely.
 *
 * Author:
 *    Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */

#include <config.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-arg.h>

BonoboArg *
bonobo_arg_new (BonoboArgType t)
{
	CORBA_any *any;
	size_t     size;

	any = CORBA_any__alloc ();
	any->_type = t;

	size = ORBit_gather_alloc_info (t);
	/*
	 *  If we were paranoid we would memzero this buffer.
	 */
	any->_value = CORBA_octet_allocbuf (size);
	CORBA_any_set_release (any, TRUE);

	return any;
}

BonoboArg *
bonobo_arg_new_default (BonoboArgType t)
{	
	CORBA_any *any;
	size_t     size;

	any = CORBA_any__alloc ();
	any->_type = t;

	size = ORBit_gather_alloc_info (t);
	any->_value = CORBA_octet_allocbuf (size);
	/*
	 * Hope this is good enough.
	 */
	memset (any->_value, size, 0);
	CORBA_any_set_release (any, TRUE);

	return any;
}

void
bonobo_arg_release (BonoboArg *a)
{
	if (!a)
		return;

	CORBA_free (a);	
}

BonoboArg *
bonobo_arg_copy (BonoboArg *a)
{
	BonoboArg *copy = CORBA_any__alloc ();

	if (!a) {
		copy->_type = TC_null;
		g_warning ("Duplicating a NULL Bonobo Arg");
	} else
		CORBA_any__copy (copy, a);

	return copy;
}
