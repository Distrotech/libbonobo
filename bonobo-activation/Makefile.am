NULL=

lib_LTLIBRARIES=liboaf.la

INCLUDES=				\
	-I$(top_srcdir) 		\
	-I$(top_builddir) 		\
	-I$(top_srcdir)/liboaf 		\
	-I$(top_builddir)/liboaf 	\
	@ORBIT_CFLAGS@ 			\
	@GLIB_CFLAGS@ 			\
	-DVERSION=\"$(VERSION)\"	\
	$(NULL)

LDFLAGS=		\
	@ORBIT_LIBS@ 	\
	@GLIB_LIBS@	\
	$(NULL)

liboafincludedir = $(includedir)/liboaf

liboafinclude_HEADERS=		\
	oaf.h 			\
	liboaf.h 		\
	gnome-factory.h 	\
	liboaf-version.h	\
	$(NULL)

liboaf_la_SOURCES=		\
	oaf-stubs.c 		\
	oaf-common.c 		\
	oaf-mainloop.c 		\
	oaf-actid.c 		\
	liboaf-private.h 	\
	gnome-factory-stubs.c 	\
	gnome-factory-common.c 	\
	oaf-plugin.c 		\
	oaf-activate.c 		\
	oaf-registration.c 	\
	oaf-servreg.c 		\
	oaf-util.c		\
	oaf-i18n.h		\
	$(NULL)

BUILT_SOURCES=			\
	oaf-stubs.c 		\
	oaf-common.c 		\
	gnome-factory-stubs.c 	\
	gnome-factory-common.c 	\
	oaf.h 			\
	gnome-factory.h		\
	$(NULL)

oaf-stubs.c oaf.h oaf-common.c: stamp-idl

gnome-factory-stubs.c gnome-factory.h gnome-factory-common.c: stamp-factory-idl

# Work around automake brokenness.
../liboaf/oaf.h: stamp-idl
# And again.
../liboaf/gnome-factory.h: stamp-factory-idl

stamp-idl: $(top_srcdir)/idl/oaf.idl
	$(ORBIT_IDL) --noskels -I$(top_srcdir)/idl --define=OAF_COMPILATION $(top_srcdir)/idl/oaf.idl
	touch stamp-idl

gnome-factory.h stamp-factory-idl: $(top_srcdir)/idl/gnome-factory.idl
	$(ORBIT_IDL) --noskels -I$(top_srcdir)/idl --define=GNOME_FACTORY_COMPILATION $(top_srcdir)/idl/gnome-factory.idl
	touch stamp-factory-idl

CLEANFILES=			\
	stamp-idl 		\
	stamp-factory-idl 	\
	$(BUILT_SOURCES)	\
	$(NULL)



