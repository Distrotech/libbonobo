/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "liboaf.h"

OAF_Attribute *
oaf_server_info_attr_find (OAF_ServerInfo * server, const char *attr_name)
{
	int i;

	for (i = 0; i < server->attrs._length; i++) {
		if (!strcmp (server->attrs._buffer[i].name, attr_name))
			return &server->attrs._buffer[i];
	}

	return NULL;
}

const char *
oaf_server_info_attr_lookup (OAF_ServerInfo * server, const char *attr_name,
			     GSList * i18n_languages)
{
	GSList *cur;
	OAF_Attribute *attr;
        const char *retval;
        char *attr_name_buf;
        char short_lang[3];
                     
	if (i18n_languages) {
		for (cur = i18n_languages; cur; cur = cur->next) {
                        attr_name_buf = g_strdup_printf ("%s-%s", attr_name, (char *) cur->data);

			retval = oaf_server_info_attr_lookup (server, attr_name_buf, NULL);
                        g_free (attr_name_buf);

                        if (!retval) {
                                if (strlen ((char *) cur->data) > 2) {
                                        strncpy (short_lang, (char *) cur->data, 2);
                                        attr_name_buf = g_strdup_printf ("%s-%s", attr_name, short_lang);
                                        retval = oaf_server_info_attr_lookup (server, attr_name_buf, NULL);
                                        g_free (attr_name_buf);
                                }
                        }

			if (retval)
				return retval;
		}
	} 

        attr = oaf_server_info_attr_find (server, attr_name);
        if (attr != NULL && attr->v._d == OAF_A_STRING)
                return attr->v._u.value_string;

	return NULL;
}

static void
CORBA_sequence_CORBA_string_copy (CORBA_sequence_CORBA_string *copy, const CORBA_sequence_CORBA_string *original)
{
	int i;

	copy->_maximum = original->_length;
	copy->_length = original->_length;
	copy->_buffer = CORBA_sequence_CORBA_string_allocbuf (original->_length);

	for (i = 0; i < original->_length; i++) {
		copy->_buffer[i] = CORBA_string_dup (original->_buffer[i]);
	}

	CORBA_sequence_set_release (copy, TRUE);
}

void
OAF_AttributeValue_copy (OAF_AttributeValue *copy, const OAF_AttributeValue *original)
{
	copy->_d = original->_d;
	switch (original->_d) {
	case OAF_A_STRING:
		copy->_u.value_string =	CORBA_string_dup (original->_u.value_string);
		break;
	case OAF_A_NUMBER:
		copy->_u.value_number =	original->_u.value_number;
		break;
	case OAF_A_BOOLEAN:
		copy->_u.value_boolean = original->_u.value_boolean;
		break;
	case OAF_A_STRINGV:
		CORBA_sequence_CORBA_string_copy
			(&copy->_u.value_stringv,
			 &original->_u.value_stringv);
		break;
	default:
		g_assert_not_reached ();
	}
}

void
OAF_Attribute_copy (OAF_Attribute *copy, const OAF_Attribute *original)
{
	copy->name = CORBA_string_dup (original->name);
	OAF_AttributeValue_copy (&copy->v, &original->v);
}

void
CORBA_sequence_OAF_Attribute_copy (CORBA_sequence_OAF_Attribute *copy, const CORBA_sequence_OAF_Attribute *original)
{
	int i;

	copy->_maximum = original->_length;
	copy->_length = original->_length;
	copy->_buffer = CORBA_sequence_OAF_Attribute_allocbuf (original->_length);

	for (i = 0; i < original->_length; i++) {
		OAF_Attribute_copy (&copy->_buffer[i], &original->_buffer[i]);
	}

	CORBA_sequence_set_release (copy, TRUE);
}

void
OAF_ServerInfo_copy (OAF_ServerInfo *copy, const OAF_ServerInfo *original)
{
	copy->iid = CORBA_string_dup (original->iid);
	copy->server_type = CORBA_string_dup (original->server_type);
	copy->location_info = CORBA_string_dup (original->location_info);
	copy->username = CORBA_string_dup (original->username);
	copy->hostname = CORBA_string_dup (original->hostname);
	copy->domain = CORBA_string_dup (original->domain);
	CORBA_sequence_OAF_Attribute_copy (&copy->attrs, &original->attrs);
}

OAF_ServerInfo *
OAF_ServerInfo_duplicate (const OAF_ServerInfo *original)
{
	OAF_ServerInfo *copy;

	copy = OAF_ServerInfo__alloc ();
	OAF_ServerInfo_copy (copy, original);
	
	return copy;
}
