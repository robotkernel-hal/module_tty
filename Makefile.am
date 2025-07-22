ACLOCAL_AMFLAGS = -I m4 
SUBDIRS=src

install-exec-hook:
	mkdir -p $(DESTDIR)$(prefix)/doc
	cp -ru $(top_builddir)/doc/* $(DESTDIR)$(prefix)/doc

