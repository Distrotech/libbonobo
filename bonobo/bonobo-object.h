#ifndef _GNOME_UNKNOWN_H_
#define _GNOME_UNKNOWN_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>

BEGIN_GNOME_DECLS
 
#define GNOME_UNKNOWN_TYPE        (gnome_unknown_get_type ())
#define GNOME_UNKNOWN(o)          (GTK_CHECK_CAST ((o), GNOME_UNKNOWN_TYPE, GnomeUnknown))
#define GNOME_UNKNOWN_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_UNKNOWN_TYPE, GnomeUnknownClass))
#define GNOME_IS_UNKNOWN(o)       (GTK_CHECK_TYPE ((o), GNOME_UNKNOWN_TYPE))
#define GNOME_IS_UNKNOWN_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_UNKNOWN_TYPE))

/*
 * If you're using a custom servant for your CORBA objects, just make
 * sure that the second element is a 'gpointer' to hold the GnomeUnknown
 * pointer for servant->GnomeUnknown translation
 */
typedef struct {
	POA_GNOME_Unknown servant_placeholder;
	gpointer gnome_unknown;
} GnomeUnknownServant;

typedef struct {
	GtkObject base;

	CORBA_Object object;
	CORBA_Environment ev;
	gpointer servant;
} GnomeUnknown;

typedef struct {
	GtkObjectClass parent_class;
	void  (*test)(GnomeUnknown *object);
} GnomeUnknownClass;

GtkType      gnome_unknown_get_type         (void);
GnomeUnknown *gnome_unknown_construct        (GnomeUnknown *object,
					    CORBA_Object corba_object);

GnomeUnknown *gnome_unknown_from_servant     (PortableServer_Servant servant);
void         gnome_unknown_bind_to_servant  (GnomeUnknown *object,
					    void *servant);

CORBA_Object gnome_unknown_activate_servant (GnomeUnknown *object,
					    void *servant);

/* CORBA default vector methods we provide */
extern PortableServer_ServantBase__epv gnome_unknown_base_epv;
extern POA_GNOME_Unknown__epv           gnome_unknown_epv;
extern POA_GNOME_Unknown__vepv          gnome_unknown_vepv;

END_GNOME_DECLS

#endif
