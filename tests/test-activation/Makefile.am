noinst_PROGRAMS=oaf-empty-server oaf-test-client
bin_PROGRAMS=oaf-run-query

oaf_test_client_SOURCES=oaf-test-client.c empty-common.c empty.h empty-stubs.c

oaf_run_query_SOURCES=oaf-run-query.c empty-common.c empty.h empty-stubs.c

oaf_empty_server_SOURCES=empty-server.c empty.h empty-common.c empty-skels.c

BUILT_SOURCES=empty.h empty-common.c empty-stubs.c empty-skels.c

empty.h empty-common.c empty-stubs.c empty-skels.c: empty_built

empty_built: empty.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/empty.idl
	touch empty_built

CLEANFILES=empty_built

INCLUDES=-I$(top_srcdir) -I$(top_builddir) -I$(top_builddir)/liboaf -DOAFINFODIR=\"$(pkgdatadir)\" \
	@ORBIT_CFLAGS@ @XML_CFLAGS@ @GLIB_CFLAGS@
LDADD=../liboaf/liboaf.la @ORBIT_LIBS@ @GLIB_LIBS@

oaffiles=empty.oafinfo broken.oafinfo

emptydatadir=$(datadir)/oaf
emptydata_DATA=$(oaffiles)

EXTRA_DIST=empty.idl $(oaffiles)

TESTS=oaf-test-client
