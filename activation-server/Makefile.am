NULL=

man_MANS = bonobo-activation-server.1

bin_PROGRAMS=bonobo-activation-server activation-client

INCLUDES=					\
	-I$(top_srcdir) 			\
	-I$(top_builddir) 			\
	-I$(top_builddir)/bonobo-activation	\
	-I$(top_srcdir)/bonobo-activation	\
	-DSERVERINFODIR=\"$(libdir)/bonobo/servers\" \
	-DSERVER_LOCALEDIR=\""$(datadir)/locale"\" \
	$(SERVER_CFLAGS)			\
	-DSERVER_CONFDIR=\"$(sysconfdir)\"	\
	-DG_DISABLE_DEPRECATED			\
	$(NULL)

bonobo_activation_server_SOURCES=				\
	activation-server-corba-extensions.c			\
	activation-server-corba-extensions.h			\
	activation-context-query.h				\
	activation-context-query.c				\
	activation-context-query-lexer.c			\
	activation-context-query-parser.c			\
	activation-context-query-parser.h			\
	activation-context-corba.c				\
	object-directory-corba.c				\
	object-directory-load.c					\
	object-directory-activate.c				\
	object-directory-config-file.c				\
	object-directory-config-file.h				\
	activation-server-main.c				\
	server.h

LDADD=../bonobo-activation/libbonobo-activation.la \
	$(SERVER_LIBS) -lname-server-2 @INTLLIBS@

activation_client_SOURCES=client.c

serverinfodir=$(libdir)/bonobo/servers
server_in_files = Bonobo_CosNaming_NamingContext.server.in
serverinfo_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

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
	activation-context-query-parser.h Bonobo_CosNaming_NamingContext.server

CLEANFILES=stamp-parser 

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
