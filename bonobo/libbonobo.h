/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * Main include file for the Bonobo component model
 *
 * FIXME: We should try to optimize the order of the include
 * files here to minimize repeated inclussion of files, and split
 * this into X vs. non X headers.
 *
 * Copyright 1999 Helix Code, Inc.
 */

#ifndef LIBBONOBO_H
#define LIBBONOBO_H 1

#include <libgnomebase/gnome-defs.h>

BEGIN_GNOME_DECLS

#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-simple.h>
#include <bonobo/bonobo-context.h>
#include <bonobo/bonobo-exception.h>

#include <bonobo/bonobo-object-client.h>
#include <bonobo/bonobo-moniker-util.h>

#include <bonobo/bonobo-property-bag.h>
#include <bonobo/bonobo-property-bag-client.h>

#include <bonobo/bonobo-listener.h>
#include <bonobo/bonobo-event-source.h>

#include <bonobo/bonobo-generic-factory.h>

#include <bonobo/bonobo-main.h>

#include <bonobo/bonobo-stream.h>
#include <bonobo/bonobo-stream-client.h>

#include <bonobo/bonobo-persist.h>
#include <bonobo/bonobo-persist-file.h>
#include <bonobo/bonobo-persist-stream.h>

#include <bonobo/bonobo-object-io.h>

#include <bonobo/bonobo-progressive.h>

#include <bonobo/bonobo-storage.h>

END_GNOME_DECLS

#endif /* BONOBO_H */
