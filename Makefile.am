ACLOCAL_AMFLAGS = -I m4 

libdir = $(prefix)/lib
lib_LTLIBRARIES = libmodule_tty.la

libmodule_tty_la_SOURCES  = module_tty.cpp tty.cpp
libmodule_tty_la_CXXFLAGS = -Wall -I$(top_srcdir)/src -I$(top_builddir) @ROBOTKERNEL_CFLAGS@ @YAMLCPP_CFLAGS@ @LN_CFLAGS@
libmodule_tty_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)
