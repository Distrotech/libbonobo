lib_LTLIBRARIES=liboaf.la

liboaf_la_SOURCES=query-lex.c query-parse.c query-parse.h query-lex.h

EXTRA_DIST=query-lex.l query-parse.y

YFLAGS                  = -d -v 2>/dev/null
LIBS                    = @GLIB_LIBS@

query-lex.c: $(srcdir)/query-lex.l query-parse.h
	@rm -f $@
	$(LEX) -t $(srcdir)/query-lex.l | sed -e 's/yy/__oaf_query_/g' >$@

stamp-parser: $(srcdir)/query-parse.y
	$(YACC) $(YFLAGS) $(srcdir)/query-parse.y
	touch $@

query-parse.c: stamp-parser
	test -f y.tab.c && mv -f y.tab.c query-parse.c || touch $@

query-parse.h: stamp-parser
	test -f y.tab.h && mv -f y.tab.h query-parse.h || touch $@

BUILT_SOURCES=query-lex.c query-parse.c query-parse.h query-lex.h

CLEANFILES=stamp-parser
