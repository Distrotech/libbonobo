lib_LTLIBRARIES=liboaf.la

INCLUDES=@ORBIT_CFLAGS@

LDADD=@ORBIT_LIBS@

liboafincludedir = $(includedir)/liboaf
liboafinclude_HEADERS=oaf.h

liboaf_la_SOURCES=oaf-stubs.c oaf-common.c

BUILT_SOURCES=oaf-stubs.c oaf.h oaf-common.c

oaf-stubs.c oaf.h oaf-common.c: stamp-idl

stamp-idl: $(top_srcdir)/idl/oaf.idl
	orbit-idl --noskels $(top_srcdir)/idl/oaf.idl
	touch stamp-idl

CLEANFILES=stamp-idl