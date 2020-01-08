ACLOCAL_AMFLAGS = -I m4 
SUBDIRS=src/main/cpp

install-exec-hook:
	mkdir -p $(DESTDIR)$(prefix)/doc
	cp -ru $(top_builddir)/doc/* $(DESTDIR)$(prefix)/doc

