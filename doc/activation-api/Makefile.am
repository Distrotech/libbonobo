NULL=

# The name of the module.
DOC_MODULE=bonobo-activation

# The top-level SGML file.
DOC_MAIN_SGML_FILE=bonobo-activation.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(top_srcdir)/bonobo-activation

HTML_DIR=$(datadir)/bonobo-activation/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =				\
	tmpl/utilities.sgml		\
	tmpl/initialization.sgml	\
	tmpl/query.sgml			\
	tmpl/registration.sgml		\
	tmpl/debugging.sgml		\
	$(NULL)


if ENABLE_GTK_DOC
bonobo_activation_docdir = $(HTML_DIR)
bonobo_activation_doc_DATA =		\
	bonobo-activation.html		\
	$(NULL)
endif


my_sgml_files = 			\
	tutorial.sgml			\
	migrating.sgml			\
	server-xml-reference.sgml	\
	configuring.sgml		\
	query-langage.sgml		\
	architecture.sgml		\
	bonobo-activation.sgml		\
	fdl.sgml			\
	$(NULL)

EXTRA_DIST = 				\
	$(my_sgml_files)		\
	bonobo-activation.png		\
	bonobo-activation-decl.txt	\
	bonobo-activation-sections.txt	\
	$(NULL)

if ENABLE_GTK_DOC
bonobo-activation.html:
	if test ! -d $(top_builddir)/api-docs/tmpl; then			\
		mkdir $(top_builddir)/api-docs/tmpl; 				\
		cp $(srcdir)/tmpl/*.sgml $(top_builddir)/api-docs/tmpl/; 	\
		cp $(srcdir)/*.sgml $(srcdir)/*.txt  $(top_builddir)/api-docs/; \
	fi
	cd $(top_builddir)/api-docs/ \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)
	if test ! -d $(top_builddir)/api-docs/html; then	\
		mkdir $(top_builddir)/api-docs/html;		\
	fi
	-cd $(top_builddir)/api-docs/html && \
	gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	-cd $(top_builddir)/api-docs/ && \
	cp $(srcdir)/bonobo-activation.png $(top_builddir)/api-docs/html

else

bonobo-activation.html:

endif



templates:
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)


clean-local:
	rm -f *~ *.bak *.signals *-unused.txt

maintainer-clean-local: clean
	cd $(top_builddir)/api-docs/ && rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(TARGET_DIR)
	(installfiles=`echo $(srcdir)/html/*.html`; \
	if test "$$installfiles" = '$(srcdir)/html/*.html'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  echo '-- Installing $(srcdir)/html/index.sgml' ; \
	  $(INSTALL_DATA) $(srcdir)/html/index.sgml $(DESTDIR)$(TARGET_DIR); \
	  echo '-- Fixing Crossreferences' ; \
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)|| true; \
	fi)

dist-hook: sgml html
	mkdir $(distdir)/html
	mkdir $(distdir)/tmpl
	-cp $(srcdir)/html/* $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl

.PHONY : html sgml templates scan


