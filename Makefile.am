SUBDIRS = \
	idl bonobo-activation bonobo \
	activation-server monikers \
	samples tests doc po

libincludedir = $(includedir)/libbonobo-2.0
libinclude_HEADERS = \
	libbonobo.h

EXTRA_DIST = \
	bonobo-activation-config.xml \
	intltool-extract.in  \
	intltool-merge.in    \
	intltool-update.in   \
	changes.txt          \
	gtk-doc.make

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

install-data-local:
	-mkdir -p $(sysconfdir)/bonobo-activation
	if test -f $(sysconfdir)/bonobo-activation/bonobo-activation-config.xml; then		\
		echo "bonobo-activation-config.xml configuration file not changed";		\
	else											\
		$(INSTALL_DATA) bonobo-activation-config.xml $(sysconfdir)/bonobo-activation;	\
	fi
