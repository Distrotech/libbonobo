/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_PERSIST_H_
#define _GNOME_PERSIST_H_

#include <bonobo/gnome-object.h>

BEGIN_GNOME_DECLS

#define GNOME_PERSIST_TYPE        (gnome_persist_get_type ())
#define GNOME_PERSIST(o)          (GTK_CHECK_CAST ((o), GNOME_PERSIST_TYPE, GnomePersist))
#define GNOME_PERSIST_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_PERSIST_TYPE, GnomePersistClass))
#define GNOME_IS_PERSIST(o)       (GTK_CHECK_TYPE ((o), GNOME_PERSIST_TYPE))
#define GNOME_IS_PERSIST_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_PERSIST_TYPE))

typedef struct _GnomePersist GnomePersistPrivate;

typedef struct {
	GnomeObject object;

	char  *goad_id;

	GnomePersistPrivate *priv;
} GnomePersist;

typedef struct {
	GnomeObjectClass parent_class;
} GnomePersistClass;

GtkType       gnome_persist_get_type  (void);
GnomePersist *gnome_persist_construct (GnomePersist *persist,
				       GNOME_Persist corba_persist,
				       const char *goad_id);

POA_GNOME_Persist__epv *gnome_persist_get_epv (void);

extern POA_GNOME_Persist__vepv gnome_persist_vepv;
END_GNOME_DECLS

#endif /* _GNOME_PERSIST_H_ */
