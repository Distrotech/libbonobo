#include "oafd.h"
#include "ac-query-expr.h"
#include <time.h>

static void
OAF_ServerInfo__copy(OAF_ServerInfo *new, const OAF_ServerInfo *old)
{
  new->iid = CORBA_string_dup(old->iid);
  new->server_type = CORBA_string_dup(old->server_type);
  new->location_info = CORBA_string_dup(old->location_info);
  new->attrs = old->attrs;
  CORBA_sequence_set_release(&new->attrs, CORBA_FALSE);
}

typedef struct {
  OAF_ObjectDirectory obj;

  char *hostname, *username, *domain;

  OAF_ServerInfoList *list;
  OAF_CacheTime time_list_pulled;
  GHashTable *by_iid;

  GHashTable *active_servers;
  OAF_ServerStateCache *active_server_list;
  OAF_CacheTime time_active_pulled;

  guchar locked;
} ChildODInfo;

static ChildODInfo *
child_od_info_new(OAF_ObjectDirectory obj, CORBA_Environment *ev)
{
  ChildODInfo *retval;
  char *host, *domain, *user;

  host = OAF_ObjectDirectory__get_hostname(obj, ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto errhost;

  domain = OAF_ObjectDirectory__get_domain(obj, ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto errdomain;

  user = OAF_ObjectDirectory__get_username(obj, ev);
  if(ev->_major != CORBA_NO_EXCEPTION)
    goto erruser;

  retval = g_new0(ChildODInfo, 1);

  retval->obj = CORBA_Object_duplicate(obj, ev);
  retval->hostname = host;
  retval->username = user;
  retval->domain = domain;

  return retval;

 erruser:
  CORBA_free(domain);
 errdomain:
  CORBA_free(host);
 errhost:
  return NULL;
}

static void
child_od_exception(ChildODInfo *child, CORBA_Environment *ev)
{
  CORBA_Object_release(child->obj, ev);
  child->obj = CORBA_OBJECT_NIL;
}

static void
child_od_update_active(ChildODInfo *child, CORBA_Environment *ev)
{
  int i;
  OAF_ServerStateCache *cache;

  cache = OAF_ObjectDirectory_get_active_servers(child->obj,
						 child->time_active_pulled,
						 ev);
  if(ev->_major != CORBA_NO_EXCEPTION) {
    child_od_exception(child, ev);
    return;
  }

  if(cache->_d) {
    if(child->active_servers) {
      g_hash_table_destroy(child->active_servers);
      CORBA_free(child->active_server_list);
    }

    child->active_server_list = cache;

    child->time_active_pulled = time(NULL);
    child->active_servers = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_freeze(child->active_servers);
    for(i = 0; i < cache->_u.active_servers._length; i++)
      g_hash_table_insert(child->active_servers,
			  cache->_u.active_servers._buffer[i],
			  GINT_TO_POINTER(1));
    g_hash_table_thaw(child->active_servers);
  } else
    CORBA_free(cache);
}

static void
child_od_update_list(ChildODInfo *child, CORBA_Environment *ev)
{
  int i;
  OAF_ServerInfoListCache *cache;

  cache = OAF_ObjectDirectory_get_servers(child->obj,
					  child->time_list_pulled,
					  ev);
  if(ev->_major != CORBA_NO_EXCEPTION) {
    child->list = NULL;
    child_od_exception(child, ev);
    return;
  }

  if(cache->_d) {

    if(child->by_iid)
      g_hash_table_destroy(child->by_iid);
    CORBA_free(child->list); child->list = NULL;

    child->list = OAF_ServerInfoList__alloc();
    *(child->list) = cache->_u.server_list;

    child->time_list_pulled = time(NULL);
    child->by_iid = g_hash_table_new(g_str_hash, g_str_equal);
    g_hash_table_freeze(child->by_iid);
    for(i = 0; i < child->list->_length; i++)
      g_hash_table_insert(child->by_iid,
			  child->list->_buffer[i].iid,
			  &(child->list->_buffer[i]));
    g_hash_table_thaw(child->by_iid);
  }

  CORBA_free(cache);
}

static void
child_od_info_free(ChildODInfo *child, CORBA_Environment *ev)
{
  CORBA_Object_release(child->obj, ev);
  CORBA_free(child->hostname);
  CORBA_free(child->username);
  CORBA_free(child->domain);
  g_free(child);
}

/*** App-specific servant structures ***/

typedef struct {
  POA_OAF_ActivationContext servant;
  PortableServer_POA poa;

  GSList *dirs;

  int total_servers;
} impl_POA_OAF_ActivationContext;

/*** Implementation stub prototypes ***/

static OAF_ObjectDirectoryList *
impl_OAF_ActivationContext__get_directories(impl_POA_OAF_ActivationContext *servant,
					     CORBA_Environment * ev);

static void
 impl_OAF_ActivationContext_add_directory(impl_POA_OAF_ActivationContext * servant,
					  OAF_ObjectDirectory dir,
					  CORBA_Environment * ev);

static void
 impl_OAF_ActivationContext_remove_directory(impl_POA_OAF_ActivationContext * servant,
					     OAF_ObjectDirectory dir,
					     CORBA_Environment * ev);

static OAF_ActivationResult *
 impl_OAF_ActivationContext_activate(impl_POA_OAF_ActivationContext * servant,
				     CORBA_char * requirements,
				     OAF_StringList * selection_order,
				     CORBA_Context ctx,
				     CORBA_Environment * ev);

static OAF_ServerInfoList *
 impl_OAF_ActivationContext__get_servers(impl_POA_OAF_ActivationContext * servant,
					 CORBA_Environment * ev);

static OAF_ServerInfoList *
impl_OAF_ActivationContext_query(impl_POA_OAF_ActivationContext * servant,
				 CORBA_char * requirements,
				 OAF_StringList * selection_order,
				 CORBA_Context ctx,
				 CORBA_Environment * ev);

static OAF_ActivationResult *
impl_OAF_ActivationContext_activate_from_id(impl_POA_OAF_ActivationContext * servant,
					    OAF_ActivationID aid,
					    CORBA_Context ctx,
					    CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_OAF_ActivationContext_base_epv =
{
  NULL,			/* _private data */
  NULL,			/* finalize routine */
  NULL			        /* default_POA routine */
};
static POA_OAF_ActivationContext__epv impl_OAF_ActivationContext_epv =
{
  NULL,			/* _private */
  &impl_OAF_ActivationContext__get_directories,
  &impl_OAF_ActivationContext_add_directory,
  &impl_OAF_ActivationContext_remove_directory,
  &impl_OAF_ActivationContext_activate,
  &impl_OAF_ActivationContext__get_servers,
  &impl_OAF_ActivationContext_query,
  &impl_OAF_ActivationContext_activate_from_id
};

/*** vepv structures ***/

static POA_OAF_ActivationContext__vepv impl_OAF_ActivationContext_vepv =
{
   &impl_OAF_ActivationContext_base_epv,
   &impl_OAF_ActivationContext_epv
};

/*** Stub implementations ***/

OAF_ActivationContext
OAF_ActivationContext_create(PortableServer_POA poa,
			     CORBA_Environment *ev)
{
   OAF_ActivationContext retval;
   impl_POA_OAF_ActivationContext *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_OAF_ActivationContext, 1);
   newservant->servant.vepv = &impl_OAF_ActivationContext_vepv;
   newservant->poa = poa;
   POA_OAF_ActivationContext__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

static void
ac_update_list(impl_POA_OAF_ActivationContext *servant,
	       ChildODInfo *child, CORBA_Environment *ev)
{
  int prev, new;

  if(child->list)
    prev = child->list->_length;
  else
    prev = 0;

  child_od_update_list(child, ev);

  if(child->list)
    new = child->list->_length;
  else
    new = 0;

  servant->total_servers += (new - prev);
}

static OAF_ObjectDirectoryList *
impl_OAF_ActivationContext__get_directories(impl_POA_OAF_ActivationContext * servant,
					    CORBA_Environment * ev)
{
   OAF_ObjectDirectoryList *retval;
   int i;
   GSList *cur;

   retval = OAF_ObjectDirectoryList__alloc();
   retval->_length = g_slist_length(servant->dirs);
   retval->_buffer =
     CORBA_sequence_OAF_ObjectDirectory_allocbuf(retval->_length);

   for(i = 0, cur = servant->dirs; cur; cur = cur->next) {
     ChildODInfo *child;
     child = cur->data;
     retval->_buffer[i] = CORBA_Object_duplicate(child->obj, ev);
   }

   CORBA_sequence_set_release(retval, CORBA_TRUE);

   return retval;
}

static void
impl_OAF_ActivationContext_add_directory(impl_POA_OAF_ActivationContext * servant,
					 OAF_ObjectDirectory dir,
					 CORBA_Environment * ev)
{
  GSList *cur;
  ChildODInfo *new_child;

  for(cur = servant->dirs; cur; cur = cur->next) {
    ChildODInfo *child;
    child = cur->data;
    if(CORBA_Object_is_equivalent(dir, child->obj, ev)) {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_OAF_ActivationContext_AlreadyListed, NULL);
      return;
    }
  }

  new_child = child_od_info_new(dir, ev);
  if(new_child)
    servant->dirs = g_slist_append(servant->dirs, new_child);
}

static void
impl_OAF_ActivationContext_remove_directory(impl_POA_OAF_ActivationContext * servant,
					    OAF_ObjectDirectory dir,
					    CORBA_Environment * ev)
{
  GSList *cur;

  for(cur = servant->dirs; cur; cur = cur->next) {
    ChildODInfo *child;
    child = cur->data;
    if(CORBA_Object_is_equivalent(dir, child->obj, ev)) {
      servant->dirs = g_slist_remove(servant->dirs, child);
      child_od_info_free(child, ev);
      break;
    }
  }
  if(!cur)
    CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			ex_OAF_ActivationContext_NotListed, NULL);
}

static OAF_ActivationResult *
impl_OAF_ActivationContext_activate(impl_POA_OAF_ActivationContext * servant,
				    CORBA_char * requirements,
				    OAF_StringList * selection_order,
				    CORBA_Context ctx,
				    CORBA_Environment * ev)
{
   OAF_ActivationResult *retval;

   return retval;
}

static void
ac_update_lists(impl_POA_OAF_ActivationContext *servant,
		CORBA_Environment *ev)
{
  GSList *cur;

  for(cur = servant->dirs; cur; cur = cur->next)
    ac_update_list(servant, cur->data, ev);
}

static OAF_ServerInfoList *
impl_OAF_ActivationContext__get_servers(impl_POA_OAF_ActivationContext * servant,
					CORBA_Environment * ev)
{
  OAF_ServerInfoList *retval;
  GSList *cur;
  int i;
  int total;

  ac_update_lists(servant, ev);

  total = servant->total_servers;

  retval = OAF_ServerInfoList__alloc();
  retval->_length = total;
  retval->_buffer = CORBA_sequence_OAF_ServerInfo_allocbuf(total);
  CORBA_sequence_set_release(retval, CORBA_TRUE);

  for(i = 0; i < total; ) {
    for(cur = servant->dirs; cur; cur = cur->next) {
      ChildODInfo *child;
      int j;

      child = cur->data;

      for(j = 0; j < child->list->_length; j++, i++)
	OAF_ServerInfo__copy(&retval->_buffer[i], &child->list->_buffer[j]);
    }
  }

  return retval;
}

static ChildODInfo *
ac_find_child_for_server(impl_POA_OAF_ActivationContext *servant,
			 OAF_ServerInfo *server,
			 CORBA_Environment *ev)
{
  GSList *cur;

  for(cur = servant->dirs; cur; cur = cur->next) {
    ChildODInfo *child;

    if(CORBA_Object_is_nil(child->obj, ev) || !child->list)
      continue;

    if((server > child->list->_buffer)
       && (server < (child->list->_buffer + child->list->_length)))
      return child;
  }

  return NULL;
}

static QueryExprConst
ac_query_get_var(OAF_ServerInfo *si, const char *id, QueryContext *qctx)
{
  ChildODInfo *child;
  QueryExprConst retval;

  retval.value_known = FALSE;
  retval.needs_free = FALSE;

  child = ac_find_child_for_server(qctx->user_data, si, NULL);
  if(!child)
    goto out;

  if(!strcasecmp(id, "_hostname")) {
    retval.value_known = TRUE;
    retval.type = CONST_STRING;
    retval.u.v_string = child->hostname;
  } else if (!strcasecmp(id, "_domain")) {
    retval.value_known = TRUE;
    retval.type = CONST_STRING;
    retval.u.v_string = child->domain;
  } else if (!strcasecmp(id, "_username")) {
    retval.value_known = TRUE;
    retval.type = CONST_STRING;
    retval.u.v_string = child->username;
  } else if (!strcasecmp(id, "_active")) {
    CORBA_Environment ev;

    CORBA_exception_init(&ev);
    child_od_update_active(child, &ev);
    CORBA_exception_free(&ev);

    retval.value_known = TRUE;
    retval.type = CONST_BOOLEAN;
    retval.u.v_boolean = g_hash_table_lookup(child->active_servers, si->iid)?TRUE:FALSE;
  }

 out:

  return retval;
}


static OAF_ServerInfoList *
impl_OAF_ActivationContext_query(impl_POA_OAF_ActivationContext * servant,
				 CORBA_char * requirements,
				 OAF_StringList * selection_order,
				 CORBA_Context ctx,
				 CORBA_Environment * ev)
{
  OAF_ServerInfoList *retval;
  int total, i, j;
  GSList *cur;
  QueryContext qctx;

  OAF_ServerInfo **items, **orig_items;
  int item_count, orig_item_count;
  char *errstr;
  OAF_ActivationContext_ParseFailed *ex;

  QueryExpr *qexp_requirements;
  QueryExpr **qexp_sort_items;

  retval = OAF_ServerInfoList__alloc();
  retval->_length = 0;
  retval->_buffer = NULL;
  CORBA_sequence_set_release(retval, CORBA_TRUE);

  /* First, parse the query */
  errstr = (char *)qexp_parse(requirements, &qexp_requirements);
  if(errstr) {
    ex = OAF_ActivationContext_ParseFailed__alloc();
    ex->description = CORBA_string_dup(errstr);
    g_free(errstr);
    CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			ex_OAF_ActivationContext_ParseFailed, ex);
    return retval;
  }

  qexp_sort_items = oaf_alloca(selection_order->_length * sizeof(QueryExpr *));
  for(i = 0; i < selection_order->_length; i++) {
    errstr = (char *)qexp_parse(selection_order->_buffer[i], &qexp_sort_items[i]);

    if(errstr) {
      qexp_free(qexp_requirements);
      for(i--; i >= 0; i--)
	qexp_free(qexp_sort_items[i]);

      ex = OAF_ActivationContext_ParseFailed__alloc();
      ex->description = CORBA_string_dup(errstr);
      g_free(errstr);

      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_OAF_ActivationContext_ParseFailed, ex);
      return retval;
    }
  }

  /* Then, pull in new lists from OD servers */
  ac_update_lists(servant, ev);

  total = servant->total_servers;
  items = oaf_alloca(total * sizeof(OAF_ServerInfo *));
  orig_items = oaf_alloca(total * sizeof(OAF_ServerInfo *));

  for(item_count = 0, cur = servant->dirs; cur; cur = cur->next) {
    ChildODInfo *child;
    int i;

    child = cur->data;

    if(CORBA_Object_is_nil(child->obj, ev) || !child->list)
      continue;

    for(i = 0; i < child->list->_length; i++, item_count++)
      items[item_count] = &child->list->_buffer[i];
  }
  memcpy(orig_items, items, item_count * sizeof(OAF_ServerInfo *));
  orig_item_count = item_count;

  qctx.sil = orig_items;
  qctx.nservers = orig_item_count;
  qctx.cctx = ctx;
  qctx.id_evaluator = ac_query_get_var;
  qctx.user_data = servant;

  for(i = 0; i < item_count; i++) {
    if(!qexp_matches(items[i], qexp_requirements, &qctx))
      items[i] = NULL;
  }

  qexp_sort(items, item_count, qexp_sort_items, selection_order->_length, &qctx);

  for(total = i = 0; i < item_count; i++) {
    if(items[i])
      total++;
  }

  retval->_length = total;
  retval->_buffer = CORBA_sequence_OAF_ServerInfo_allocbuf(total);
  for(i = j = 0; i < item_count; i++) {
    if(!items[i])
      continue;

    OAF_ServerInfo__copy(&retval->_buffer[j], items[i]);

    j++;
  }

  return retval;
}

static OAF_ActivationResult *
impl_OAF_ActivationContext_activate_from_id(impl_POA_OAF_ActivationContext * servant,
					    OAF_ActivationID aid,
					    CORBA_Context ctx,
					    CORBA_Environment * ev)
{
  OAF_ActivationResult *retval;

  retval = OAF_ActivationResult__alloc();

  return retval;
}
