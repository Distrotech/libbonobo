NULL=

lib_LTLIBRARIES=libbonobo-activation.la

INCLUDES=					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-I$(top_srcdir)/bbonobo-activation	\
	-I$(top_builddir)/bonobo-activation	\
	-DOAF_LOCALEDIR=\""$(datadir)/locale"\"	\
	$(BONOBO_ACTIVATION_CFLAGS)		\
	-DVERSION=\"$(VERSION)\"		\
	$(NULL)

LDFLAGS=					\
	$(BONOBO_ACTIVATION_LIBS)		\
	@LT_VERSION_INFO@			\
	$(NULL)

bonobo_activationincludedir = $(includedir)/bonobo-activation-2.0/bonobo-activation

bonobo_activationinclude_HEADERS=		\
	Bonobo_Unknown.h			\
	Bonobo_GenericFactory.h			\
	Bonobo_ActivationContext.h		\
	liboaf.h				\
	oaf-async.h				\
	oaf-registration.h			\
	oaf-actid.h				\
	oaf-activate.h				\
	oaf-mainloop.h				\
	oaf-plugin.h				\
	oaf-servreg.h				\
	oaf-util.h				\
	oaf-async.h				\
	oaf-factory.h				\
	bonobo-factory-suppress.h		\
	liboaf-version.h			\
	$(NULL)


bonobo_unknown_sources =			\
	Bonobo_Unknown.h			\
	Bonobo_Unknown-common.c			\
	Bonobo_Unknown-stubs.c			\
	Bonobo_Unknown-skels.c			\
	$(NULL)

bonobo_generic_factory_sources =		\
	Bonobo_GenericFactory.h			\
	Bonobo_GenericFactory-common.c		\
	Bonobo_GenericFactory-stubs.c		\
	Bonobo_GenericFactory-skels.c		\
	$(NULL)
bonobo_activation_context_sources =		\
	Bonobo_ActivationContext.h		\
	Bonobo_ActivationContext-common.c	\
	Bonobo_ActivationContext-stubs.c	\
	Bonobo_ActivationContext-skels.c	\
	$(NULL)

BUILT_SOURCES=					\
	$(bonobo_unknown_sources)		\
	$(bonobo_generic_factory_sources)	\
	$(bonobo_activation_context_sources)	\
	$(NULL)


libbonobo_activation_la_SOURCES=		\
	$(BUILT_SOURCES)			\
	oaf-actid.c				\
	liboaf-private.h			\
	oaf-factory-stubs.c			\
	oaf-factory-skels.c			\
	oaf-factory-common.c			\
	oaf-plugin.c				\
	oaf-activate.c				\
	oaf-activate-private.h			\
	oaf-registration.c			\
	oaf-servreg.c				\
	oaf-util.c				\
	oaf-i18n.h				\
	oaf-async.c				\
	oaf-async-corba.h			\
	oaf-async-corba.c			\
	oaf-fork-server.c			\
	$(NULL)



bonobo_unknown_idls =				\
	$(top_srcdir)/idl/Bonobo_Unknown.idl	\
	$(NULL)

bonobo_unknown_idl_flags = -I$(top_srcdir)/idl -D__BONOBO_UNKNOWN_COMPILATION

$(bonobo_unknown_sources): $(bonobo_unknown_idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(idl_flags) $(top_srcdir)/idl/Bonobo_Unknown.idl


bonobo_generic_factory_idls =				\
	$(top_srcdir)/idl/Bonobo_Unknown.idl		\
	$(top_srcdir)/idl/Bonobo_GenericFactory.idl	\
	$(NULL)

bonobo_generic_factory_idl_flags = -I$(top_srcdir)/idl -D__BONOBO_GENERIC_FACTORY_COMPILATION

$(bonobo_generic_factory_sources): $(bonobo_generic_factory_idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(idl_flags) $(top_srcdir)/idl/Bonobo_GenericFactory.idl


bonobo_activation_context_idls =			\
	$(top_srcdir)/idl/Bonobo_Unknown.idl		\
	$(top_srcdir)/idl/Bonobo_GenericFactory.idl	\
	$(top_srcdir)/idl/Bonobo_ActivationContext.idl	\
	$(NULL)

bonobo_activation_context_idl_flags = -I$(top_srcdir)/idl -D__BONOBO_ACTIVATION_CONTEXT_COMPILATION

$(bonobo_activation_context_sources): $(bonobo_activation_context_idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(idl_flags) $(top_srcdir)/idl/Bonobo_ActivationContext.idl


CLEANFILES=					\
	$(BUILT_SOURCES)			\
	$(NULL)

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
