ORBIT_IDL = orbit-idl

PLUGIN_DIR = $(libdir)/bonobo/plugin

INCLUDES = \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-I$(builddir)					\
        -I$(includedir)                                 \
	$(WARN_CFLAGS)					\
	$(GLIB_CFLAGS)					\
	$(ORBIT_CFLAGS)					\
	$(OAF_CFLAGS)					\
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

idls =							\
	$(top_srcdir)/idl/Bonobo.idl			\
	$(top_srcdir)/idl/Bonobo_Advise.idl		\
	$(top_srcdir)/idl/Bonobo_Desktop.idl		\
	$(top_srcdir)/idl/Bonobo_Canvas.idl		\
	$(top_srcdir)/idl/Bonobo_ClientSite.idl		\
	$(top_srcdir)/idl/Bonobo_Context.idl		\
	$(top_srcdir)/idl/Bonobo_Control.idl		\
	$(top_srcdir)/idl/Bonobo_ConfigDatabase.idl	\
	$(top_srcdir)/idl/Bonobo_Embeddable.idl		\
	$(top_srcdir)/idl/Bonobo_ItemContainer.idl	\
	$(top_srcdir)/idl/Bonobo_Listener.idl		\
	$(top_srcdir)/idl/Bonobo_Moniker.idl		\
	$(top_srcdir)/idl/Bonobo_Persist.idl		\
	$(top_srcdir)/idl/Bonobo_Print.idl		\
	$(top_srcdir)/idl/Bonobo_Progressive.idl	\
	$(top_srcdir)/idl/Bonobo_Property.idl		\
	$(top_srcdir)/idl/Bonobo_Storage.idl		\
	$(top_srcdir)/idl/Bonobo_UI.idl			\
	$(top_srcdir)/idl/Bonobo_Unknown.idl		\
	$(top_srcdir)/idl/Bonobo_Zoomable.idl		\
	$(top_srcdir)/idl/Bonobo_Exception.idl

idl_flags = `$(GNOME_CONFIG) --cflags idl` -D__BONOBO_COMPILATION

$(CORBA_SOURCE): $(idls)
	$(ORBIT_IDL) $(top_srcdir)/idl/Bonobo.idl -I$(top_srcdir)/idl $(idl_flags)

lib_LTLIBRARIES = libbonobo-2.la

libbonobo_2_la_LIBADD = \
	$(GLIB_LIBS) $(ORBIT_LIBS) $(OAF_LIBS)

libbonoboincludedir = $(includedir)/gnome/2/bonobo

libbonoboinclude_HEADERS =		\
	Bonobo.h			\
	bonobo-arg.h			\
	bonobo-async.h			\
	bonobo-context.h		\
	bonobo-config-database.h	\
	bonobo-event-source.h		\
	bonobo-exception.h		\
	bonobo-generic-factory.h	\
	bonobo-item-container.h		\
	bonobo-item-handler.h		\
	bonobo-listener.h		\
	bonobo-main.h			\
	bonobo-marshal.h		\
	bonobo-moniker-extender.h	\
	bonobo-moniker-simple.h		\
	bonobo-moniker-util.h		\
	bonobo-moniker.h		\
	bonobo-object-client.h		\
	bonobo-object-directory.h	\
	bonobo-object-io.h		\
	bonobo-object.h			\
	bonobo-persist-file.h		\
	bonobo-persist-stream.h		\
	bonobo-persist.h		\
	bonobo-progressive.h		\
	bonobo-property-bag-client.h	\
	bonobo-property-bag.h		\
	bonobo-property.h		\
	bonobo-shlib-factory.h		\
	bonobo-simple-source.h		\
	bonobo-storage-plugin.h		\
	bonobo-storage.h		\
	bonobo-stream.h			\
	bonobo-stream-client.h		\
	bonobo-stream-memory.h		\
	bonobo-transient.h		\
	bonobo-xobject.h		\
	bonobo-i18n.h			\
	bonobo-marshal.h

noinst_HEADERS =			\
	bonobo-activation-context.h	\
	bonobo-running-context.h

libbonobo_src =				\
	bonobo-marshal-main.c		\
	bonobo-activation-context.c	\
	bonobo-arg.c			\
	bonobo-async.c			\
	bonobo-context.c		\
	bonobo-config-database.c	\
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
	bonobo-object-client.c		\
	bonobo-object-directory.c	\
	bonobo-object-io.c		\
	bonobo-persist-file.c		\
	bonobo-persist-stream.c		\
	bonobo-persist.c		\
	bonobo-progressive.c		\
	bonobo-shlib-factory.c		\
	bonobo-storage-plugin.c		\
	bonobo-storage.c		\
	bonobo-stream-client.c		\
	bonobo-stream-memory.c		\
	bonobo-transient.c		\
	bonobo-property.c		\
	bonobo-property-bag.c		\
	bonobo-property-bag-client.c	\
	bonobo-running-context.c

$(libbonobo_src) : $(CORBA_SOURCE)

libbonobo_2_la_SOURCES = \
	$(libbonoboinclude_HEADERS)	\
	$(libbonobo_src)		\
	$(CORBA_SOURCE)

BUILT_SOURCES = \
	bonobo-marshal.c		\
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

EXTRA_DIST = libbonobo-2.0.pc.in bonobo-marshal.list

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libbonobo-2.0.pc

dist-hook:
	for file in $(CORBA_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
