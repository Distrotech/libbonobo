bin_PROGRAMS=oafd

INCLUDES=@ORBIT_CFLAGS@ -I$(top_builddir) -I$(top_builddir)/liboaf

oafd_SOURCES=ac-query-lex.c ac-query-parse.c ac-query-parse.h ac-query-expr.h main.c ac-query-expr.c oaf-skels.c
oafd_LDADD=../liboaf/liboaf.la @ORBIT_LIBS@

EXTRA_DIST=ac-query-lex.l ac-query-parse.y

YFLAGS                  = -d -v

ac-query-lex.c: $(srcdir)/ac-query-lex.l ac-query-parse.h
	@rm -f $@
	$(LEX) -t $(srcdir)/ac-query-lex.l >$@

stamp-parser: $(srcdir)/ac-query-parse.y
	$(YACC) $(YFLAGS) $(srcdir)/ac-query-parse.y
	touch $@

ac-query-parse.c: stamp-parser
	test -f y.tab.c && mv -f y.tab.c ac-query-parse.c || touch $@

ac-query-parse.h: stamp-parser
	test -f y.tab.h && mv -f y.tab.h ac-query-parse.h || touch $@

BUILT_SOURCES=ac-query-lex.c ac-query-parse.c ac-query-parse.h

CLEANFILES=stamp-parser stamp-idl

BUILT_SOURCES=oaf-skels.c

oaf-skels.c: $(top_srcdir)/idl/oaf.idl
	orbit-idl --nostubs --noheaders $(top_srcdir)/idl/oaf.idl
