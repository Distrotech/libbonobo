/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_STREAM_MEM_H_
#define _GNOME_STREAM_MEM_H_

#include <bonobo/gnome-stream.h>

BEGIN_GNOME_DECLS

struct _GnomeStreamMem;
typedef struct _GnomeStreamMem GnomeStreamMem;
typedef struct _GnomeStreamMemPrivate GnomeStreamMemPrivate;

#ifndef _GNOME_STORAGE_MEM_H_
struct _GnomeStorageMem;
typedef struct _GnomeStorageMem GnomeStorageMem;
#endif

#define GNOME_STREAM_MEM_TYPE        (gnome_stream_mem_get_type ())
#define GNOME_STREAM_MEM(o)          (GTK_CHECK_CAST ((o), GNOME_STREAM_MEM_TYPE, GnomeStreamMem))
#define GNOME_STREAM_MEM_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_STREAM_MEM_TYPE, GnomeStreamMemClass))
#define GNOME_IS_STREAM_MEM(o)       (GTK_CHECK_TYPE ((o), GNOME_STREAM_MEM_TYPE))
#define GNOME_IS_STREAM_MEM_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_STREAM_MEM_TYPE))

struct _GnomeStreamMem {
	GnomeStream  stream;

	char        *buffer;
	size_t       size;
	long         pos;
	gboolean     read_only;

	GnomeStreamMemPrivate *priv
};

typedef struct {
	GnomeStreamClass parent_class;
} GnomeStreamMemClass;

GtkType         gnome_stream_mem_get_type     (void);
GnomeStream    *gnome_stream_mem_create       (char *buffer, size_t size, gboolean read_only);

END_GNOME_DECLS

#endif /* _GNOME_STREAM_MEM_H_ */
