SUBDIRS = test-activation

noinst_PROGRAMS =		\
	test-thread		\
	test-moniker		\
	test-properties-server	\
	test-properties-client	\
	test-event-source	\
	test-object		\
	test-stream-mem		\
	test-storage-mem

#	generic-factory		\
#	test-generic-factory

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(top_srcdir) 				\
	$(WARN_CFLAGS)					\
	$(LIBBONOBO_CFLAGS)

LDADD =	$(top_builddir)/bonobo/libbonobo-2.la $(LIBBONOBO_LIBS)

test_properties_server_SOURCES =			\
	test-properties-server.c

test_properties_client_SOURCES =			\
	test-properties-client.c

test_thread_SOURCES = test-thread.c

test_moniker_SOURCES = test-moniker.c

test_object_SOURCES = test-object.c

test_event_source_SOURCES = test-event-source.c

test_stream_mem_SOURCES = test-stream-mem.c

test_storage_mem_SOURCES = test-storage-mem.c

TESTS_ENVIRONMENT =					\
	LD_LIBRARY_PATH="$(top_builddir)/bonobo/.libs:$$LD_LIBRARY_PATH"	\
	BONOBO_PLUGIN_PATH="$(top_builddir)/storage-modules/.libs:$$BONOBO_PLUGIN_PATH"

TESTS = test-moniker test-event-source test-object \
	test-stream-mem $(srcdir)/test-properties.sh test-storage-mem
#	test-generic-factory test-generic-factory.sh
EXTRA_DIST = test-properties.sh
# Test_Generic_Factory.server.in
# test-generic-factory.sh.in

# noinst_DATA=Test_Generic_Factory.server

# Test_Generic_Factory.server: Test_Generic_Factory.server.in
#	sed -e "s|\@builddir\@|$(shell pwd)|"  $< > $@

# test-generic-factory.sh: test-generic-factory.sh.in
#	sed -e "s|\@bonoboserverdir\@|$(libdir)/bonobo/servers|"  $< > $@

# noinst_SCRIPTS=test-generic-factory.sh

# CLEANFILES=Test_Generic_Factory.server test-generic-factory.sh

