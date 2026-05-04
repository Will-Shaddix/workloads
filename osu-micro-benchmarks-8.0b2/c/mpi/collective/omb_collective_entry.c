/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_general_util.h"
#include "./core/omb_coll_core.h"

/*
 * Below pre-processor statements are concerned with macros from omb_*_defs.h
 * Macro blocks in omb_*_defs.h are used once per-test.
 * eg. -DOMB_NAME_osu_bcast will be passed only when building the bcast entry.
 */

#ifndef OMB_NAME
#error "OMB_NAME must be defined"
#endif
#ifndef OMB_SUBTYPE
#error "OMB_SUBTYPE must be defined"
#endif
#ifndef BENCHMARK
#error "BENCHMARK must be defined"
#endif
#ifndef OMB_MPI_CALL
#error "OMB_MPI_CALL must be defined"
#endif

#ifndef OMB_ENTRY_SYM
#error "OMB_ENTRY_SYM must be defined"
#endif

#ifndef OMB_NONBLOCKING
#define OMB_NONBLOCKING 0
#endif
#ifndef OMB_PERSISTENT
#define OMB_PERSISTENT 0
#endif
#ifndef OMB_NEIGHBORHOOD
#define OMB_NEIGHBORHOOD 0
#endif
#ifndef OMB_IRREGULAR_ARGS
#define OMB_IRREGULAR_ARGS 0
#endif
#ifndef OMB_IRREGULAR_BLOCK
#define OMB_IRREGULAR_BLOCK 0
#endif
#ifndef OMB_GENERALIZED_COLL
#define OMB_GENERALIZED_COLL 0
#endif

static void invoke_latency_step(omb_env_t *ctx)
{
    RUN->t_start = MPI_Wtime();
    MPI_CHECK(OMB_MPI_CALL);
    MPI_CHECK(OMB_MPI_WAIT);
    RUN->t_stop = MPI_Wtime();
}

static void invoke_overlap_step(omb_env_t *ctx)
{
    RUN->t_start = MPI_Wtime();
    RUN->init_time = MPI_Wtime();
    MPI_CHECK(OMB_MPI_CALL);
    RUN->init_time = MPI_Wtime() - RUN->init_time;

    RUN->compute_time_sec = MPI_Wtime();
    RUN->test_time = dummy_compute(RUN->latency_sec, &(BENCH->request));
    RUN->compute_time_sec = MPI_Wtime() - RUN->compute_time_sec;

    RUN->wait_time = MPI_Wtime();
    MPI_CHECK(MPI_Wait(&(BENCH->request), &(BENCH->status)));
    RUN->wait_time = MPI_Wtime() - RUN->wait_time;
    RUN->t_stop = MPI_Wtime();
}

static int invoke_coll_init(omb_env_t *ctx) { return OMB_MPI_COLL_INIT; }

void omb_run_benchmark_for_msg_size(omb_env_t *ctx);

int OMB_ENTRY_SYM(omb_env_t *ctx)
{
#ifdef OMB_DONT_BUILD_ENTRY
    ctx->title = OMB_NAME;
    if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
        fprintf(stderr, "\n%s: %s", OMB_NAME, OMB_DONT_BUILD_MSG);
    }
    not_launched_message = OMB_DONT_BUILD_MSG;
    return OMB_ERR_NOT_LAUNCHED;
#else

    int init_ret;

    /* zero initialize states */
    MALLOC_CHECK_AND_RET(BENCH, sizeof(omb_bench_t), ctx->rank);
    MALLOC_CHECK_AND_RET(RUN, sizeof(omb_run_t), ctx->rank);
    memset(BENCH, 0, sizeof(omb_bench_t));
    memset(RUN, 0, sizeof(omb_run_t));

    ctx->title = OMB_NAME;
    BENCH->domain = COLLECTIVE;
    BENCH->subtype = OMB_SUBTYPE;
    BENCH->header = HEADER;

    BENCH->mpi_coll_init = invoke_coll_init;
    BENCH->latency_step = invoke_latency_step;
    BENCH->overlap_step = invoke_overlap_step;

    BENCH->is_nonblocking = OMB_NONBLOCKING;
    BENCH->is_persistent = OMB_PERSISTENT;
    BENCH->is_neighborhood = OMB_NEIGHBORHOOD;
    BENCH->irregular_args = OMB_IRREGULAR_ARGS;
    BENCH->irregular_block = OMB_IRREGULAR_BLOCK;
    BENCH->is_generalized_coll = OMB_GENERALIZED_COLL;

    if ((init_ret = omb_bench_init(ctx)) != EXIT_SUCCESS) {
        return init_ret;
    }

    print_bench_preamble(ctx->rank, BENCH->is_nonblocking);

    for (RUN->dtype_index = 0; RUN->dtype_index < options.omb_dtype_itr;
         RUN->dtype_index++) {
        omb_select_active_dtype(ctx);
        omb_print_dtype_and_header(ctx);

        for (RUN->msg_bytes = options.min_message_size;
             RUN->msg_bytes <= options.max_message_size;
             RUN->msg_bytes *= options.message_size_incr) {
            if (omb_compute_iters_for_msg_size(ctx) == 0) {
                continue;
            }
            omb_prepare_graph_for_msg_size(ctx);
            omb_handle_ddt_size(ctx);
            omb_init_irregular_args(ctx);

            omb_run_benchmark_for_msg_size(ctx);

            if (options.graph && 0 == ctx->rank) {
                BENCH->graph_data->avg = RUN->comm_avg_latency_sec;
            }

            omb_ddt_append_stats(RUN->ddt_transmit_size);
            omb_ddt_free(&(RUN->dtype));

            MPI_CHECK(MPI_Barrier(ctx->comm));

            if (0 != RUN->errors) {
                break;
            }
        }
    }

    omb_plot_graphs(ctx);
    omb_teardown_benchmark(ctx);

    return omb_check_and_report_errors(ctx);
#endif
}
