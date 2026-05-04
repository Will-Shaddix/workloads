# --- Domain metadata (consumed by autogen.sh) ---
OMB_DOMAIN_NAME      	= ONE_SIDED
OMB_DOMAIN_TEMPLATE		= $(top_srcdir)/c/mpi/one-sided/one_sided_entry.c
OMB_DOMAIN_EXEC_MAIN	= $(top_srcdir)/c/mpi/mpi_exec_main.c

# Launcher
OMB_LAUNCHER_EXECUTABLE = omb_one_sided
OMB_LAUNCHER_TITLE = "OSU Micro Benchmarks - One Sided"

# Optional domain-wide compile options
OMB_DOMAIN_LIBS     = \
	$(top_srcdir)/c/mpi/one-sided/core/libomb_one_sided_runtime.la \
	$(top_srcdir)/c/mpi/one-sided/core/sync/libomb_one_sided_sync.la
OMB_DOMAIN_MACROS	=
OMB_DOMAIN_CFLAGS	=
