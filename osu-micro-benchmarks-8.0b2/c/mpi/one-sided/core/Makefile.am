include $(top_srcdir)/c/mpi/Makefile_prologue.am

SUBDIRS = . sync

noinst_LTLIBRARIES = libomb_one_sided_runtime.la
libomb_one_sided_runtime_la_SOURCES = \
  $(srcdir)/omb_one_sided_buffers.c \
  $(srcdir)/omb_one_sided_core.c \
  $(srcdir)/omb_one_sided_execution.c \
  $(UTILITIES)

libomb_one_sided_runtime_la_LINK = $(LIBTOOL) --mode=link $(CCLD) -o $@ \
  $(libomb_one_sided_runtime_la_OBJECTS) $(libomb_one_sided_runtime_la_LDFLAGS)
