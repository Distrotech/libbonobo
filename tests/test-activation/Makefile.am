NULL=

noinst_PROGRAMS=			\
	bonobo-activation-test		\
	bonobo-activation-test-async	\
	$(NULL)

noinst_LTLIBRARIES=			\
	libtest-plugin.la		\
	$(NULL)

bin_PROGRAMS=				\
	bonobo-activation-empty-server 	\
	bonobo-activation-run-query	\
	$(NULL)

bonobo_activation_test_async_SOURCES=	\
	bonobo-activation-test-async.c 	\
	$(NULL)

bonobo_activation_test_SOURCES=		\
	bonobo-activation-test.c	\
	empty-common.c			\
	empty.h				\
	empty-stubs.c			\
	$(NULL)

# Horrible, digusting and atrocious hack to
# get libtool to build a noinst shared lib.
libtest_plugin_la_LDFLAGS=					\
	-rpath $(shell (cd $(top_builddir);pwd))/test/.libs 	\
	-module -avoid-version -no-undefined			\
	$(NULL)

libtest_plugin_la_LIBADD=					\
	../../bonobo-activation/libbonobo-activation.la		\
	$(LIBBONOBO_LIBS)

libtest_plugin_la_SOURCES=		\
	plugin.h			\
	plugin-common.c			\
	plugin-skels.c			\
	plugin-stubs.c			\
	bonobo-activation-testplugin.c	\
	$(NULL)

bonobo_activation_run_query_SOURCES=	\
	bonobo-activation-run-query.c 	\
	empty-common.c			\
	empty.h				\
	empty-stubs.c			\
	$(NULL)

bonobo_activation_empty_server_SOURCES=	\
	empty-server.c			\
	empty.h				\
	empty-common.c			\
	empty-skels.c			\
	$(NULL)

BUILT_SOURCES=				\
	empty.h				\
	empty-common.c			\
	empty-stubs.c			\
	empty-skels.c			\
	$(NULL)

empty.h empty-common.c empty-stubs.c empty-skels.c: empty_built
plugin.h plugin-common.c plugin-stubs.c plugin-skels.c: plugin_built

empty_built: empty.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/empty.idl
	touch empty_built

plugin_built: plugin.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/plugin.idl
	touch plugin_built

CLEANFILES=empty_built plugin_built

INCLUDES= \
	-I$(top_srcdir)			\
	-I$(top_builddir)		\
	-DG_DISABLE_DEPRECATED $(LIBBONOBO_CFLAGS)

LDADD = ../../bonobo-activation/libbonobo-activation.la \
	$(LIBBONOBO_LIBS) libtest-plugin.la

serverfiles=empty.server broken.server plugin.server

emptydatadir=$(libdir)/bonobo/servers
emptydata_DATA=$(serverfiles)

EXTRA_DIST= test.shempty.idl plugin.idl $(serverfiles)

TESTS= test.sh bonobo-activation-test-async
