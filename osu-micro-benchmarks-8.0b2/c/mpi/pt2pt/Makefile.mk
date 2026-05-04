# --- Domain metadata (consumed by autogen.sh) ---
OMB_DOMAIN_NAME      	= PT2PT
OMB_DOMAIN_TEMPLATE		= $(top_srcdir)/c/mpi/pt2pt/pt2pt_entry.c
OMB_DOMAIN_EXEC_MAIN	= $(top_srcdir)/c/mpi/mpi_exec_main.c

# Launcher
OMB_LAUNCHER_EXECUTABLE = omb_pt2pt
OMB_LAUNCHER_TITLE = "OSU Micro Benchmarks - Point to Point"

# Optional domain-wide compile options
OMB_DOMAIN_LIBS      = $(top_srcdir)/c/mpi/pt2pt/core/libomb_pt2pt_runtime.la
OMB_DOMAIN_MACROS    =
OMB_DOMAIN_CFLAGS    =
