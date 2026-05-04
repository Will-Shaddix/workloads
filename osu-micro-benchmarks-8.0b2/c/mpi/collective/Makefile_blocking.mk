# --- Suite metadata (consumed by autogen.sh) ---
OMB_SUITE_NAME  = blocking
OMB_SUITE_PROGS = \
    osu_allgather osu_allgatherv osu_allreduce osu_alltoall osu_alltoallv \
    osu_alltoallw osu_barrier osu_bcast osu_gather osu_gatherv osu_reduce \
    osu_reduce_scatter osu_reduce_scatter_block osu_scatter osu_scatterv

# Optional meta-data: concatenates domain
OMB_SUITE_MACROS    = OMB_BLOCK_TEST
OMB_SUITE_CFLAGS    =
OMB_SUITE_LIBS      = 
