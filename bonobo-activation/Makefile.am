lib_LTLIBRARIES=liboaf.la

INCLUDES=@ORBIT_CFLAGS@ -I$(top_srcdir) -I$(top_builddir)

LDADD=@ORBIT_LIBS@

liboafincludedir = $(includedir)/liboaf
liboafinclude_HEADERS=oaf.h liboaf.h

liboaf_la_SOURCES=oaf-stubs.c oaf-common.c oaf-mainloop.c oaf-actid.c liboaf-private.h

BUILT_SOURCES=oaf-stubs.c oaf.h oaf-common.c

oaf-stubs.c oaf.h oaf-common.c: stamp-idl

stamp-idl: $(top_srcdir)/idl/oaf.idl
	orbit-idl --noskels $(top_srcdir)/idl/oaf.idl
	touch stamp-idl

CLEANFILES=stamp-idl
