INCLUDES = \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-I$(builddir)					\
        -I$(includedir)                                 \
	$(WARN_CFLAGS)					\
	$(GLIB_CFLAGS)					\
	$(ORBIT_CFLAGS)					\
	$(OAF_CFLAGS)					\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
        -DBONOBO_LIBDIR=\""$(libdir)"\" 		\
        -DBONOBO_DATADIR=\""$(datadir)"\" 		\
        -DBONOBO_PIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DBONOBO_BINDIR=\""$(bindir)"\" 		\
        -DBONOBO_LOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DBONOBO_LOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"

Bonobo_Moniker_std.oaf.in : $(srcdir)/Bonobo_Moniker_std.oaf.in.in
	sed -e "s|\@MONIKER_LIBDIR\@|$(monikerdir)|" \
	 $(srcdir)/Bonobo_Moniker_std.oaf.in.in > Bonobo_Moniker_std.oaf.in

CLEANFILES+=Bonobo_Moniker_std.oaf Bonobo_Moniker_std.oaf.in

oafdir = $(datadir)/oaf
oaf_DATA = Bonobo_Moniker_std.oaf

@XML_I18N_MERGE_OAF_RULE@

monikerdir = $(libdir)/bonobo/monikers
moniker_LTLIBRARIES = libmoniker_std.la

libmoniker_std_la_SOURCES =			\
	bonobo-moniker-extender-stream.c  	\
	bonobo-moniker-item.c			\
	bonobo-moniker-new.c			\
	bonobo-moniker-oaf.c			\
	bonobo-moniker-cache.c			\
	bonobo-stream-cache.h			\
	bonobo-stream-cache.c			\
	bonobo-moniker-query.c			\
	bonobo-moniker-query.h			\
	bonobo-moniker-std.h            	\
	bonobo-moniker-std.c			

EXTRA_DIST = Bonobo_Moniker_std.oaf.in.in
