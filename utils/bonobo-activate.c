#include "config.h"

#include "liboaf/liboaf.h"
#include "oafd.h"
#include <stdio.h>
#include <popt.h>

static char *query=NULL, *acior=NULL;

static struct poptOption options[] = {
  {"ac-ior", '\0', POPT_ARG_STRING, &acior, 0, "IOR of ActivationContext to use", "IOR"},
  {"query", 'q', POPT_ARG_STRING, &query, 0, "Object database query to run", "QUERY"},
  POPT_AUTOHELP
  {NULL}
};

static void
od_dump_list(OAF_ServerInfoList *list)
{
  int i, j, k;

  for(i = 0; i < list->_length; i++) {
    g_print("IID %s, type %s, location %s\n",
	    list->_buffer[i].iid,
	    list->_buffer[i].server_type,
	    list->_buffer[i].location_info);
    for(j = 0; j < list->_buffer[i].attrs._length; j++) {
      OAF_Attribute *attr = &(list->_buffer[i].attrs._buffer[j]);
      g_print("    %s = ", attr->name);
      switch(attr->v._d) {
      case OAF_A_STRING:
	g_print("\"%s\"\n", attr->v._u.value_string);
	break;
      case OAF_A_NUMBER:
	g_print("%f\n", attr->v._u.value_number);
	break;
      case OAF_A_BOOLEAN:
	g_print("%s\n", attr->v._u.value_boolean?"TRUE":"FALSE");
	break;
      case OAF_A_STRINGV:
	g_print("[");
	for(k = 0; k < attr->v._u.value_stringv._length; k++) {
	  g_print("\"%s\"", attr->v._u.value_stringv._buffer[k]);
	  if(k < (attr->v._u.value_stringv._length - 1))
	    g_print(", ");
	}
	g_print("]\n");
	break;
      }
    }
  }
}

int main(int argc, char *argv[])
{
  CORBA_Environment ev;
  OAF_ActivationContext ac;
  poptContext ctx;
  gboolean do_usage_exit = FALSE;
  OAF_ServerInfoList *slist;
  CORBA_ORB orb;

  CORBA_exception_init(&ev);

  ctx = poptGetContext("oaf-client", argc, argv, options, 0);
  while(poptGetNextOpt(ctx) >= 0) /**/;

  orb = oaf_orb_init(&argc, argv);

  if(!acior) {
    g_print("You must specify an ActivationContext IOR.\n");
    do_usage_exit = TRUE;
  }

  if(!query) {
    g_print("You must specify an operation to perform.\n");
    do_usage_exit = TRUE;
  }

  ac = CORBA_ORB_string_to_object(orb, acior, &ev);
  if(ev._major != CORBA_NO_EXCEPTION) {
    g_print("Error doing string_to_object(%s)\n", acior);
    do_usage_exit = TRUE;
  }

  if(do_usage_exit) {
    poptPrintUsage(ctx, stdout, 0);
    return 1;
  }
  poptFreeContext(ctx);

  {
    GNOME_stringlist reqs = {0};

    slist = OAF_ActivationContext_query(ac, query, &reqs, oaf_context_get(), &ev);
    switch(ev._major) {
    case CORBA_NO_EXCEPTION:
      od_dump_list(slist);
      CORBA_free(slist);
      break;
    case CORBA_USER_EXCEPTION:
      {
	char *id;
	id = CORBA_exception_id(&ev);
	g_print("User exception \"%s\" resulted from query\n",
		id);
	if(!strcmp(id, "IDL:OAF/ActivationContext/ParseFailed:1.0")) {
	  OAF_ActivationContext_ParseFailed *exdata = CORBA_exception_value(&ev);

	  if(exdata)
	    g_print("Description: %s\n", exdata->description);
	}
      }
      break;
    case CORBA_SYSTEM_EXCEPTION:
      {
	char *id;
	id = CORBA_exception_id(&ev);
	g_print("System exception \"%s\" resulted from query\n",
		id);
      }
      break;
    }
  }

  CORBA_exception_free(&ev);

  return 0;
}
