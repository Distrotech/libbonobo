INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	$(BONOBO_TEST_CFLAGS)

oafdir = $(datadir)/oaf
oaf_DATA = std-moniker.oafinfo bonobo-moniker-gunzip.oafinfo bonobo-moniker-http.oafinfo bonobo-moniker-untar.oafinfo

bin_PROGRAMS = std-moniker bonobo-moniker-config bonobo-moniker-gunzip bonobo-moniker-http bonobo-moniker-untar

noinst_PROGRAMS = moniker-control-test moniker-test

moniker_ldadd = 				\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
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

bonobo_moniker_gunzip_SOURCES =		\
	bonobo-moniker-gunzip.c		\
	bonobo-moniker-gunzip.h

bonobo_moniker_gunzip_LDADD =		\
	$(moniker_ldadd)

bonobo_moniker_untar_SOURCES =          \
	bonobo-moniker-untar.c          \
	bonobo-moniker-untar.h

bonobo_moniker_untar_LDADD =            \
	$(moniker_ldadd)

bonobo_moniker_http_SOURCES =		\
	bonobo-moniker-http.c		\
	bonobo-moniker-http.h

bonobo_moniker_http_LDADD =		\
	$(moniker_ldadd)

moniker_control_test_SOURCES =		\
	moniker-control-test.c

moniker_control_test_LDADD =		\
	$(moniker_ldadd) -lgtkhtml

moniker_test_SOURCSE =			\
	moniker-test.c

moniker_test_LDADD =			\
	$(moniker_ldadd) -lgtkhtml

EXTRA_DIST=std-moniker.oafinfo bonobo-moniker-gunzip.oafinfo bonobo-moniker-http.oafinfo
