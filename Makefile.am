ACLOCAL_AMFLAGS = -I m4 
SUBDIRS=src

install-exec-hook:
	mkdir -p $(DESTDIR)$(prefix)/doc
	cp -ru $(top_srcdir)/doc/* $(DESTDIR)$(prefix)/doc

