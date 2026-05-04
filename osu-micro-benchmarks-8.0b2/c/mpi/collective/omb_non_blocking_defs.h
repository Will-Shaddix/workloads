#ifdef OMB_NAME_osu_iallgather
#define OMB_NAME    "osu_iallgather"
#define OMB_SUBTYPE NBC_ALL_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Iallgather(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,           \
                   BENCH->recvbuf, RUN->elem_count, RUN->dtype, ctx->comm,     \
                   &(BENCH->request))
#define BENCHMARK "OSU MPI%s Non-blocking Allgather Latency Test"
#endif

#ifdef OMB_NAME_osu_iallgatherv
#define OMB_NAME    "osu_iallgatherv"
#define OMB_SUBTYPE NBC_ALL_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Iallgatherv(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,          \
                    BENCH->recvbuf, BENCH->recv_counts, BENCH->recv_displs,    \
                    RUN->dtype, ctx->comm, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Non-blocking Allgatherv Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_iallreduce
#define OMB_NAME    "osu_iallreduce"
#define OMB_SUBTYPE NBC_ALL_REDUCE
#define OMB_MPI_CALL                                                           \
    MPI_Iallreduce(RUN->sendbuf_active, BENCH->recvbuf, RUN->elem_count,       \
                   RUN->dtype, MPI_SUM, ctx->comm, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Non-blocking Allreduce Latency Test"
#endif

#ifdef OMB_NAME_osu_ialltoall
#define OMB_NAME    "osu_ialltoall"
#define OMB_SUBTYPE NBC_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Ialltoall(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,            \
                  BENCH->recvbuf, RUN->elem_count, RUN->dtype, ctx->comm,      \
                  &(BENCH->request))
#define BENCHMARK                                                              \
    "OSU MPI%s Non-blocking All-to-All Personalized Exchange"                  \
    "Latency Test"
#endif

#ifdef OMB_NAME_osu_ialltoallv
#define OMB_NAME    "osu_ialltoallv"
#define OMB_SUBTYPE NBC_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Ialltoallv(RUN->sendbuf_active, BENCH->send_counts,                    \
                   BENCH->send_displs, RUN->dtype, BENCH->recvbuf,             \
                   BENCH->recv_counts, BENCH->recv_displs, RUN->dtype,         \
                   ctx->comm, &(BENCH->request))
#define BENCHMARK                                                              \
    "OSU MPI%s Non-blocking All-to-Allv Personalized Exchange"                 \
    "Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_ialltoallw
#define OMB_NAME    "osu_ialltoallw"
#define OMB_SUBTYPE NBC_ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Ialltoallw(RUN->sendbuf_active, BENCH->send_counts,                    \
                   BENCH->send_displs, BENCH->send_types, BENCH->recvbuf,      \
                   BENCH->recv_counts, BENCH->recv_displs, BENCH->recv_types,  \
                   ctx->comm, &(BENCH->request))
#define BENCHMARK                                                              \
    "OSU MPI%s Non-blocking All-to-Allw Personalized Exchange"                 \
    "Latency Test"
#define OMB_IRREGULAR_ARGS   1
#define OMB_GENERALIZED_COLL 1
#endif

#ifdef OMB_NAME_osu_ibarrier
#define OMB_NAME     "osu_ibarrier"
#define OMB_SUBTYPE  NBC_BARRIER
#define OMB_MPI_CALL MPI_Ibarrier(ctx->comm, &(BENCH->request))
#define BENCHMARK    "OSU MPI%s Non-blocking Barrier Latency Test"
#endif

#ifdef OMB_NAME_osu_ibcast
#define OMB_NAME    "osu_ibcast"
#define OMB_SUBTYPE NBC_BCAST
#define OMB_MPI_CALL                                                           \
    MPI_Ibcast(BENCH->sendbuf, RUN->elem_count, RUN->dtype, 0, ctx->comm,      \
               &(BENCH->request))
#define BENCHMARK "OSU MPI%s Non-blocking Broadcast Latency Test"
#endif

#ifdef OMB_NAME_osu_igather
#define OMB_NAME    "osu_igather"
#define OMB_SUBTYPE NBC_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Igather(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,              \
                RUN->recvbuf_active, RUN->elem_count, RUN->dtype,              \
                RUN->root_rank, ctx->comm, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Non-blocking Gather Latency Test"
#endif

#ifdef OMB_NAME_osu_igatherv
#define OMB_NAME    "osu_igatherv"
#define OMB_SUBTYPE NBC_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Igatherv(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,             \
                 RUN->recvbuf_active, BENCH->recv_counts, BENCH->recv_displs,  \
                 RUN->dtype, RUN->root_rank, ctx->comm, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Non-blocking Gatherv Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_ireduce
#define OMB_NAME    "osu_ireduce"
#define OMB_SUBTYPE NBC_REDUCE
#define OMB_MPI_CALL                                                           \
    MPI_Ireduce(RUN->sendbuf_active, BENCH->recvbuf, RUN->elem_count,          \
                RUN->dtype, MPI_SUM, RUN->root_rank, ctx->comm,                \
                &(BENCH->request))
#define BENCHMARK "OSU MPI%s Non-blocking Reduce Latency Test"
#endif

#ifdef OMB_NAME_osu_ireduce_scatter
#define OMB_NAME    "osu_ireduce_scatter"
#define OMB_SUBTYPE NBC_REDUCE_SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Ireduce_scatter(RUN->sendbuf_active, BENCH->recvbuf,                   \
                        BENCH->recv_counts, RUN->dtype, MPI_SUM, ctx->comm,    \
                        &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Non-blocking Reduce_scatter Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_ireduce_scatter_block
#define OMB_NAME    "osu_ireduce_scatter_block"
#define OMB_SUBTYPE NBC_REDUCE_SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Ireduce_scatter_block(RUN->sendbuf_active, BENCH->recvbuf,             \
                              BENCH->portion, RUN->dtype, MPI_SUM, ctx->comm,  \
                              &(BENCH->request))
#define BENCHMARK           "OSU MPI%s Non-blocking Reduce_scatter_block Latency Test"
#define OMB_IRREGULAR_ARGS  1
#define OMB_IRREGULAR_BLOCK 1
#endif

#ifdef OMB_NAME_osu_iscatter
#define OMB_NAME    "osu_iscatter"
#define OMB_SUBTYPE NBC_SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Iscatter(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,             \
                 RUN->recvbuf_active, RUN->elem_count, RUN->dtype,             \
                 RUN->root_rank, ctx->comm, &(BENCH->request))
#define BENCHMARK "OSU MPI%s Non-blocking Scatter Latency Test"
#endif

#ifdef OMB_NAME_osu_iscatterv
#define OMB_NAME    "osu_iscatterv"
#define OMB_SUBTYPE NBC_SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Iscatterv(RUN->sendbuf_active, BENCH->send_counts, BENCH->send_displs, \
                  RUN->dtype, RUN->recvbuf_active, RUN->elem_count,            \
                  RUN->dtype, RUN->root_rank, ctx->comm, &(BENCH->request))
#define BENCHMARK          "OSU MPI%s Non-blocking Scatterv Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifndef OMB_NAME
#error "Blocking Collective benchmark not recognized"
#endif

#define OMB_MPI_COLL_INIT MPI_SUCCESS
#define OMB_MPI_WAIT      MPI_Wait(&(BENCH->request), &(BENCH->status))
