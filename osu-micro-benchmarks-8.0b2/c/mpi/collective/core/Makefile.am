include $(top_srcdir)/c/mpi/Makefile_prologue.am

noinst_LTLIBRARIES = libomb_collective_runtime.la
libomb_collective_runtime_la_SOURCES = \
  $(srcdir)/omb_coll_buffers.c \
  $(srcdir)/omb_coll_core.c \
  $(srcdir)/omb_coll_iterations.c \
  $(UTILITIES)

libomb_collective_runtime_la_LINK = $(LIBTOOL) --mode=link $(CCLD) -o $@ \
  $(libomb_collective_runtime_la_OBJECTS) $(libomb_collective_runtime_la_LDFLAGS)
