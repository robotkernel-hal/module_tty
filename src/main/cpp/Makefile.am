lib_LTLIBRARIES = libmodule_tty.la

libmodule_tty_la_SOURCES  = tty.cpp
libmodule_tty_la_CXXFLAGS = -Wall -I$(top_srcdir)/src -I$(top_builddir) \
							@ROBOTKERNEL_CFLAGS@ \
							@LIBSTRING_UTIL_CFLAGS@ \
							@YAML_CPP_RMC_CFLAGS@

libmodule_tty_la_LDFLAGS  = -Wall -release $(PACKAGE_VERSION)

