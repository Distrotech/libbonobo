/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_OBJECT_H_
#define _GNOME_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/bonobo.h>

BEGIN_GNOME_DECLS
 
#define GNOME_OBJECT_TYPE        (gnome_object_get_type ())
#define GNOME_OBJECT(o)          (GTK_CHECK_CAST ((o), GNOME_OBJECT_TYPE, GnomeObject))
#define GNOME_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_OBJECT_TYPE, GnomeObjectClass))
#define GNOME_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), GNOME_OBJECT_TYPE))
#define GNOME_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_OBJECT_TYPE))

/*
 * If you're using a custom servant for your CORBA objects, just make
 * sure that the second element is a 'gpointer' to hold the GnomeObject
 * pointer for servant->GnomeObject translation
 */
typedef struct {
	POA_GNOME_Unknown servant_placeholder;
	gpointer gnome_object;
} GnomeObjectServant;

typedef struct _GnomeObjectPrivate GnomeObjectPrivate;

typedef struct {
	GtkObject base;

	CORBA_Object          corba_objref;
	gpointer              servant;
	GnomeObjectPrivate   *priv;
} GnomeObject;

typedef struct {
	GtkObjectClass parent_class;

	/*
	 * signals.  
	 */
	void  (*query_interface) (GnomeObject *object, const char *repo_id, CORBA_Object *retval);
	void  (*system_exception)(GnomeObject *object, CORBA_Object cobject, CORBA_Environment *ev);
	void  (*object_gone)     (GnomeObject *object, CORBA_Object cobject, CORBA_Environment *ev);
} GnomeObjectClass;

GtkType      gnome_object_get_type         (void);
GnomeObject *gnome_object_construct        (GnomeObject *object,
					    CORBA_Object corba_object);
GnomeObject *gnome_object_new_from_servant (void *servant);

GnomeObject *gnome_object_from_servant     (PortableServer_Servant servant);
void         gnome_object_bind_to_servant  (GnomeObject *object,
					    void *servant);
PortableServer_Servant
             gnome_object_get_servant      (GnomeObject *object);

CORBA_Object gnome_object_activate_servant (GnomeObject *object,
					    void *servant);

void         gnome_object_add_interface    (GnomeObject *object,
					    GnomeObject *newobj);
CORBA_Object gnome_object_query_interface  (GnomeObject *object,
					    const char *repo_id);
CORBA_Object gnome_object_corba_objref     (GnomeObject *object);

/*
 * Gnome Object Life Cycle
 */
void         gnome_object_ref              (GnomeObject *object);
void         gnome_object_unref            (GnomeObject *object);
void         gnome_object_destroy          (GnomeObject *object);

POA_GNOME_Unknown__epv *gnome_object_get_epv (void);


/*
 * Error checking
 */
void         gnome_object_check_env        (GnomeObject *object,
					    CORBA_Object corba_object,
					    CORBA_Environment *ev);

#define GNOME_OBJECT_CHECK(o,c,e) do { \
                                        if ((e)->_major != CORBA_NO_EXCEPTION) { \
                                        	gnome_object_check_env(o,c,e);     \
			        } while (0)

/*
 * Others
 */
gboolean  gnome_unknown_ping (GNOME_Unknown object);

/* CORBA default vector methods we provide */
extern POA_GNOME_Unknown__vepv          gnome_object_vepv;

END_GNOME_DECLS

#endif

