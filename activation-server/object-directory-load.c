/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oafd: OAF CORBA dameon.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 1999, 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Authors: Elliot Lee <sopwith@redhat.com>,
 *
 */

#include "oafd.h"
#include "oaf-i18n.h"
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <parser.h>      /* gnome-xml */
#include <xmlmemory.h>   /* gnome-xml */

#define my_slist_prepend(slist, datum) \
new_item = oaf_alloca(sizeof(GSList)); \
new_item->next = slist; \
new_item->data = datum; \
slist = new_item;

static gboolean
od_string_to_boolean (const char *str)
{
	if (!strcasecmp (str, "true")
	    || !strcasecmp (str, "yes")
	    || !strcmp (str, "1"))
		return TRUE;
	else
		return FALSE;
}

static void
od_entry_read_attrs (OAF_ServerInfo * ent, xmlNodePtr node)
{
	int i, n;
	xmlNodePtr sub;
	OAF_Attribute *curattr;

	for (n = 0, sub = node->childs; sub; sub = sub->next) {
		if (sub->type != XML_ELEMENT_NODE)
			continue;

		if (strcasecmp (sub->name, "oaf_attribute"))
			continue;

		n++;
	}

	ent->attrs._length = n;
	ent->attrs._buffer = curattr = g_new (OAF_Attribute, n);

	for (i = 0, sub = node->childs; i < n; sub = sub->next, i++) {
		char *type, *valuestr;

		type = xmlGetProp (sub, "type");
		if (!type)
			continue;

		valuestr = xmlGetProp (sub, "name");
		if (!valuestr) {
			free (type);
			continue;
		}
		curattr->name = CORBA_string_dup (valuestr);
		free (valuestr);

		if (!strcasecmp (type, "stringv")) {
			int j, o;
			xmlNodePtr sub2;

			curattr->v._d = OAF_A_STRINGV;

			for (o = 0, sub2 = sub->childs; sub2;
			     sub2 = sub2->next) {
				if (sub2->type != XML_ELEMENT_NODE)
					continue;
				if (strcasecmp (sub2->name, "item"))
					continue;

				o++;
			}

			curattr->v._u.value_stringv._length = o;
			curattr->v._u.value_stringv._buffer =
				CORBA_sequence_CORBA_string_allocbuf (o);

			for (j = 0, sub2 = sub->childs; j < o;
			     sub2 = sub2->next, j++) {
				valuestr = xmlGetProp (sub2, "value");
				if (valuestr)
					curattr->v._u.
						value_stringv._buffer[j] =
						CORBA_string_dup (valuestr);
				else {
					g_warning
						(N_("Attribute '%s' has no value"),
						 curattr->name);
					curattr->v._u.
						value_stringv._buffer[j] =
						CORBA_string_dup ("");
				}
				free (valuestr);
			}

		} else if (!strcasecmp (type, "number")) {
			valuestr = xmlGetProp (sub, "value");

			curattr->v._d = OAF_A_NUMBER;
			curattr->v._u.value_number = atof (valuestr);

			free (valuestr);
		} else if (!strcasecmp (type, "boolean")) {
			valuestr = xmlGetProp (sub, "value");
			curattr->v._d = OAF_A_BOOLEAN;
			curattr->v._u.value_boolean =
				od_string_to_boolean (valuestr);
			free (valuestr);
		} else {
			valuestr = xmlGetProp (sub, "value");
			/* Assume string */
			curattr->v._d = OAF_A_STRING;
			if (valuestr)
				curattr->v._u.value_string =
					CORBA_string_dup (valuestr);
			else {
				g_warning (N_("Attribute '%s' has no value"),
					   curattr->name);
				curattr->v._u.value_string =
					CORBA_string_dup ("");
			}
			free (valuestr);
		}

		free (type);

		curattr++;
	}
}

static gboolean
od_validate_iid (const char *iid)
{
        int i;

        for (i = 0; iid && iid [i]; i++) {
                char c = iid [i];

                if (c == ',' || c == '[' || c == ']' ||
                    /* Reserved for future expansion */
                    c == '!' || c == '#' || c == '|')
                        return FALSE;

        }

        return TRUE;
}

