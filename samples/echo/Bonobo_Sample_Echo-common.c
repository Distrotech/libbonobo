/*
 * This file was generated by orbit-idl-2 - DO NOT EDIT!
 */

#include <string.h>
#define ORBIT2_STUBS_API
#define ORBIT_IDL_C_COMMON
#define Bonobo_Sample_Echo_COMMON
#include "Bonobo_Sample_Echo.h"

static const CORBA_unsigned_long ORBit_zero_int = 0;

#ifndef ORBIT_IDL_C_IMODULE_Bonobo_Sample_Echo
void _ORBIT_skel_small_Bonobo_Sample_Echo_echo(POA_Bonobo_Sample_Echo             *_o_servant, gpointer            _o_retval,gpointer           *_o_args,CORBA_Context       _o_ctx,CORBA_Environment  *_o_ev,
void (*_impl_echo)(PortableServer_Servant _servant, const CORBA_char * message, CORBA_Environment *ev)) {
_impl_echo (_o_servant, *(const CORBA_char * *)_o_args[0], _o_ev);
}

#endif
#if ( (TC_IMPL_TC_Bonobo_Sample_Echo_0 == 'B') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_1 == 'o') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_2 == 'n') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_3 == 'o') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_4 == 'b') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_5 == 'o') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_6 == '_') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_7 == 'S') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_8 == 'a') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_9 == 'm') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_10 == 'p') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_11 == 'l') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_12 == 'e') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_13 == '_') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_14 == 'E') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_15 == 'c') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_16 == 'h') \
&& (TC_IMPL_TC_Bonobo_Sample_Echo_17 == 'o') \
) && !defined(TC_DEF_TC_Bonobo_Sample_Echo)
#define TC_DEF_TC_Bonobo_Sample_Echo 1
#ifdef ORBIT_IDL_C_IMODULE_Bonobo_Sample_Echo
static
#endif
ORBIT2_MAYBE_CONST struct CORBA_TypeCode_struct TC_Bonobo_Sample_Echo_struct = {
{&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
CORBA_tk_objref,
0,
0,
ORBIT_ALIGNOF_CORBA_POINTER,
0,
0
,
NULL,
CORBA_OBJECT_NIL,
(char *)"Echo",
(char *)"IDL:Bonobo/Sample/Echo:1.0",
NULL,
NULL,
-1,
0,
0, 0
};
#endif

#ifndef ORBIT_IDL_C_IMODULE_Bonobo_Sample_Echo
CORBA_unsigned_long Bonobo_Sample_Echo__classid = 0;
#endif

/* Interface type data */

static ORBit_IArg Bonobo_Sample_Echo_echo__arginfo [] = {
	{ TC_CORBA_string,  ORBit_I_ARG_IN , (char *)"message" }
};

#ifdef ORBIT_IDL_C_IMODULE_Bonobo_Sample_Echo
static
#endif
ORBit_IMethod Bonobo_Sample_Echo__imethods [] = {
	{
		{ 1, 1, Bonobo_Sample_Echo_echo__arginfo, FALSE },
		{ 0, 0, NULL, FALSE },
		{ 0, 0, NULL, FALSE },
TC_void, (char *)"echo", 4,
		0
}
};

static CORBA_string Bonobo_Sample_Echo__base_itypes[] = {
(char *)"IDL:Bonobo/Unknown:1.0",
(char *)"IDL:omg.org/CORBA/Object:1.0"
};
#ifdef ORBIT_IDL_C_IMODULE_Bonobo_Sample_Echo
static
#endif
ORBit_IInterface Bonobo_Sample_Echo__iinterface = {
TC_Bonobo_Sample_Echo,{1, 1, Bonobo_Sample_Echo__imethods, FALSE},
{2, 2, Bonobo_Sample_Echo__base_itypes, FALSE}
};

