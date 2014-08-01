libdir = $(prefix)/lib/robotkernel/modules
lib_LTLIBRARIES = libmodule_tty.la

libmodule_tty_la_SOURCES  = module_tty.cpp tty.cpp
libmodule_tty_la_CXXFLAGS = -Wall -I$(top_srcdir)/src -I$(top_builddir) @YAMLCPP_CFLAGS@ @LN_CFLAGS@
libmodule_tty_la_LDFLAGS  = -Wall @PTHREAD_LIBS@ @RT_LIBS@  -release $(PACKAGE_VERSION)

include $(top_srcdir)/Makefile.defs

