NULL=

noinst_PROGRAMS=			\
	bonobo-activation-empty-server 	\
	bonobo-activation-test		\
	bonobo-activation-test-async	\
	$(NULL)

bin_PROGRAMS=				\
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

empty_built: empty.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/empty.idl
	touch empty_built

CLEANFILES=empty_built

INCLUDES=-I$(top_srcdir) -I$(top_builddir) $(UTILS_CFLAGS)

LDADD=../bonobo-activation/libbonobo-activation.la $(UTILS_LIBS)

serverfiles=empty.server

emptydatadir=$(datadir)/oaf
emptydata_DATA=$(serverfiles)

bin_SCRIPTS=bonobo-activation-slay

EXTRA_DIST=empty.idl $(serverfiles) broken.server bonobo-activation-slay

check:
	BONOBO_ACTIVATION_PATH="$(top_srcdir)/test:$$BONOBO_ACTIVATION_PATH" PATH="$(top_builddir)/test:$$PATH" $(top_builddir)/test/bonobo-activation-test
