libdir = $(hexchatlibdir)

lib_LTLIBRARIES = sysinfo.la
sysinfo_la_SOURCES = sysinfo.c unix/sysinfo-backend-unix.c unix/match.c unix/parse.c unix/pci.c
sysinfo_la_LDFLAGS = $(PLUGIN_LDFLAGS) -module
sysinfo_la_LIBADD = $(LIBPCI_LIBS) $(GLIB_LIBS)
sysinfo_la_CFLAGS = $(LIBPCI_CFLAGS) $(GLIB_CFLAGS) -I$(srcdir)/../../src/common
