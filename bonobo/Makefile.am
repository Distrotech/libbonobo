INCLUDES = \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-I$(builddir)					\
        -I$(includedir)                                 \
	$(WARN_CFLAGS)					\
        -DBONOBOLIBDIR=\""$(libdir)"\" 		\
        -DBONOBODATADIR=\""$(datadir)"\" 		\
        -DBONOBOPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DBONOBOBINDIR=\""$(bindir)"\" 		\
        -DBONOBOLOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DBONOBOLOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"

lib_LTLIBRARIES = libbonobo-2.la

libbonobo_2_la_LIBADD =

libbonoboincludedir = $(includedir)/gnome/2/libbonobo

libbonoboinclude_HEADERS =

libbonobo_2_la_SOURCES =			\
	$(libbonoboinclude_HEADERS)

EXTRA_DIST = libbonobo-2.0.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbonobo-2.0.pc

