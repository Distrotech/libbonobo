ORBIT_IDL = @ORBIT_IDL@

PLUGIN_DIR = $(libdir)/bonobo/plugin
orbittypelibdir = $(libdir)/orbit

INCLUDES = \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-I$(top_builddir)/bonobo			\
	-I$(builddir)					\
        -I$(includedir)                                 \
	$(WARN_CFLAGS)					\
	$(LIBBONOBO_CFLAGS)				\
	-DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"		\
        -DBONOBO_LIBDIR=\""$(libdir)"\" 		\
        -DBONOBO_DATADIR=\""$(datadir)"\" 		\
        -DBONOBO_PIXMAPDIR=\""$(datadir)/pixmaps"\"	\
        -DBONOBO_BINDIR=\""$(bindir)"\" 		\
        -DBONOBO_LOCALSTATEDIR=\""$(localstatedir)"\" 	\
        -DBONOBO_LOCALEDIR=\""$(gnomelocaledir)"\" 	\
	-DG_LOG_DOMAIN=\"Bonobo\"			\
	-DVERSION=\"$(VERSION)\"

CORBA_SOURCE =			\
	Bonobo.h		\
	Bonobo-common.c		\
	Bonobo-stubs.c		\
	Bonobo-skels.c

CORBA_ALL_SOURCE =		\
	$(CORBA_SOURCE)		\
	Bonobo-imethod.c

idls =							\
	$(top_srcdir)/idl/Bonobo.idl			\
	$(top_srcdir)/idl/Bonobo_Canvas.idl		\
	$(top_srcdir)/idl/Bonobo_Context.idl		\
	$(top_srcdir)/idl/Bonobo_Control.idl		\
	$(top_srcdir)/idl/Bonobo_Embeddable.idl		\
	$(top_srcdir)/idl/Bonobo_ItemContainer.idl	\
	$(top_srcdir)/idl/Bonobo_Listener.idl		\
	$(top_srcdir)/idl/Bonobo_Moniker.idl		\
	$(top_srcdir)/idl/Bonobo_Persist.idl		\
	$(top_srcdir)/idl/Bonobo_Print.idl		\
	$(top_srcdir)/idl/Bonobo_Property.idl		\
	$(top_srcdir)/idl/Bonobo_Storage.idl		\
	$(top_srcdir)/idl/Bonobo_UI.idl			\
	$(top_srcdir)/idl/Bonobo_Zoomable.idl		\
	$(top_srcdir)/idl/Bonobo_Exception.idl		\
	$(top_srcdir)/idl/Bonobo_Clipboard.idl

idl_flags = -I$(top_srcdir)/idl -I $(BONOBO_ACTIVATION_IDL_DIR) -D__BONOBO_COMPILATION

$(CORBA_SOURCE): $(idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(idl_flags) $(top_srcdir)/idl/Bonobo.idl

Bonobo-imodule.c : $(idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(idl_flags) --imodule $(top_srcdir)/idl/Bonobo.idl

lib_LTLIBRARIES = libbonobo-2.la

orbittypelib_LTLIBRARIES = Bonobo_module.la

libbonobo_2_la_LIBADD = \
	$(LIBBONOBO_LIBS)

libbonoboincludedir = $(includedir)/gnome/2/bonobo

libbonoboinclude_HEADERS =		\
	Bonobo.h			\
	bonobo-arg.h			\
	bonobo-async.h			\
	bonobo-context.h		\
	bonobo-event-source.h		\
	bonobo-exception.h		\
	bonobo-generic-factory.h	\
	bonobo-item-container.h		\
	bonobo-item-handler.h		\
	bonobo-listener.h		\
	bonobo-main.h			\
	bonobo-moniker-extender.h	\
	bonobo-moniker-simple.h		\
	bonobo-moniker-util.h		\
	bonobo-moniker.h		\
	bonobo-object.h			\
	bonobo-persist-file.h		\
	bonobo-persist-stream.h		\
	bonobo-persist.h		\
	bonobo-persist-client.h		\
	bonobo-property-bag.h		\
	bonobo-property-bag-client.h	\
	bonobo-shlib-factory.h		\
	bonobo-storage.h		\
	bonobo-stream.h			\
	bonobo-stream-client.h		\
	bonobo-stream-memory.h		\
	bonobo-storage-memory.h		\
	bonobo-xobject.h		\
	bonobo-i18n.h			\
	bonobo-marshal.h		\
	bonobo-types.h

noinst_HEADERS =			\
	bonobo-moniker-context.h	\
	bonobo-running-context.h

libbonobo_src =				\
	bonobo-moniker-context.c	\
	bonobo-arg.c			\
	bonobo-async.c			\
	bonobo-context.c		\
	bonobo-event-source.c		\
	bonobo-exception.c		\
	bonobo-generic-factory.c	\
	bonobo-item-container.c		\
	bonobo-item-handler.c		\
	bonobo-listener.c		\
	bonobo-main.c			\
	bonobo-moniker.c		\
	bonobo-moniker-extender.c	\
	bonobo-moniker-simple.c		\
	bonobo-moniker-util.c		\
	bonobo-object.c			\
	bonobo-persist-file.c		\
	bonobo-persist-stream.c		\
	bonobo-persist.c		\
	bonobo-persist-client.c		\
	bonobo-property-bag.c		\
	bonobo-property-bag-client.c	\
	bonobo-shlib-factory.c		\
	bonobo-storage.c		\
	bonobo-stream-client.c		\
	bonobo-stream-memory.c		\
	bonobo-storage-memory.c		\
	bonobo-running-context.c	\
	bonobo-types.c

$(libbonobo_2_la_OBJECTS) : Bonobo.h bonobo-marshal.h

libbonobo_2_la_SOURCES =		\
	$(libbonoboinclude_HEADERS)	\
	$(libbonobo_src)		\
	$(CORBA_SOURCE)

Bonobo_module_la_SOURCES = \
	Bonobo-imodule.c

Bonobo_module_la_LIBADD = \
	$(LIBBONOBO_LIBS)

Bonobo_module_la_LDFLAGS = \
	-export-dynamic -module

marshal_sources = \
	bonobo-marshal.c \
	bonobo-marshal.h

bonobo-marshal.h: bonobo-marshal.list
	cd $(srcdir) \
	&& glib-genmarshal --prefix=bonobo_marshal bonobo-marshal.list --header >> xgen-gmh \
	&& (cmp -s xgen-gmh bonobo-marshal.h || cp xgen-gmh bonobo-marshal.h) \
	&& rm -f xgen-gmh xgen-gmh~

bonobo-marshal.c: bonobo-marshal.list bonobo-marshal.h
	cd $(srcdir) \
	&& glib-genmarshal --prefix=bonobo_marshal bonobo-marshal.list --body >> xgen-gmc \
	&& cp xgen-gmc bonobo-marshal.c \
	&& rm -f xgen-gmc xgen-gmc~

EXTRA_DIST = libbonobo-2.0.pc.in bonobo-marshal.list $(marshal_sources)
MAINTAINERCLEANFILES = $(marshal_sources)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbonobo-2.0.pc

DONT_DIST_SOURCE = $(CORBA_ALL_SOURCE) $(marshal_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
