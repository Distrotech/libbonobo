/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 c-set-style: linux -*- */

#include "bonobo-object-directory.h"

struct _ODServerInfo {
        guint refcount;
        gchar* iid;
        gchar* desc;
};

ODServerInfo*
od_server_info_new(const gchar* iid, const gchar* desc)
{
        ODServerInfo *info;

        info = g_new(ODServerInfo, 1);

        info->refcount = 1;
        info->iid = iid ? g_strdup(iid) : NULL;
        info->desc = desc ? g_strdup(desc) : NULL;

        return info;
}

const gchar*
od_server_info_get_id          (ODServerInfo      *info)
{
        return info->iid;
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
                g_free(info->iid);
                g_free(info->desc);
                g_free(info);
        }
}

void
od_server_list_free            (GList             *list)
{
        GList *iter;

        iter = list;

        while (iter != NULL) {
                od_server_info_unref(iter->data);
                
                iter = g_list_next(iter);
        }

        g_list_free(list);
}
