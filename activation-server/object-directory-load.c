#include "oafd.h"
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <parser.h>

#define my_slist_prepend(slist, datum) \
new_item = oaf_alloca(sizeof(GSList)); \
new_item->next = slist; \
new_item->data = datum; \
slist = new_item;

static gboolean
od_string_to_boolean(const char *str)
{
  if(!strcasecmp(str, "true")
     || !strcasecmp(str, "yes")
     || !strcmp(str, "1"))
    return TRUE;
  else
    return FALSE;
}

static void
od_entry_read_attrs(OAF_ServerInfo *ent, xmlNodePtr node)
{
  int i, n;
  xmlNodePtr sub;
  OAF_Attribute *curattr;

  for(n = 0, sub = node->childs; sub; sub = sub->next) {
    if(sub->type != XML_ELEMENT_NODE)
      continue;

    if(strcasecmp(sub->name, "oaf_attribute"))
      continue;

    n++;
  }

  ent->attrs._length = n;
  ent->attrs._buffer = curattr = g_new(OAF_Attribute, n);
  
  for(i = 0, sub = node->childs; i < n; sub = sub->next, i++) {
    char *type, *valuestr;

    type = xmlGetProp(sub, "type");
    if(!type)
      continue;

    valuestr = xmlGetProp(sub, "name");
    if(!valuestr) {
      free(type);
      continue;
    }
    curattr->name = CORBA_string_dup(valuestr);
    free(valuestr);

    if(!strcasecmp(type, "stringv")) {
      int j, o;
      xmlNodePtr sub2;

      curattr->v._d = OAF_A_STRINGV;

      for(o = 0, sub2 = sub->childs; sub2; sub2 = sub2->next) {
	if(sub2->type != XML_ELEMENT_NODE)
	  continue;
	if(strcasecmp(sub2->name, "item"))
	  continue;

	o++;
      }

      curattr->v._u.value_stringv._length = o;
      curattr->v._u.value_stringv._buffer = CORBA_sequence_CORBA_string_allocbuf(o);

      for(j = 0, sub2 = sub->childs; j < o; sub2 = sub2->next, j++) {
	valuestr = xmlGetProp(sub2, "value");
	if (valuestr)
		curattr->v._u.value_stringv._buffer[j] = CORBA_string_dup(valuestr);
	else {
		g_warning ("Attribute '%s' has no value", curattr->name);
		curattr->v._u.value_stringv._buffer[j] = CORBA_string_dup("");
	}
	free(valuestr);
      }
      
    } else if(!strcasecmp(type, "number")) {
      valuestr = xmlGetProp(sub, "value");

      curattr->v._d = OAF_A_NUMBER;
      curattr->v._u.value_number = atof(valuestr);

      free(valuestr);
    } else if(!strcasecmp(type, "boolean")) {
      valuestr = xmlGetProp(sub, "value");
      curattr->v._d = OAF_A_BOOLEAN;
      curattr->v._u.value_boolean = od_string_to_boolean(valuestr);
      free(valuestr);
    } else {
      valuestr = xmlGetProp(sub, "value");
      /* Assume string */
      curattr->v._d = OAF_A_STRING;
      if (valuestr)
	      curattr->v._u.value_string = CORBA_string_dup(valuestr);
      else {
	      g_warning ("Attribute '%s' has no value", curattr->name);
	      curattr->v._u.value_string = CORBA_string_dup("");
      }
      free(valuestr);
    }

    free(type);

    curattr++;
  }
}

OAF_ServerInfo *
OAF_ServerInfo_load(const char *source_directory,
		    CORBA_unsigned_long *nservers,
		    GHashTable **by_iid,
		    const char *user,
		    const char *host,
		    const char *domain)
{
  DIR *dirh;
  struct dirent *dent;
  char tmpstr[PATH_MAX];
  GSList *entries = NULL, *cur, *new_item;
  int i, n;
  OAF_ServerInfo *retval;
  char **dirs;
  int dirnum;

  g_return_val_if_fail(source_directory, NULL);
  g_return_val_if_fail(nservers, NULL);
  g_return_val_if_fail(by_iid, NULL);

  if(*by_iid)
    g_hash_table_destroy(*by_iid);
  *by_iid = g_hash_table_new(g_str_hash, g_str_equal);

  dirs = g_strsplit(source_directory, ":", -1);

  *nservers = 0;

  for(dirnum = 0; dirs[dirnum]; dirnum++)
    {
      g_print("Trying dir %s\n", dirs[dirnum]);
      dirh = opendir(dirs[dirnum]);
      if(!dirh)
	continue;
  
      while((dent = readdir(dirh)))
	{
	  char *ext;
	  xmlDocPtr doc;
	  xmlNodePtr curnode;

	  ext = strrchr(dent->d_name, '.');
	  if(!ext || strcasecmp(ext, ".oafinfo"))
	    continue;
    
	  g_snprintf(tmpstr, sizeof(tmpstr), "%s/%s", dirs[dirnum], dent->d_name);

	  doc = xmlParseFile(tmpstr);
	  if(!doc)
	    continue;

	  /* This should go in a separate function, but I'm sticking it in
	     here so alloca can be used. "Eeeek!" is still fine as a
	     response, :) but this has a direct impact on startup time. */

	  for (curnode = (!strcasecmp (doc->root->name, "oaf_info") 
			 ? doc->root->childs : doc->root)
		; NULL != curnode; curnode = curnode->next)
	    {
	      OAF_ServerInfo *new_ent;
	      char *ctmp;

	      if(curnode->type != XML_ELEMENT_NODE)
		continue;

	      /* I'd love to use XML namespaces, but unfortunately they appear
		 to require putting complicated stuff into the .oafinfo file, and even
		 more complicated stuff to use. */

	      if(strcasecmp(curnode->name, "oaf_server"))
		continue;

	      new_ent = oaf_alloca(sizeof(OAF_ServerInfo));
	      memset(new_ent, 0, sizeof(OAF_ServerInfo));

	      ctmp = xmlGetProp(curnode, "iid");
	      new_ent->iid = CORBA_string_dup(ctmp);
	      free(ctmp);

	      ctmp = xmlGetProp(curnode, "type");
	      new_ent->server_type = CORBA_string_dup(ctmp);
	      free(ctmp);

	      ctmp = xmlGetProp(curnode, "location");
	      new_ent->location_info = CORBA_string_dup(ctmp);
	      new_ent->hostname = CORBA_string_dup(host);
	      new_ent->domain = CORBA_string_dup(domain);
	      new_ent->username = CORBA_string_dup(g_get_user_name());
	      free(ctmp);

	      od_entry_read_attrs(new_ent, curnode);

	      my_slist_prepend(entries, new_ent);

	    }
    
	  xmlFreeDoc(doc);
	}
      closedir(dirh);
    }
  g_strfreev(dirs);

  /* Now convert 'entries' into something that the server can store and pass back */
  n = g_slist_length(entries);
  *nservers = n;

  retval = CORBA_sequence_OAF_ServerInfo_allocbuf(n);

  g_hash_table_freeze(*by_iid);
  for(i = 0, cur = entries; i < n; i++, cur = cur->next)
    {
      memcpy(&retval[i], cur->data, sizeof(OAF_ServerInfo));
      g_hash_table_insert(*by_iid, retval[i].iid, &retval[i]);
    }
  g_hash_table_thaw(*by_iid);

  return retval;
}
