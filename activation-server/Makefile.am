NULL=

bin_PROGRAMS=bonobo-activation-server

INCLUDES=							\
	-I$(top_srcdir)						\
	-I$(top_builddir)					\
	-I$(top_builddir)/liboaf				\
	-I$(top_srcdir)/liboaf					\
	-DSERVERINFODIR=\"$(pkgdatadir)/servers\"		\
	-DBONOBO_ACTIVATION_LOCALEDIR=\""$(datadir)/locale"\"	\
	$(SERVER_CFLAGS)					\
	-DBONOBO_ACTIVATION_CONFDIR=\"$(sysconfdir)\"		\
	$(NULL)

bonobo_activation_server_SOURCES=		\
	activation-server-corba-extensions.c		\
	activation-server-corba-extensions.h	\
	qsort_ex.c				\
	qsort_ex.h				\
	activation-context-query-lexer.c	\
	activation-context-query-parser.c	\
	activation-context-query-parser.h	\
	ac-query-expr.h				\
	ac-corba.c				\
	ac-query-expr.c				\
	od-corba.c				\
	od-load.c				\
	od-utils.c				\
	od-activate.c				\
	main.c					\
	od-utils.h				\
	oafd.h

LDADD=../bonobo-activation/libbonobo-activation.la \
	$(SERVER_LIBS) -lname-server @INTLLIBS@

serverinfodir=$(pkgdatadir)

server_in_files = Bonobo_CosNaming_NamingContext.server.in
serverinfo_DATA = $(server_in_files:.server.in=.server)

@XML_I18N_MERGE_SERVER_RULE@

EXTRA_DIST=activation-context-query-lexer.l activation-context-query-parser.y $(server_in_files) $(serverinfo_DATA)

YFLAGS                  = -d -v

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

BUILT_SOURCES=activation-context-query-lexer.c activation-context-query-parser.c activation-context-query-parser.h Bonobo_CosNaming_NamingContext.server

CLEANFILES=stamp-parser 

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
