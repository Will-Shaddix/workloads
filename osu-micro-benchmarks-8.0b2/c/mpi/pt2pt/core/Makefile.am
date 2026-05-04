include $(top_srcdir)/c/mpi/Makefile_prologue.am

noinst_LTLIBRARIES = libomb_pt2pt_runtime.la
libomb_pt2pt_runtime_la_SOURCES = \
  $(srcdir)/omb_pt2pt_buffers.c \
  $(srcdir)/omb_pt2pt_core.c \
  $(srcdir)/omb_pt2pt_iterations.c \
  $(srcdir)/omb_pt2pt_execution.c \
  $(srcdir)/omb_pt2pt_multi.c \
  $(srcdir)/omb_bw_fan_util.c \
  $(UTILITIES)

libomb_pt2pt_runtime_la_LINK = $(LIBTOOL) --mode=link $(CCLD) -o $@ \
  $(libomb_pt2pt_runtime_la_OBJECTS) $(libomb_pt2pt_runtime_la_LDFLAGS)
