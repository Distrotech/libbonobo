#ifndef _GNOME_PERSIST_STREAM_H_
#define _GNOME_PERSIST_STREAM_H_

#include <bonobo/gnome-persist.h>

BEGIN_GNOME_DECLS

typedef struct {
	GnomePersist persist;
} GnomePersistStream;

typedef struct {
	GnomePersistClass parent_class;
} GnomePersistStreamClass;

END_GNOME_DECLS

#endif /* _GNOME_PERSIST_STREAM_H_ */
