INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-I$(top_srcdir)/bonobo                          \
        -I$(includedir)                                 \
	$(BONOBO_TEST_CFLAGS)

OAF_FILES =					\
	Bonobo_Moniker_std.oafinfo		\
	Bonobo_Moniker_gzip.oafinfo		\
	Bonobo_Moniker_http.oafinfo		\
	Bonobo_Moniker_Extender_file.oafinfo

oafdir = $(datadir)/oaf
oaf_DATA = $(OAF_FILES)

# bonobo-moniker-untar.oafinfo

bin_PROGRAMS = std-moniker bonobo-moniker-gunzip bonobo-moniker-http bonobo-moniker-extender-file

# bonobo-moniker-untar

#noinst_PROGRAMS = moniker-control-test moniker-test
noinst_PROGRAMS = moniker-test

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

bonobo_moniker_extender_file_SOURCES =	\
	bonobo-moniker-extender-file.h	\
	bonobo-moniker-extender-file.c

bonobo_moniker_extender_file_LDADD =	\
	$(moniker_ldadd)

bonobo_moniker_gunzip_SOURCES =		\
	bonobo-moniker-gunzip.c		\
	bonobo-moniker-gunzip.h

bonobo_moniker_gunzip_LDADD =		\
	$(moniker_ldadd)

## bonobo_moniker_untar_SOURCES =          \
## 	bonobo-moniker-untar.c          \
## 	bonobo-moniker-untar.h

## bonobo_moniker_untar_LDADD =            \
## 	$(moniker_ldadd)

bonobo_moniker_http_SOURCES =		\
	bonobo-moniker-http.c		\
	bonobo-moniker-http.h

bonobo_moniker_http_LDADD =		\
	$(moniker_ldadd)

#moniker_control_test_SOURCES =		\
#	moniker-control-test.c

#moniker_control_test_LDADD =		\
#	$(moniker_ldadd) -lgtkhtml

moniker_test_SOURCES =			\
	moniker-test.c

moniker_test_LDADD =			\
	$(moniker_ldadd)

EXTRA_DIST= $(OAF_FILES)
