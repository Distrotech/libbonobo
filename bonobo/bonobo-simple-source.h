/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
#ifndef _GNOME_SIMPLE_DATA_SOURCE_H_
#define _GNOME_SIMPLE_DATA_SOURCE_H_

#include <bonobo/bonobo-object.h>

BEGIN_GNOME_DECLS

#define GNOME_SIMPLE_DATA_SOURCE_TYPE        (gnome_simple_data_source_get_type ())
#define GNOME_SIMPLE_DATA_SOURCE(o)          (GTK_CHECK_CAST ((o), GNOME_SIMPLE_DATA_SOURCE_TYPE, GnomeSimpleDataSource))
#define GNOME_SIMPLE_DATA_SOURCE_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), GNOME_SIMPLE_DATA_SOURCE_TYPE, GnomeSimpleDataSourceClass))
#define GNOME_IS_SIMPLE_DATA_SOURCE(o)       (GTK_CHECK_TYPE ((o), GNOME_SIMPLE_DATA_SOURCE_TYPE))
#define GNOME_IS_SIMPLE_DATA_SOURCE_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), GNOME_SIMPLE_DATA_SOURCE_TYPE))

typedef struct _GnomeSimpleDataSource GnomeSimpleDataSource;
typedef struct _GnomeSimpleDataSourcePrivate GnomeSimpleDataSourcePrivate;

/* Callback typedefs. */
typedef int (*GnomeSimpleDataSourcePopDataFn)			(GnomeSimpleDataSource *ssource,
								 const CORBA_long count,
								 GNOME_SimpleDataSource_iobuf **buffer,
								 void *closure);

typedef CORBA_long (*GnomeSimpleDataSourceRemainingDataFn)	(GnomeSimpleDataSource *ssource,
								 void *closure);


struct _GnomeSimpleDataSource {
	BonoboObject object;

	/*
	 * These are the callbacks the user can set.  If we use the
	 * default class methods, then these are NULL.
	 */
	GnomeSimpleDataSourcePopDataFn pop_data_fn;
	GnomeSimpleDataSourceRemainingDataFn remaining_data_fn;

	void *closure;

	GnomeSimpleDataSourcePrivate *priv;
};

typedef struct {
	BonoboObjectClass parent_class;

	/*
	 * Methods.
	 */
	int (*pop_data_fn)		(GnomeSimpleDataSource *ssource,
					 const CORBA_long count,
					 GNOME_SimpleDataSource_iobuf **buffer);

	CORBA_long (*remaining_data_fn)	(GnomeSimpleDataSource *ssource);
			 
} GnomeSimpleDataSourceClass;


GtkType		gnome_simple_data_source_get_type		(void);

GnomeSimpleDataSource *gnome_simple_data_source_new		(GnomeSimpleDataSourcePopDataFn pop_data_fn,
								 GnomeSimpleDataSourceRemainingDataFn remaining_data_fn,
								 void *closure);

GnomeSimpleDataSource *gnome_simple_data_source_construct	(GnomeSimpleDataSource *ssource,
								 GNOME_SimpleDataSource corba_ssource,
								 GnomeSimpleDataSourcePopDataFn pop_data_fn,
								 GnomeSimpleDataSourceRemainingDataFn remaining_data_fn,
								 void *closure);
							       
extern POA_GNOME_SimpleDataSource__epv gnome_simple_data_source_epv;
END_GNOME_DECLS

#endif /* _GNOME_SIMPLE_DATA_SOURCE_H_ */

