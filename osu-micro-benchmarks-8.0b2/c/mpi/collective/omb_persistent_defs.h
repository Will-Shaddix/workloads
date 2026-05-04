#ifdef OMB_NAME_osu_allgather_persistent
#define OMB_NAME    "osu_allgather_persistent"
#define OMB_SUBTYPE ALL_GATHER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Allgather_init(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,       \
                       BENCH->recvbuf, RUN->elem_count, RUN->dtype, ctx->comm, \
                       MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Allgather Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_allgatherv_persistent
#define OMB_NAME    "osu_allgatherv_persistent"
#define OMB_SUBTYPE ALL_GATHER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Allgatherv_init(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,      \
                        BENCH->recvbuf, BENCH->recv_counts,                    \
                        BENCH->recv_displs, RUN->dtype, ctx->comm,             \
                        MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Allgatherv Persistent Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_allreduce_persistent
#define OMB_NAME    "osu_allreduce_persistent"
#define OMB_SUBTYPE ALL_REDUCE_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Allreduce_init(RUN->sendbuf_active, BENCH->recvbuf, RUN->elem_count,   \
                       RUN->dtype, MPI_SUM, ctx->comm, MPI_INFO_NULL,          \
                       &(BENCH->request))
#define BENCHMARK "OSU MPI%s Allreduce Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_alltoall_persistent
#define OMB_NAME    "osu_alltoall_persistent"
#define OMB_SUBTYPE ALLTOALL_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Alltoall_init(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,        \
                      BENCH->recvbuf, RUN->elem_count, RUN->dtype, ctx->comm,  \
                      MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK                                                              \
    "OSU MPI%s All-to-All Personalized Exchange Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_alltoallv_persistent
#define OMB_NAME    "osu_alltoallv_persistent"
#define OMB_SUBTYPE ALLTOALL_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Alltoallv_init(RUN->sendbuf_active, BENCH->send_counts,                \
                       BENCH->send_displs, RUN->dtype, BENCH->recvbuf,         \
                       BENCH->recv_counts, BENCH->recv_displs, RUN->dtype,     \
                       ctx->comm, MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK                                                              \
    "OSU MPI%s All-to-Allv Personalized Exchange Persistent Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_alltoallw_persistent
#define OMB_NAME    "osu_alltoallw_persistent"
#define OMB_SUBTYPE ALLTOALL_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Alltoallw_init(RUN->sendbuf_active, BENCH->send_counts,                \
                       BENCH->send_displs, BENCH->send_types, BENCH->recvbuf,  \
                       BENCH->recv_counts, BENCH->recv_displs,                 \
                       BENCH->recv_types, ctx->comm, MPI_INFO_NULL,            \
                       &(BENCH->request))
#define BENCHMARK                                                              \
    "OSU MPI%s All-to-Allw Personalized Exchange Persistent Latency Test"
#define OMB_IRREGULAR_ARGS   1
#define OMB_GENERALIZED_COLL 1
#endif

#ifdef OMB_NAME_osu_barrier_persistent
#define OMB_NAME    "osu_barrier_persistent"
#define OMB_SUBTYPE BARRIER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Barrier_init(ctx->comm, MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Barrier Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_bcast_persistent
#define OMB_NAME    "osu_bcast_persistent"
#define OMB_SUBTYPE BCAST_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Bcast_init(BENCH->sendbuf, RUN->elem_count, RUN->dtype, 0, ctx->comm,  \
                   MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Broadcast Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_gather_persistent
#define OMB_NAME    "osu_gather_persistent"
#define OMB_SUBTYPE GATHER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Gather_init(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,          \
                    RUN->recvbuf_active, RUN->elem_count, RUN->dtype,          \
                    RUN->root_rank, ctx->comm, MPI_INFO_NULL,                  \
                    &(BENCH->request))
#define BENCHMARK "OSU MPI%s Gather Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_gatherv_persistent
#define OMB_NAME    "osu_gatherv_persistent"
#define OMB_SUBTYPE GATHER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Gatherv_init(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,         \
                     RUN->recvbuf_active, BENCH->recv_counts,                  \
                     BENCH->recv_displs, RUN->dtype, RUN->root_rank,           \
                     ctx->comm, MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Gatherv Persistent Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_reduce_persistent
#define OMB_NAME    "osu_reduce_persistent"
#define OMB_SUBTYPE REDUCE_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Reduce_init(RUN->sendbuf_active, BENCH->recvbuf, RUN->elem_count,      \
                    RUN->dtype, MPI_SUM, RUN->root_rank, ctx->comm,            \
                    MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Reduce Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_reduce_scatter_persistent
#define OMB_NAME    "osu_reduce_scatter_persistent"
#define OMB_SUBTYPE REDUCE_SCATTER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Reduce_scatter_init(RUN->sendbuf_active, BENCH->recvbuf,               \
                            BENCH->recv_counts, RUN->dtype, MPI_SUM,           \
                            ctx->comm, MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Reduce_scatter Persistent Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_reduce_scatter_block_persistent
#define OMB_NAME    "osu_reduce_scatter_block_persistent"
#define OMB_SUBTYPE REDUCE_SCATTER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Reduce_scatter_block(RUN->sendbuf_active, BENCH->recvbuf,              \
                             BENCH->portion, RUN->dtype, MPI_SUM, ctx->comm)
#define BENCHMARK           "OSU MPI%s Reduce_scatter_block Persistent Latency Test"
#define OMB_IRREGULAR_ARGS  1
#define OMB_IRREGULAR_BLOCK 1
#endif

#ifdef OMB_NAME_osu_scatter_persistent
#define OMB_NAME    "osu_scatter_persistent"
#define OMB_SUBTYPE SCATTER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Scatter_init(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,         \
                     RUN->recvbuf_active, RUN->elem_count, RUN->dtype,         \
                     RUN->root_rank, ctx->comm, MPI_INFO_NULL,                 \
                     &(BENCH->request))
#define BENCHMARK "OSU MPI%s Scatter Persistent Latency Test"
#endif

#ifdef OMB_NAME_osu_scatterv_persistent
#define OMB_NAME    "osu_scatterv_persistent"
#define OMB_SUBTYPE SCATTER_P
#define OMB_MPI_COLL_INIT                                                      \
    MPI_Scatterv_init(RUN->sendbuf_active, BENCH->send_counts,                 \
                      BENCH->send_displs, RUN->dtype, RUN->recvbuf_active,     \
                      RUN->elem_count, RUN->dtype, RUN->root_rank, ctx->comm,  \
                      MPI_INFO_NULL, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Scatterv Persistent Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifndef OMB_NAME
#error "Blocking Collective benchmark not recognized"
#endif

#define OMB_MPI_CALL MPI_Start(&(BENCH->request))
#define OMB_MPI_WAIT MPI_Wait(&(BENCH->request), &(BENCH->status))

/* Suite-wide disablement if persistents aren't supported */
#ifndef _ENABLE_MPI4_
#define OMB_DONT_BUILD_ENTRY 1
#define OMB_DONT_BUILD_MSG   "All persistent tests require _ENABLE_MPI4_\n"
#undef OMB_MPI_COLL_INIT
#define OMB_MPI_COLL_INIT 0
#endif
