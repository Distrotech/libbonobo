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

#endif
