# --- Domain metadata (consumed by autogen.sh) ---
OMB_DOMAIN_NAME      	= COLLECTIVE
OMB_DOMAIN_TEMPLATE		= $(top_srcdir)/c/mpi/collective/omb_collective_entry.c
OMB_DOMAIN_EXEC_MAIN	= $(top_srcdir)/c/mpi/mpi_exec_main.c

# Launcher
OMB_LAUNCHER_EXECUTABLE = omb_coll
OMB_LAUNCHER_TITLE = "OSU Micro Benchmarks - Collectives"

# Optional domain-wide compile options
OMB_DOMAIN_LIBS      = $(top_srcdir)/c/mpi/collective/core/libomb_collective_runtime.la
OMB_DOMAIN_MACROS    =
OMB_DOMAIN_CFLAGS    =
