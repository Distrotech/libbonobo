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
	bonobo-activation-async.h		\
	bonobo-activation-base-service.h	\
	bonobo-activation-id.h			\
	bonobo-activation-activate.h		\
	bonobo-activation-init.h		\
	bonobo-activation-shlib.h		\
	bonobo-activation-register.h		\
	bonobo-activation-server-info.h		\
	bonobo-activation-async.h		\
	bonobo-factory-suppress.h		\
	bonobo-activation-version.h		\
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
	bonobo-activation-id.c			\
	bonobo-activation-private.h		\
	bonobo-activation-shlib.c		\
	bonobo-activation-activate.c		\
	bonobo-activation-activate-private.h	\
	bonobo-activation-base-service.c	\
	bonobo-activation-register.c		\
	bonobo-activation-server-info.c		\
	bonobo-activation-i18n.h		\
	bonobo-activation-async.c		\
	bonobo-activation-async-callback.h	\
	bonobo-activation-async-callback.c	\
	bonobo-activation-fork-server.c		\
	$(NULL)



bonobo_unknown_idls =				\
	$(top_srcdir)/idl/Bonobo_Unknown.idl	\
	$(NULL)

bonobo_unknown_idl_flags = -I$(top_srcdir)/idl -D__BONOBO_UNKNOWN_COMPILATION

$(bonobo_unknown_sources): $(bonobo_unknown_idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(bonobo_unknown_idl_flags) $(top_srcdir)/idl/Bonobo_Unknown.idl


bonobo_generic_factory_idls =				\
	$(top_srcdir)/idl/Bonobo_Unknown.idl		\
	$(top_srcdir)/idl/Bonobo_GenericFactory.idl	\
	$(NULL)

bonobo_generic_factory_idl_flags = -I$(top_srcdir)/idl -D__BONOBO_GENERIC_FACTORY_COMPILATION

$(bonobo_generic_factory_sources): $(bonobo_generic_factory_idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(bonobo_generic_factory_idl_flags) $(top_srcdir)/idl/Bonobo_GenericFactory.idl


bonobo_activation_context_idls =			\
	$(top_srcdir)/idl/Bonobo_Unknown.idl		\
	$(top_srcdir)/idl/Bonobo_GenericFactory.idl	\
	$(top_srcdir)/idl/Bonobo_ActivationContext.idl	\
	$(NULL)

bonobo_activation_context_idl_flags = -I$(top_srcdir)/idl -D__BONOBO_ACTIVATION_CONTEXT_COMPILATION

$(bonobo_activation_context_sources): $(bonobo_activation_context_idls) $(ORBIT_IDL)
	$(ORBIT_IDL) $(bonobo_activation_context_idl_flags) $(top_srcdir)/idl/Bonobo_ActivationContext.idl


CLEANFILES=					\
	$(BUILT_SOURCES)			\
	$(NULL)

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
