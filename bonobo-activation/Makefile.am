lib_LTLIBRARIES=liboaf.la

INCLUDES=-I$(top_srcdir) -I$(top_builddir) -I$(top_builddir)/liboaf -I$(top_srcdir)/liboaf @ORBIT_CFLAGS@ @GLIB_CFLAGS@

LDFLAGS=@ORBIT_LIBS@ @GLIB_LIBS@

liboafincludedir = $(includedir)/liboaf
liboafinclude_HEADERS=oaf.h liboaf.h gnome-factory.h

liboaf_la_SOURCES=oaf-stubs.c oaf-common.c oaf-mainloop.c oaf-actid.c liboaf-private.h \
	gnome-factory-stubs.c gnome-factory-common.c oaf-plugin.c oaf-activate.c

BUILT_SOURCES=oaf-stubs.c oaf.h oaf-common.c gnome-factory-stubs.c gnome-factory.h gnome-factory-common.c

oaf-stubs.c oaf.h oaf-common.c: stamp-idl

gnome-factory-stubs.c gnome-factory.h gnome-factory-common.c: stamp-factory-idl

stamp-idl: $(top_srcdir)/idl/oaf.idl
	orbit-idl --noskels -I$(top_srcdir)/idl --define=OAF_COMPILATION $(top_srcdir)/idl/oaf.idl
	touch stamp-idl

stamp-factory-idl: $(top_srcdir)/idl/gnome-factory.idl
	orbit-idl --noskels -I$(top_srcdir)/idl --define=GNOME_FACTORY_COMPILATION $(top_srcdir)/idl/gnome-factory.idl
	touch stamp-factory-idl

CLEANFILES=stamp-idl stamp-factory-idl $(BUILT_SOURCES)
