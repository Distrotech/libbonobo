#ifndef _GNOME_PERSIST_H_
#define _GNOME_PERSIST_H_

#include <bonobo/gnome-object.h>

BEGIN_GNOME_DECLS

typedef struct {
	GnomeObject object;
} GnomePersist;

typedef struct {
	GnomeObjectClass parent_class;
} GnomePersistClass;

END_GNOME_DECLS

#endif /* _GNOME_PERSIST_H_ */
