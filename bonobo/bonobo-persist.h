/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _BONOBO_PERSIST_H_
#define _BONOBO_PERSIST_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define BONOBO_PERSIST_TYPE        (bonobo_persist_get_type ())
#define BONOBO_PERSIST(o)          (GTK_CHECK_CAST ((o), BONOBO_PERSIST_TYPE, BonoboPersist))
#define BONOBO_PERSIST_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), BONOBO_PERSIST_TYPE, BonoboPersistClass))
#define BONOBO_IS_PERSIST(o)       (GTK_CHECK_TYPE ((o), BONOBO_PERSIST_TYPE))
#define BONOBO_IS_PERSIST_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), BONOBO_PERSIST_TYPE))

typedef struct _BonoboPersist BonoboPersistPrivate;

typedef struct {
	BonoboObject object;

	BonoboPersistPrivate *priv;
} BonoboPersist;

typedef struct {
	BonoboObjectClass parent_class;
} BonoboPersistClass;

GtkType       bonobo_persist_get_type  (void);
BonoboPersist *bonobo_persist_construct (BonoboPersist *persist,
				       Bonobo_Persist corba_persist);

POA_Bonobo_Persist__epv *bonobo_persist_get_epv (void);

extern POA_Bonobo_Persist__vepv bonobo_persist_vepv;
END_GNOME_DECLS

#endif /* _BONOBO_PERSIST_H_ */
