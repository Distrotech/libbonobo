/*
 * This file was generated by orbit-idl-2 - DO NOT EDIT!
 */

#include <string.h>
#define ORBIT2_STUBS_API
#include "Bonobo_Sample_Echo.h"

static ORBitSmallSkeleton get_skel_small_Bonobo_Sample_Echo(POA_Bonobo_Sample_Echo *servant,
const char *opname,gpointer *m_data, gpointer *impl)
{
switch(opname[0]) {
case 'e':
if(strcmp((opname + 1), "cho")) break;
*impl = (gpointer)servant->vepv->Bonobo_Sample_Echo_epv->echo;
{ORBit_IInterface *volatile _t_=&Bonobo_Sample_Echo__iinterface;*m_data = (gpointer)&_t_->methods._buffer [0];}
return (ORBitSmallSkeleton)_ORBIT_skel_small_Bonobo_Sample_Echo_echo;
break;
case 'q':
if(strcmp((opname + 1), "ueryInterface")) break;
*impl = (gpointer)servant->vepv->Bonobo_Unknown_epv->queryInterface;
{ORBit_IInterface *volatile _t_=&Bonobo_Unknown__iinterface;*m_data = (gpointer)&_t_->methods._buffer [2];}
return (ORBitSmallSkeleton)_ORBIT_skel_small_Bonobo_Unknown_queryInterface;
break;
case 'r':
if(strcmp((opname + 1), "ef")) break;
*impl = (gpointer)servant->vepv->Bonobo_Unknown_epv->ref;
{ORBit_IInterface *volatile _t_=&Bonobo_Unknown__iinterface;*m_data = (gpointer)&_t_->methods._buffer [0];}
return (ORBitSmallSkeleton)_ORBIT_skel_small_Bonobo_Unknown_ref;
break;
case 'u':
if(strcmp((opname + 1), "nref")) break;
*impl = (gpointer)servant->vepv->Bonobo_Unknown_epv->unref;
{ORBit_IInterface *volatile _t_=&Bonobo_Unknown__iinterface;*m_data = (gpointer)&_t_->methods._buffer [1];}
return (ORBitSmallSkeleton)_ORBIT_skel_small_Bonobo_Unknown_unref;
break;
default: break; 
}
return NULL;
}

void POA_Bonobo_Sample_Echo__init(PortableServer_Servant servant,
CORBA_Environment *env)
{
  static PortableServer_ClassInfo class_info = {NULL, (ORBit_small_impl_finder)&get_skel_small_Bonobo_Sample_Echo, "IDL:Bonobo/Sample/Echo:1.0", &Bonobo_Sample_Echo__classid, NULL, &Bonobo_Sample_Echo__iinterface};
  PortableServer_ServantBase__init (       ((PortableServer_ServantBase *)servant), env);
  POA_Bonobo_Unknown__init(servant, env);
   ORBit_skel_class_register (&class_info,
   (PortableServer_ServantBase *)servant, POA_Bonobo_Sample_Echo__fini,
   ORBIT_VEPV_OFFSET (POA_Bonobo_Sample_Echo__vepv, Bonobo_Sample_Echo_epv),
(CORBA_unsigned_long) Bonobo_Unknown__classid,
ORBIT_VEPV_OFFSET (POA_Bonobo_Sample_Echo__vepv, Bonobo_Unknown_epv),
   (CORBA_unsigned_long) 0);}

void POA_Bonobo_Sample_Echo__fini(PortableServer_Servant servant,
CORBA_Environment *env)
{
  POA_Bonobo_Unknown__fini(servant, env);
  PortableServer_ServantBase__fini(servant, env);
}

