# --- Suite metadata (consumed by autogen.sh) ---
OMB_SUITE_NAME  = non_blocking
OMB_SUITE_PROGS = \
    osu_iallgather osu_ibarrier osu_ibcast osu_ialltoall    \
    osu_igather osu_iscatter osu_iscatterv osu_igatherv     \
    osu_iallgatherv osu_ialltoallv osu_ialltoallw           \
    osu_ireduce osu_iallreduce osu_ireduce_scatter          \
    osu_ireduce_scatter_block

# Optional meta-data: concatenates domain
OMB_SUITE_MACROS    = OMB_NONBLOCKING
OMB_SUITE_CFLAGS    =
OMB_SUITE_LIBS      = 
