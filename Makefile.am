SUBDIRS = \
	idl bonobo-activation bonobo \
	activation-server monikers \
	samples tests doc po

libincludedir = $(includedir)/libbonobo-2.0
libinclude_HEADERS = \
	libbonobo.h

EXTRA_DIST = \
	bonobo-activation-config.xml.in \
	intltool-extract.in  \
	intltool-merge.in    \
	intltool-update.in   \
	changes.txt          \
	gtk-doc.make

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

sysconfigdir=$(sysconfdir)/bonobo-activation
sysconfig_DATA = \
	bonobo-activation-config.xml
