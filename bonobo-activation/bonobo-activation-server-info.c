#include "liboaf.h"

OAF_Attribute *
oaf_server_info_attr_find(OAF_ServerInfo *server, const char *attr_name)
{
	int i;

  for(i = 0; i < server->attrs._length; i++) {
    if(!strcmp(server->attrs._buffer[i].name, attr_name))
      return &server->attrs._buffer[i];
  }

  return NULL;
}

const char *
oaf_server_info_attr_lookup(OAF_ServerInfo *server, const char *attr_name, GSList *i18n_languages)
{
  int i;
  GSList *cur;

  if(i18n_languages) {
    char *retval = NULL;

    for(cur = i18n_languages; cur; cur = cur->next) {
      char *retval, cbuf[256];

      g_snprintf(cbuf, sizeof(cbuf), "%s-%s", attr_name, cur->data);
      retval = oaf_server_info_attr_lookup(server, cbuf, NULL);
      if(retval)
	return retval;
    }
  }

  {
    OAF_Attribute *attr;
    attr = oaf_server_info_attr_find(server, attr_name);
    if(attr->v._d == OAF_A_STRING)
      return attr->v._u.value_string;
  }

  return NULL;
}
