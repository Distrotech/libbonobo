/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_OBJECT_H_
#define _BONOBO_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>
#include <bonobo/Bonobo.h>

BEGIN_GNOME_DECLS
 
#define BONOBO_OBJECT_TYPE        (bonobo_object_get_type ())
#define BONOBO_OBJECT(o)          (GTK_CHECK_CAST ((o), BONOBO_OBJECT_TYPE, BonoboObject))
#define BONOBO_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_OBJECT_TYPE, BonoboObjectClass))
#define BONOBO_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), BONOBO_OBJECT_TYPE))
#define BONOBO_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_OBJECT_TYPE))

/*
 * If you're using a custom servant for your CORBA objects, just make
 * sure that the second element is a 'gpointer' to hold the BonoboObject
 * pointer for servant->BonoboObject translation
 */
typedef struct {
	POA_Bonobo_Unknown servant_placeholder;
	gpointer bonobo_object;
} BonoboObjectServant;

typedef struct _BonoboObjectPrivate BonoboObjectPrivate;

typedef struct {
	GtkObject base;

	CORBA_Object          corba_objref;
	gpointer              servant;
	BonoboObjectPrivate   *priv;
} BonoboObject;

typedef struct {
	GtkObjectClass parent_class;

	/*
	 * signals.  
	 */
	void  (*query_interface) (BonoboObject *object, const char *repo_id, CORBA_Object *retval);
	void  (*system_exception)(BonoboObject *object, CORBA_Object cobject, CORBA_Environment *ev);
	void  (*object_gone)     (BonoboObject *object, CORBA_Object cobject, CORBA_Environment *ev);
} BonoboObjectClass;

GtkType      bonobo_object_get_type         (void);
BonoboObject *bonobo_object_construct        (BonoboObject *object,
					    CORBA_Object corba_object);
BonoboObject *bonobo_object_new_from_servant (void *servant);

BonoboObject *bonobo_object_from_servant     (PortableServer_Servant servant);
void         bonobo_object_bind_to_servant  (BonoboObject *object,
					    void *servant);
PortableServer_Servant
             bonobo_object_get_servant      (BonoboObject *object);

CORBA_Object bonobo_object_activate_servant (BonoboObject *object,
					    void *servant);

void         bonobo_object_add_interface    (BonoboObject *object,
					    BonoboObject *newobj);
CORBA_Object bonobo_object_query_interface  (BonoboObject *object,
					    const char *repo_id);
CORBA_Object bonobo_object_corba_objref     (BonoboObject *object);

/*
 * Gnome Object Life Cycle
 */
void         bonobo_object_ref              (BonoboObject *object);
void         bonobo_object_unref            (BonoboObject *object);
void         bonobo_object_destroy          (BonoboObject *object);

POA_Bonobo_Unknown__epv *bonobo_object_get_epv (void);


/*
 * Error checking
 */
void         bonobo_object_check_env        (BonoboObject *object,
					    CORBA_Object corba_object,
					    CORBA_Environment *ev);

#define BONOBO_OBJECT_CHECK(o,c,e) do { \
                                        if ((e)->_major != CORBA_NO_EXCEPTION) { \
                                        	bonobo_object_check_env(o,c,e);     \
			        } while (0)

/*
 * Others
 */
gboolean  gnome_unknown_ping (Bonobo_Unknown object);

/* CORBA default vector methods we provide */
extern POA_Bonobo_Unknown__vepv          bonobo_object_vepv;

END_GNOME_DECLS

#endif

