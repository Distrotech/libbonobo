NULL = 

bin_PROGRAMS = oaf-sysconf

oaf_sysconf_SOURCES = 			\
	oaf-sysconf.c			\
	$(NULL)

INCLUDES = 					\
	$(XML_CFLAGS)				\
	$(GLIB_CFLAGS)				\
	-I$(top_srcdir)/oafd/			\
	-DOAF_LOCALEDIR=\""$(datadir)/locale"\"	\
	-DOAF_CONFDIR=\"$(sysconfdir)\"		\
	$(NULL)

oaf_sysconf_LDADD =			\
	$(XML_LIBS)			\
	$(GLIB_LIBS)			\
	$(NULL)
