NULL=

bin_PROGRAMS=bonobo-activation-server activation-client

INCLUDES=					\
	-I$(top_srcdir) 			\
	-I$(top_builddir) 			\
	-I$(top_builddir)/bonobo-activation	\
	-I$(top_srcdir)/bonobo-activation	\
	-DOAFINFODIR=\"$(pkgdatadir)\" 		\
	-DOAF_LOCALEDIR=\""$(datadir)/locale"\" \
	$(SERVER_CFLAGS)			\
	-DOAF_CONFDIR=\"$(sysconfdir)\"		\
	$(NULL)

bonobo_activation_server_SOURCES=				\
	activation-server-corba-extensions.c			\
	activation-server-corba-extensions.h			\
	qsort_ex.c						\
	qsort_ex.h						\
	Bonobo_ActivationContext-skels.c			\
	activation-context-query-lexer.c			\
	activation-context-query-parser.c			\
	activation-context-query-parser.h			\
	activation-context-query-expr.h				\
	activation-context-corba.c				\
	activation-context-query-expr.c				\
	object-directory-corba.c				\
	object-directory-load.c					\
	object-directory-utils.c				\
	object-directory-activate.c				\
	main.c							\
	object-directory-utils.h				\
	server.h

LDADD=../bonobo-activation/libbonobo-activation.la \
	$(SERVER_LIBS) -lname-server @INTLLIBS@

activation_client_SOURCES=client.c

serverinfodir=$(pkgdatadir)
server_in_files = Bonobo_CosNaming_NamingContext.server.in
serverinfo_DATA = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST=activation-context-query-lexer.l activation-context-query-parser.y \
	$(server_in_files) $(serverinfo_DATA)

YFLAGS = -d -v

activation-context-query-lexer.c: $(srcdir)/activation-context-query-lexer.l activation-context-query-parser.h
	@rm -f $@
	$(LEX) -t $(srcdir)/activation-context-query-lexer.l >$@

stamp-parser: $(srcdir)/activation-context-query-parser.y
	$(YACC) $(YFLAGS) $(srcdir)/activation-context-query-parser.y
	touch $@

activation-context-query-parser.c: stamp-parser
	test -f y.tab.c && mv -f y.tab.c activation-context-query-parser.c || touch $@

activation-context-query-parser.h: stamp-parser
	test -f y.tab.h && mv -f y.tab.h activation-context-query-parser.h || touch $@

BUILT_SOURCES= activation-context-query-lexer.c activation-context-query-parser.c \
	activation-context-query-parser.h Bonobo_ActivationContext-skels.c Bonobo_CosNaming_NamingContext.server

Bonobo_ActivationContext-skels.c : $(top_srcdir)/idl/Bonobo_ActivationContext.idl $(ORBIT_IDL)
	$(ORBIT_IDL) --noheaders $(top_srcdir)/idl/Bonobo_ActivationContext.idl

CLEANFILES=stamp-parser 

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