OAF_ServerInfo *
OAF_ServerInfo_load (char **dirs,
		     CORBA_unsigned_long *nservers,
		     GHashTable ** by_iid,
		     const char *user, const char *host, const char *domain)
{
	DIR *dirh;
	struct dirent *dent;
	char tmpstr[PATH_MAX];
	GSList *entries = NULL, *cur, *new_item;
	int i, n;
	OAF_ServerInfo *retval;
	int dirnum;

	g_return_val_if_fail (dirs, NULL);
	g_return_val_if_fail (nservers, NULL);
	g_return_val_if_fail (by_iid, NULL);

	if (*by_iid)
		g_hash_table_destroy (*by_iid);
	*by_iid = g_hash_table_new (g_str_hash, g_str_equal);


	*nservers = 0;

	for (dirnum = 0; dirs[dirnum]; dirnum++) {
		g_print (N_("Trying dir %s\n"), dirs[dirnum]);
		dirh = opendir (dirs[dirnum]);
		if (!dirh)
			continue;

		while ((dent = readdir (dirh))) {
			char *ext;
			xmlDocPtr doc;
			xmlNodePtr curnode;

			ext = strrchr (dent->d_name, '.');
			if (!ext || strcasecmp (ext, ".oafinfo"))
				continue;

			g_snprintf (tmpstr, sizeof (tmpstr), "%s/%s",
				    dirs[dirnum], dent->d_name);

			doc = xmlParseFile (tmpstr);
			if (!doc)
				continue;

			/* This should go in a separate function, but I'm sticking it in
			 * here so alloca can be used. "Eeeek!" is still fine as a
			 * response, :) but this has a direct impact on startup time. */

			for (curnode =
			     (!strcasecmp (doc->root->name, "oaf_info")
			      ? doc->root->childs : doc->root);
			     NULL != curnode; curnode = curnode->next) {
				OAF_ServerInfo *new_ent;
				char *ctmp, *iid;
                                gboolean already_there;

				if (curnode->type != XML_ELEMENT_NODE)
					continue;

				/* I'd love to use XML namespaces, but unfortunately they appear
				 * to require putting complicated stuff into the .oafinfo file, 
                                 * and even more complicated stuff to use. 
                                 */

				if (strcasecmp (curnode->name, "oaf_server"))
					continue;

				iid = xmlGetProp (curnode, "iid");

                                if (!od_validate_iid (iid)) {
                                        g_print (N_("IID '%s' contains illegal characters; discarding\n"), 
                                                 iid);
                                        free (iid);
                                        continue;
                                }

                                /* make sure the component has not been already read. If so,
                                   do not add this entry to the entries list */
                                already_there = FALSE;
                                for (cur = entries; cur != NULL; cur = cur->next) {
                                        if (strcmp (((OAF_ServerInfo *)cur->data)->iid, iid) == 0) {
                                                already_there = TRUE;
                                                break;
                                        }
                                }
                                
                                if (already_there == FALSE) {
                                        new_ent = oaf_alloca (sizeof (OAF_ServerInfo));
                                        memset (new_ent, 0, sizeof (OAF_ServerInfo));

                                        new_ent->iid = CORBA_string_dup (iid);
                                        xmlFree (iid);

                                        ctmp = xmlGetProp (curnode, "type");
                                        new_ent->server_type =
                                                CORBA_string_dup (ctmp);
                                        free (ctmp);

                                        ctmp = xmlGetProp (curnode, "location");
                                        new_ent->location_info =
                                                CORBA_string_dup (ctmp);
                                        new_ent->hostname = CORBA_string_dup (host);
                                        new_ent->domain = CORBA_string_dup (domain);
                                        new_ent->username =
                                                CORBA_string_dup (g_get_user_name ());
                                        free (ctmp);

                                        od_entry_read_attrs (new_ent, curnode);
                                        
                                        my_slist_prepend (entries, new_ent);
                                } else {
                                        xmlFree (iid);
                                }

			}

			xmlFreeDoc (doc);
		}
		closedir (dirh);
	}

	/* Now convert 'entries' into something that the server can store and pass back */
	n = g_slist_length (entries);
	*nservers = n;

	retval = CORBA_sequence_OAF_ServerInfo_allocbuf (n);

	g_hash_table_freeze (*by_iid);
	for (i = 0, cur = entries; i < n; i++, cur = cur->next) {
		memcpy (&retval[i], cur->data, sizeof (OAF_ServerInfo));
		g_hash_table_insert (*by_iid, retval[i].iid, &retval[i]);
	}
	g_hash_table_thaw (*by_iid);

	return retval;
}
