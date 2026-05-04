OMB_SYNC_LIB_NAME = libomb_one_sided_sync.la

OMB_SYNC_TEMPLATE = $(top_srcdir)/c/mpi/one-sided/core/sync/omb_one_sided_sync.c
OMB_SYNC_DEFS = $(top_srcdir)/c/mpi/one-sided/core/sync/omb_one_sided_sync_defs.h

OMB_SYNC_LIBADD = \
	$(top_srcdir)/c/mpi/one-sided/core/libomb_one_sided_runtime.la

OMB_SYNC_TYPES = \
	fence lock lock_all flush flush_local pscw

