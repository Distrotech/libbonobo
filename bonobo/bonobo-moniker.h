/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_MONIKER_H_
#define _GNOME_MONIKER_H_

#include <bonobo/gnome-bind-context.h>
#include <bonobo/gnome-persist-stream.h>
#include <libgnorba/gnorba.h>

BEGIN_GNOME_DECLS

#define GNOME_MONIKER_TYPE        (gnome_moniker_get_type ())
#define GNOME_MONIKER(o)          (GTK_CHECK_CAST ((o), GNOME_MONIKER_TYPE, GnomeMoniker))
#define GNOME_MONIKER_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_MONIKER_TYPE, GnomeMonikerClass))
#define GNOME_IS_MONIKER(o)       (GTK_CHECK_TYPE ((o), GNOME_MONIKER_TYPE))
#define GNOME_IS_MONIKER_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_MONIKER_TYPE))

struct _GnomeMoniker;
typedef struct _GnomeMoniker GnomeMoniker;

typedef CORBA_Object (*GnomeMonikerBindFn)(GnomeMoniker *moniker,
					   GNOME_BindOptions *bind_options,
					   void *closure);

struct _GnomeMoniker {
	GnomePersistStream persist_stream;
	GnomeMonikerBindFn bind_function;
	void *bind_function_closure;
};

typedef struct {
	GnomePersistStreamClass parent_class;
} GnomeMonikerClass;

GtkType           gnome_moniker_get_type   (void);
GnomeMoniker	 *gnome_moniker_new (GnomeMonikerBindFn bind_function, void *bind_function_closure);
GnomeMoniker     *gnome_moniker_construct  (GnomeMoniker *moniker,
					    GNOME_Moniker corba_moniker,
					    GnomeMonikerBindFn bind_function,
					    void *bind_function_closure);

GnomeBindContext *gnome_bind_context_new   (void);
GnomeMoniker     *gnome_parse_display_name (GnomeBindContext *bind,
					    const char *display_name,
					    int *chars_parsed);

CORBA_Object      find_moniker_in_naming_service (gchar *name, gchar *kind);


/* The CORBA vepv for the class */
extern POA_GNOME_Moniker__vepv gnome_moniker_vepv;

END_GNOME_DECLS

#endif
