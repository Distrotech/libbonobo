INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	$(BONOBO_TEST_CFLAGS)

oafdir = $(datadir)/oaf
oaf_DATA = std-moniker.oafinfo

bin_PROGRAMS = std-moniker bonobo-moniker-config

moniker_ldadd = 				\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(BONOBOX_TEST_LIBS)

std_moniker_SOURCES =			\
	bonobo-moniker-file.c		\
	bonobo-moniker-file.h		\
	bonobo-moniker-item.c		\
	bonobo-moniker-item.h		\
	bonobo-moniker-new.c		\
	bonobo-moniker-new.h		\
	bonobo-moniker-oaf.c		\
	bonobo-moniker-oaf.h		\
	bonobo-moniker-query.c		\
	bonobo-moniker-query.h		\
	std-moniker.c

std_moniker_LDADD =			\
	$(moniker_ldadd)

bonobo_moniker_config_SOURCES =		\
	bonobo-moniker-config.c		\
	bonobo-moniker-config.h

bonobo_moniker_config_LDADD =		\
	$(moniker_ldadd)

EXTRA_DIST=std-moniker.oafinfo
