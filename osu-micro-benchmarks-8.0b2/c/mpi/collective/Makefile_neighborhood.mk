# --- Suite metadata (consumed by autogen.sh) ---
OMB_SUITE_NAME  = neighborhood
OMB_SUITE_PROGS = \
    osu_neighbor_allgather osu_neighbor_allgatherv \
    osu_neighbor_alltoall osu_neighbor_alltoallv \
    osu_neighbor_alltoallw osu_ineighbor_allgather \
    osu_ineighbor_allgatherv osu_ineighbor_alltoall \
    osu_ineighbor_alltoallv osu_ineighbor_alltoallw

# Optional meta-data: concatenates domain
OMB_SUITE_MACROS    = OMB_NEIGHBORHOOD
OMB_SUITE_CFLAGS    =
OMB_SUITE_LIBS      = 
