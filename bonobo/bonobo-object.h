#ifndef _GNOME_OBJECT_H_
#define _GNOME_OBJECT_H_

#include <libgnome/gnome-defs.h>
#include <gtk/gtkobject.h>

BEGIN_GNOME_DECLS

#define GNOME_OBJECT_TYPE        (gnome_object_get_type ())
#define GNOME_OBJECT_ITEM(o)     (GTK_CHECK_CAST ((o), GNOME_OBJECT_TYPE, GnomeObject))
#define GNOME_OBJECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_OBJECT_TYPE, GnomeObjectClass))
#define GNOME_IS_OBJECT(o)       (GTK_CHECK_TYPE ((o), GNOME_OBJECT_TYPE))
#define GNOME_IS_OBJECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_OBJECT_TYPE))

typedef enum {
	GNOME_OBJECT_SAVE_OK,
	GNOME_OBJECT_SAVE_CANCELLED
} GnomeObjectSaveResult;

/*
 * Signature for a GnomeObjectViewFactory
 */
typedef GtkWidget * (*GnomeObjectViewFactory)(GnomeObject *object, gpointer data);

/*
 * Signature for a GnomeObjectSaveInStorage
 */
typedef GnomeObjectSaveResult
      (*GnomeObjectSaveInStorage)(GnomeObject *object, GNOME_Storage *storage, gpointer data);

/*
 * Signature for a GnomeObjectSaveInFile
 */
typedef GnomeObjectSaveResult
      (*GnomeObjectSaveInFile)(GnomeObject *object, int file_handle, gpointer data);

/* CORBA servants */
typedef struct {
	POA_GNOME_Component servant;
	PortableServer_POA  poa;
	GtkObject          *gnome_object;
} GNOME_Component_servant;

typedef struct {
	GtkObject object;

	GnomeObjectViewFactory view_factory;

	/*
	 * Save routines
	 */
	GnomeObjectSaveInStorage storage_save;
	GnomeObjectSaveInFile    file_save;
	
	GNOME_Component_servant  *component;
	
	char *window_title;
	char *container_app_name;
	
	/* A GList containing VerbInfo structures */
	GList *VerbInfo_list;
	
	/* CORBA fields */
	GNOME_Component  component;
	GNOME_ClientSite client_site;
	GNOME_AdviseSink advise_sink;

	CORBA_Environment ev;
} GnomeObject;

typedef struct {
	GtkObjectClass parent_class;

	void (*do_verb)       (GnomeObject *o, int verb, char *verb_name);
	void (*set_host_name) (GnomeObject *o, char *name, char *appname);

} GnomeObjectClass;

GtkType      gnome_object_get_type         (void);
GnomeObject *gnome_object_new              (void);

void         gnome_object_set_verb_list    (GnomeObject *object,
					    GList *VerbInfo_list);
void         gnome_object_advise_notify    (GnomeObject *object,
					    int status);
void         gnome_object_set_view_factory (GnomeObject *object,
					    GnomeObjectViewFactory factory);

void gnome_object_set_save_in_storage (GnomeObject *object,
				       GnomeObjectSaveInStorage *storage_save);
void gnome_object_set_save_in_file    (GnomeObject *object,
				       GnomeObjectSaveInFile *file_save);

#endif
