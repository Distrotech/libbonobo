SUBDIRS = \
	idl bonobo-activation bonobo \
	activation-server monikers \
	samples tests po
# doc - needs newer automake sadly

libincludedir = $(includedir)/libbonobo-2.0
libinclude_HEADERS = \
	libbonobo.h

EXTRA_DIST = \
	intltool-extract.in  \
	intltool-merge.in    \
	intltool-update.in   \
	changes.txt          \
	gtk-doc.make

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

