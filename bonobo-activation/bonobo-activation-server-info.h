/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */

#ifndef BONOBO_ACTIVATION_SERVER_INFO_H
#define BONOBO_ACTIVATION_SERVER_INFO_H

#include <bonobo-activation/Bonobo_ActivationContext.h>

Bonobo_Property   *bonobo_server_info_prop_find        (Bonobo_ServerInfo                      *server,
                                                        const char                             *prop_name);
const char        *bonobo_server_info_prop_lookup      (Bonobo_ServerInfo                      *server,
                                                        const char                             *prop_name,
                                                        GSList                                 *i18n_languages);
void               Bonobo_PropertyValue_copy           (Bonobo_PropertyValue                   *copy,
                                                        const Bonobo_PropertyValue             *original);
void               Bonobo_Property_copy                (Bonobo_Property                        *copy,
                                                        const Bonobo_Property                  *original);
void               CORBA_sequence_Bonobo_Property_copy (CORBA_sequence_Bonobo_Property         *copy,
                                                        const CORBA_sequence_Bonobo_Property   *original);
void               Bonobo_ServerInfo_copy              (Bonobo_ServerInfo                      *copy, 
                                                        const Bonobo_ServerInfo                *original);
Bonobo_ServerInfo *Bonobo_ServerInfo_duplicate         (const Bonobo_ServerInfo                *original);

#endif /* BONOBO_ACTIVATION_SERVER_INFO_H */


