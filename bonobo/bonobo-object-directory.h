/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 c-set-style: linux -*- */
#ifndef __BONOBO_OBJECT_DIRECTORY_H__
#define __BONOBO_OBJECT_DIRECTORY_H__

#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <orb/orbit.h>
#include <ORBitservices/CosNaming.h>

BEGIN_GNOME_DECLS

/* This file provides an abstraction of the object directory Bonobo
   will use (GOAD or OAF). However, the object directory is not
   dynamically loaded or anything like that; it must be selected
   at compile time. This is a private, non-installed API.
*/

typedef struct _ODServerInfo ODServerInfo;

typedef enum {
        OD_REG_SUCCESS,
        OD_REG_NOT_LISTED,
        OD_REG_ALREADY_ACTIVE,
        OD_REG_ERROR
} ODRegistrationResult;

CORBA_ORB            od_get_orb                     (void);

ODServerInfo        *od_server_info_new             (const gchar       *iid,
                                                     const gchar       *desc);
const gchar         *od_server_info_get_id          (ODServerInfo      *info);
const gchar         *od_server_info_get_description (ODServerInfo      *info);
void                 od_server_info_ref             (ODServerInfo      *info);
void                 od_server_info_unref           (ODServerInfo      *info);



/* returns list of ODServerInfo */
GList               *od_get_server_list             (const gchar      **required_ids);
void                 od_server_list_free            (GList             *list);
CORBA_Object         od_server_activate_with_id     (const gchar       *iid,
						     gint               flags,
                                                     CORBA_Environment *ev);
ODRegistrationResult od_server_register             (CORBA_Object       objref,
                                                     const gchar       *iid);
ODRegistrationResult od_server_unregister           (CORBA_Object       objref,
						     const gchar       *iid);
void                 od_assert_using_goad           (void);
void                 od_assert_using_oaf            (void);

CORBA_Object         od_name_service_get            (CORBA_Environment *ev);

END_GNOME_DECLS

#endif /* __BONOBO_OBJECT_DIRECTORY_H__ */


