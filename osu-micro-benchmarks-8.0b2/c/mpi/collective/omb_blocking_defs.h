#ifdef OMB_NAME_osu_allgather
#define OMB_NAME    "osu_allgather"
#define OMB_SUBTYPE ALL_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Allgather(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,            \
                  BENCH->recvbuf, RUN->elem_count, RUN->dtype, ctx->comm)
#define BENCHMARK "OSU MPI%s Allgather Latency Test"
#endif

#ifdef OMB_NAME_osu_allgatherv
#define OMB_NAME    "osu_allgatherv"
#define OMB_SUBTYPE ALL_GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Allgatherv(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,           \
                   BENCH->recvbuf, BENCH->recv_counts, BENCH->recv_displs,     \
                   RUN->dtype, ctx->comm)
#define BENCHMARK          "OSU MPI%s Allgatherv Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_allreduce
#define OMB_NAME    "osu_allreduce"
#define OMB_SUBTYPE ALL_REDUCE
#define OMB_MPI_CALL                                                           \
    MPI_Allreduce(RUN->sendbuf_active, BENCH->recvbuf, RUN->elem_count,        \
                  RUN->dtype, MPI_SUM, ctx->comm)
#define BENCHMARK "OSU MPI%s Allreduce Latency Test"
#endif

#ifdef OMB_NAME_osu_alltoall
#define OMB_NAME    "osu_alltoall"
#define OMB_SUBTYPE ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Alltoall(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,             \
                 BENCH->recvbuf, RUN->elem_count, RUN->dtype, ctx->comm)
#define BENCHMARK "OSU MPI%s All-to-All Personalized Exchange Latency Test"
#endif

#ifdef OMB_NAME_osu_alltoallv
#define OMB_NAME    "osu_alltoallv"
#define OMB_SUBTYPE ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Alltoallv(RUN->sendbuf_active, BENCH->send_counts, BENCH->send_displs, \
                  RUN->dtype, BENCH->recvbuf, BENCH->recv_counts,              \
                  BENCH->recv_displs, RUN->dtype, ctx->comm)
#define BENCHMARK          "OSU MPI%s All-to-Allv Personalized Exchange Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_alltoallw
#define OMB_NAME    "osu_alltoallw"
#define OMB_SUBTYPE ALLTOALL
#define OMB_MPI_CALL                                                           \
    MPI_Alltoallw(RUN->sendbuf_active, BENCH->send_counts, BENCH->send_displs, \
                  BENCH->send_types, BENCH->recvbuf, BENCH->recv_counts,       \
                  BENCH->recv_displs, BENCH->recv_types, ctx->comm)
#define BENCHMARK            "OSU MPI%s All-to-Allw Personalized Exchange Latency Test"
#define OMB_IRREGULAR_ARGS   1
#define OMB_GENERALIZED_COLL 1
#endif

#ifdef OMB_NAME_osu_barrier
#define OMB_NAME     "osu_barrier"
#define OMB_SUBTYPE  BARRIER
#define OMB_MPI_CALL MPI_Barrier(ctx->comm)
#define BENCHMARK    "OSU MPI%s Barrier Latency Test"
#endif

#ifdef OMB_NAME_osu_bcast
#define OMB_NAME    "osu_bcast"
#define OMB_SUBTYPE BCAST
#define OMB_MPI_CALL                                                           \
    MPI_Bcast(BENCH->sendbuf, RUN->elem_count, RUN->dtype, 0, ctx->comm)
#define BENCHMARK "OSU MPI%s Broadcast Latency Test"
#endif

#ifdef OMB_NAME_osu_gather
#define OMB_NAME    "osu_gather"
#define OMB_SUBTYPE GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Gather(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,               \
               RUN->recvbuf_active, RUN->elem_count, RUN->dtype,               \
               RUN->root_rank, ctx->comm)
#define BENCHMARK "OSU MPI%s Gather Latency Test"
#endif

#ifdef OMB_NAME_osu_gatherv
#define OMB_NAME    "osu_gatherv"
#define OMB_SUBTYPE GATHER
#define OMB_MPI_CALL                                                           \
    MPI_Gatherv(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,              \
                RUN->recvbuf_active, BENCH->recv_counts, BENCH->recv_displs,   \
                RUN->dtype, RUN->root_rank, ctx->comm)
#define BENCHMARK          "OSU MPI%s Gatherv Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_reduce
#define OMB_NAME    "osu_reduce"
#define OMB_SUBTYPE REDUCE
#define OMB_MPI_CALL                                                           \
    MPI_Reduce(RUN->sendbuf_active, BENCH->recvbuf, RUN->elem_count,           \
               RUN->dtype, MPI_SUM, RUN->root_rank, ctx->comm)
#define BENCHMARK "OSU MPI%s Reduce Latency Test"
#endif

#ifdef OMB_NAME_osu_reduce_scatter
#define OMB_NAME    "osu_reduce_scatter"
#define OMB_SUBTYPE REDUCE_SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Reduce_scatter(RUN->sendbuf_active, BENCH->recvbuf,                    \
                       BENCH->recv_counts, RUN->dtype, MPI_SUM, ctx->comm)
#define BENCHMARK          "OSU MPI%s Reduce_scatter Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifdef OMB_NAME_osu_reduce_scatter_block
#define OMB_NAME    "osu_reduce_scatter_block"
#define OMB_SUBTYPE REDUCE_SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Reduce_scatter_block(RUN->sendbuf_active, BENCH->recvbuf,              \
                             BENCH->portion, RUN->dtype, MPI_SUM, ctx->comm)
#define BENCHMARK           "OSU MPI%s Reduce_scatter_block Latency Test"
#define OMB_IRREGULAR_ARGS  1
#define OMB_IRREGULAR_BLOCK 1
#endif

#ifdef OMB_NAME_osu_scatter
#define OMB_NAME    "osu_scatter"
#define OMB_SUBTYPE SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Scatter(RUN->sendbuf_active, RUN->elem_count, RUN->dtype,              \
                RUN->recvbuf_active, RUN->elem_count, RUN->dtype,              \
                RUN->root_rank, ctx->comm)
#define BENCHMARK "OSU MPI%s Scatter Latency Test"
#endif

#ifdef OMB_NAME_osu_scatterv
#define OMB_NAME    "osu_scatterv"
#define OMB_SUBTYPE SCATTER
#define OMB_MPI_CALL                                                           \
    MPI_Scatterv(RUN->sendbuf_active, BENCH->send_counts, BENCH->send_displs,  \
                 RUN->dtype, RUN->recvbuf_active, RUN->elem_count, RUN->dtype, \
                 RUN->root_rank, ctx->comm)
#define BENCHMARK          "OSU MPI%s Scatterv Latency Test"
#define OMB_IRREGULAR_ARGS 1
#endif

#ifndef OMB_NAME
#error "Blocking Collective benchmark not recognized"
#endif

#define OMB_MPI_COLL_INIT MPI_SUCCESS
#define OMB_MPI_WAIT      MPI_SUCCESS
