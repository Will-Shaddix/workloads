/* Non-Blocking */

#ifdef OMB_NAME_osu_ineighbor_allgather
#define OMB_NAME    "osu_ineighbor_allgather"
#define OMB_SUBTYPE NBC_NHBR_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Ineighbor_allgather(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,  \
                            RUN->recvbuf_active, RUN->elem_count, RUN->dtype,  \
                            BENCH->dist_graph_comm, &(BENCH->request))
#define BENCHMARK       "OSU MPI%s Non-blocking Neighborhood Allgather Latency Test"
#define OMB_NONBLOCKING 1
#endif

#ifdef OMB_NAME_osu_ineighbor_allgatherv
#define OMB_NAME    "osu_ineighbor_allgatherv"
#define OMB_SUBTYPE NBC_NHBR_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Ineighbor_allgatherv(RUN->sendbuf_active, RUN->elem_count, RUN->dtype, \
                             RUN->recvbuf_active, BENCH->recv_counts,          \
                             BENCH->recv_displs, RUN->dtype,                   \
                             BENCH->dist_graph_comm, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Non-blocking Neighborhood Allgatherv Latency Test"
#define OMB_NONBLOCKING    1
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_ineighbor_alltoall
#define OMB_NAME    "osu_ineighbor_alltoall"
#define OMB_SUBTYPE NBC_NHBR_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Ineighbor_alltoall(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,   \
                           RUN->recvbuf_active, RUN->elem_count, RUN->dtype,   \
                           BENCH->dist_graph_comm, &(BENCH->request))
#define BENCHMARK       "OSU MPI%s Non-blocking Neighborhood All-to-All Latency Test"
#define OMB_NONBLOCKING 1
#endif

#ifdef OMB_NAME_osu_ineighbor_alltoallv
#define OMB_NAME    "osu_ineighbor_alltoallv"
#define OMB_SUBTYPE NBC_NHBR_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Ineighbor_alltoallv(RUN->sendbuf_active, BENCH->send_counts,           \
                            BENCH->send_displs, RUN->dtype,                    \
                            RUN->recvbuf_active, BENCH->recv_counts,           \
                            BENCH->recv_displs, RUN->dtype,                    \
                            BENCH->dist_graph_comm, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Non-blocking Neighborhood All-to-Allv Latency Test"
#define OMB_NONBLOCKING    1
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_ineighbor_alltoallw
#define OMB_NAME    "osu_ineighbor_alltoallw"
#define OMB_SUBTYPE NBC_NHBR_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Ineighbor_alltoallw(RUN->sendbuf_active, BENCH->send_counts,           \
                            BENCH->send_displs_bytes, BENCH->send_types,       \
                            RUN->recvbuf_active, BENCH->recv_counts,           \
                            BENCH->recv_displs_bytes, BENCH->recv_types,       \
                            BENCH->dist_graph_comm, &(BENCH->request))
#define BENCHMARK            "OSU MPI%s Non-blocking Neighborhood All-to-Allw Latency Test"
#define OMB_NONBLOCKING      1
#define OMB_IRREGULAR_ARGS   1
#define OMB_GENERALIZED_COLL 1
#endif

/* Blocking */

#ifdef OMB_NAME_osu_neighbor_allgather
#define OMB_NAME    "osu_neighbor_allgather"
#define OMB_SUBTYPE NHBR_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Neighbor_allgather(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,   \
                           BENCH->recvbuf, RUN->elem_count, RUN->dtype,        \
                           BENCH->dist_graph_comm)
#define BENCHMARK      "OSU MPI%s Neighborhood Allgather Latency Test"
#define OMB_BLOCK_TEST 1
#endif

#ifdef OMB_NAME_osu_neighbor_allgatherv
#define OMB_NAME    "osu_neighbor_allgatherv"
#define OMB_SUBTYPE NHBR_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Neighbor_allgatherv(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,  \
                            RUN->recvbuf_active, BENCH->recv_counts,           \
                            BENCH->recv_displs, RUN->dtype,                    \
                            BENCH->dist_graph_comm)
#define BENCHMARK          "OSU MPI%s Neighborhood Allgatherv Latency Test"
#define OMB_BLOCK_TEST     1
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_neighbor_alltoall
#define OMB_NAME    "osu_neighbor_alltoall"
#define OMB_SUBTYPE NHBR_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Neighbor_alltoall(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,    \
                          RUN->recvbuf_active, RUN->elem_count, RUN->dtype,    \
                          BENCH->dist_graph_comm)
#define BENCHMARK      "OSU MPI%s Neighborhood All-to-All Latency Test"
#define OMB_BLOCK_TEST 1
#endif

#ifdef OMB_NAME_osu_neighbor_alltoallv
#define OMB_NAME    "osu_neighbor_alltoallv"
#define OMB_SUBTYPE NHBR_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Neighbor_alltoallv(                                                    \
        RUN->sendbuf_active, BENCH->send_counts, BENCH->send_displs,           \
        RUN->dtype, RUN->recvbuf_active, BENCH->recv_counts,                   \
        BENCH->recv_displs, RUN->dtype, BENCH->dist_graph_comm)
#define BENCHMARK          "OSU MPI%s Neighborhood All-to-Allv Latency Test"
#define OMB_BLOCK_TEST     1
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_neighbor_alltoallw
#define OMB_NAME    "osu_neighbor_alltoallw"
#define OMB_SUBTYPE NHBR_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Neighbor_alltoallw(                                                    \
        RUN->sendbuf_active, BENCH->send_counts, BENCH->send_displs_bytes,     \
        BENCH->send_types, RUN->recvbuf_active, BENCH->recv_counts,            \
        BENCH->recv_displs_bytes, BENCH->recv_types, BENCH->dist_graph_comm)
#define BENCHMARK            "OSU MPI%s Neighborhood All-to-Allw Latency Test"
#define OMB_BLOCK_TEST       1
#define OMB_IRREGULAR_ARGS   1
#define OMB_GENERALIZED_COLL 1
#endif

#ifndef OMB_NAME
#error "Collective benchmark not recognized"
#endif

#define OMB_MPI_COLL_INIT MPI_SUCCESS
#ifdef OMB_NONBLOCKING
#define OMB_MPI_WAIT MPI_Wait(&(BENCH->request), &(BENCH->status))
#else
#define OMB_MPI_WAIT MPI_SUCCESS
#endif
