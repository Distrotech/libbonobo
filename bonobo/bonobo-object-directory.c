/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 c-set-style: linux -*- */
/**
 * bonobo-object-directory.c: abstract the object directory
 *
 * Authors:
 *    Havoc Pennington  (hp@redhat.com)
 *    Anders Carlsson   (andersca@gnu.org)
 *    Maciej Stachowiak (mjs@eazel.com)
 *
 * Copyright 1999, 2000 Havoc Pennington, Anders Carlsson,
 *                      Maciej Stachowiak
 */

#include "bonobo-object-directory.h"
#include <liboaf/liboaf.h>

struct _ODServerInfo {
        guint refcount;
        gchar* iid;
	gchar* name;
        gchar* desc;
};

ODServerInfo*
od_server_info_new (const gchar* iid, const gchar* name, const gchar* desc)
{
        ODServerInfo *info;

        info = g_new(ODServerInfo, 1);

        info->refcount = 1;
        info->iid = iid ? g_strdup (iid) : NULL;
	info->name = name ? g_strdup (name) : NULL;
        info->desc = desc ? g_strdup (desc) : NULL;

        return info;
}

const gchar*
od_server_info_get_id          (ODServerInfo      *info)
{
        return info->iid;
}

const gchar*
od_server_info_get_name (ODServerInfo     *info)
{
	return info->name;
}
			 
const gchar*
od_server_info_get_description (ODServerInfo      *info)
{
        return info->desc;
}

void
od_server_info_ref             (ODServerInfo      *info)
{
        info->refcount += 1;
}

void
od_server_info_unref           (ODServerInfo      *info)
{
        g_return_if_fail(info != NULL);
        g_return_if_fail(info->refcount > 0);

        info->refcount -= 1;

        if (info->refcount == 0) {
                g_free (info->iid);
		g_free (info->name);
                g_free (info->desc);
                g_free (info);
        }
}

void
od_server_list_free            (GList             *list)
{
        GList *iter;

        iter = list;

        while (iter != NULL) {
                od_server_info_unref (iter->data);
                
                iter = g_list_next (iter);
        }

        g_list_free (list);
}


CORBA_ORB
od_get_orb (void)
{
	return oaf_orb_get();
}

GList*
od_get_server_list (const gchar **required_ids)
{
        GList *retval = NULL;
        gchar *query;
        const gchar** required_ids_iter;
        const gchar** query_components_iter;
        guint n_required = 0;
        gchar **query_components;
        OAF_ServerInfoList *servers;
        CORBA_Environment ev;
        guint i, j;
        
        g_return_val_if_fail (required_ids != NULL, NULL);
        g_return_val_if_fail (*required_ids != NULL, NULL);

        /* We need to build a query up from the required_ids */
        required_ids_iter = required_ids;

        while (*required_ids_iter) {
                ++n_required;
                ++required_ids_iter;
        }

        query_components = g_new0 (gchar*, n_required + 1);

        query_components_iter = (const gchar **) query_components;
        required_ids_iter = required_ids;

        while (*required_ids_iter) {
                *query_components_iter = g_strconcat ("repo_ids.has('",
                                                      *required_ids_iter,
                                                      "')",
                                                      NULL);
                ++query_components_iter;
                ++required_ids_iter;
        }

        query = g_strjoinv (" AND ", query_components);

        g_strfreev (query_components);

        CORBA_exception_init (&ev);
        servers = oaf_query (query, NULL, &ev);
        g_free (query);
        CORBA_exception_free (&ev);

        if (servers == NULL)
                return NULL;

	for (i = 0; i < servers->_length; i++) {
                OAF_ServerInfo *oafinfo = &servers->_buffer[i];
                ODServerInfo *info;
		gchar *name = NULL, *desc = NULL;


		for (j = 0; j < oafinfo->props._length; j++) {
			if (oafinfo->props._buffer[j].v._d != OAF_P_STRING)
				continue;

			if (strcmp (oafinfo->props._buffer[j].name, "name") == 0) {
				name = oafinfo->props._buffer[j].v._u.value_string;
			}

			else if (strcmp (oafinfo->props._buffer[j].name, "description") == 0)
				desc = oafinfo->props._buffer[j].v._u.value_string;
		}

		/*
		 * If no name attribute exists, use the description attribute.
		 *  If no description attribute exists, use the name attribute.
		 *  If neither a description attribute nor a name attribute exists, use the oafiid
		 */
		if (!name && !desc)
			name = desc = oafinfo->iid;

		if (!name)
			name = desc;

		if (!desc)
			desc = name;
		
                info = od_server_info_new (oafinfo->iid,
					   name,
					   desc);


                retval = g_list_prepend (retval, info);

        }

        CORBA_free (servers);
        
        return g_list_reverse (retval);
}

CORBA_Object
od_server_activate_with_id     (const gchar       *iid,
				gint               flags,
                                CORBA_Environment *ev)
{
	CORBA_Environment myev;
	CORBA_Object retval;
	
	CORBA_exception_init(&myev);
	
	if (ev == NULL) {
		ev = &myev;
	}
		
	retval = oaf_activate_from_id ((gchar *)iid, 0, NULL, ev);

	CORBA_exception_free(ev);
	
	return retval;
}

ODRegistrationResult
od_server_register             (CORBA_Object       objref,
                                const gchar       *iid)
{
        OAF_RegistrationResult result;


        result = oaf_active_server_register(iid, objref);

        switch (result) {
        case OAF_REG_SUCCESS:
                return OD_REG_SUCCESS;
                break;
                
        case OAF_REG_NOT_LISTED:
                return OD_REG_NOT_LISTED;
                break;

        case OAF_REG_ALREADY_ACTIVE:
                return OD_REG_ALREADY_ACTIVE;
                break;

        case OAF_REG_ERROR:
        default:
                return OD_REG_ERROR;
                break;
        }
}

ODRegistrationResult
od_server_unregister           (CORBA_Object       objref,
                                const gchar       *iid)
{
        oaf_active_server_unregister(iid, objref);
        
        return OD_REG_SUCCESS;
}

CORBA_Object
od_name_service_get (CORBA_Environment *ev)
{
	return oaf_name_service_get (ev);
}
