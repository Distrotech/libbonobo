INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	$(GNOME_INCLUDEDIR)				\
	$(OAF_CFLAGS)					\
	$(GDK_PIXBUF_CFLAGS)

gnome_libs =				\
	$(GNOME_LIBDIR)			\
	$(GDK_PIXBUF_LIBS) 		\
        $(GNOMEUI_LIBS)			\
	$(OAF_LIBS) 	\
	$(INTLLIBS)

bonobo_libs =				\
	$(top_builddir)/bonobo/libbonobo.la

oafdir = $(datadir)/oaf
oaf_DATA = std-moniker.oafinfo

bin_PROGRAMS = std-moniker

std_moniker_SOURCES =			\
	bonobo-moniker-file.c		\
	bonobo-moniker-file.h		\
	bonobo-moniker-item.c		\
	bonobo-moniker-item.h		\
	bonobo-moniker-oaf.c		\
	bonobo-moniker-oaf.h		\
	bonobo-moniker-query.c		\
	bonobo-moniker-query.h		\
	std-moniker.c

std_moniker_LDADD =			\
	$(bonobo_libs)
