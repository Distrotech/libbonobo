/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#ifndef LIBOAF_PRIVATE_H
#define LIBOAF_PRIVATE_H 1

#include "config.h"

#include "liboaf.h"

#ifdef g_alloca
#define oaf_alloca g_alloca
#else
#define oaf_alloca alloca
#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif
#endif

CORBA_Object oaf_server_by_forking (const char **cmd, int fd_Arg,
				    CORBA_Environment * ev);
extern void oaf_rloc_file_register (void);
int oaf_ior_fd_get (void);
CORBA_Object oaf_activation_context_get (void);

#endif

