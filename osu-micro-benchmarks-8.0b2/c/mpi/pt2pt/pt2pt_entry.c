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
#include "core/omb_pt2pt_core.h"
#include "core/omb_pt2pt_buffers.h"
#include "core/omb_pt2pt_env.h"
#include "core/omb_pt2pt_multi.h"

#ifndef OMB_NAME
#error "OMB_NAME must be defined"
#endif

#ifndef OMB_SUBTYPE
#error "OMB_SUBTYPE must be defined"
#endif

#ifndef OMB_ENTRY_SYM
#error "OMB_ENTRY_SYM must be defined"
#endif

#ifndef OMB_PERSISTENT
#define OMB_PERSISTENT 0
#endif

#ifndef OMB_PAIRED
#define OMB_PAIRED 0
#endif

#ifndef OMB_FAN_OUT
#define OMB_FAN_OUT 0
#endif

int OMB_ENTRY_SYM(omb_env_t *ctx);
void omb_run_benchmark_for_msg_size(omb_env_t *ctx);

/* Creates thread-private run context and runs OMB_ENTRY_SYM per-thread */
static void *thread_bouncer(void *arg)
{
    omb_env_t t_ctx;
    omb_bench_t t_bench;
    omb_run_t t_run;
    omb_env_tid_t *env_and_tid = (omb_env_tid_t *)arg;

    /* Copy omb_env and omb_bench */
    t_ctx = *env_and_tid->omb_env;
    t_bench = *t_ctx.omb_bench;

    /* New thread-local env pointers */
    t_ctx.omb_bench = &t_bench;
    t_ctx.omb_run = &t_run;

    /* Mark as thread */
    t_ctx.omb_run->is_pthread = true;
    t_ctx.omb_run->tid = env_and_tid->id;

    /* Alloc run and free */
    omb_alloc_benchmark_buffers(&t_ctx);
    OMB_ENTRY_SYM(&t_ctx);
    omb_free_benchmark_buffers(&t_ctx);

    return NULL;
}

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

    if (!RUN || !RUN->is_pthread) {
        /* malloc and initialize states */
        MALLOC_CHECK_AND_RET(BENCH, sizeof(omb_bench_t), ctx->rank);
        MALLOC_CHECK_AND_RET(RUN, sizeof(omb_run_t), ctx->rank);
        memset(BENCH, 0, sizeof(omb_bench_t));
        memset(RUN, 0, sizeof(omb_run_t));

        ctx->title = OMB_NAME;
        BENCH->domain = PT2PT;
        BENCH->subtype = OMB_SUBTYPE;
        BENCH->header = HEADER;

        BENCH->is_persistent = OMB_PERSISTENT;
        BENCH->is_paired = OMB_PAIRED;
        BENCH->is_fan_out = OMB_FAN_OUT;

        if ((init_ret = omb_bench_init(ctx)) != EXIT_SUCCESS) {
            return init_ret;
        }

        print_preamble(ctx->rank);

        if (BENCH->is_paired) {
            omb_print_pair_info(ctx);
        }
        if (BENCH->subtype == PART_LAT && !ctx->rank) {
            fprintf(stdout, "# Partitions: %i\t\n", options.num_partitions);
        }

        if (options.subtype == LAT_MP) {
            omb_handle_multiprocess(ctx);
        } else if (options.subtype == LAT_MT) {
            OMB_CHECK_AND_RET(omb_handle_multithread(ctx, thread_bouncer));
            omb_plot_graphs(ctx);
            omb_teardown_benchmark(ctx);
            return omb_check_and_report_errors(ctx);
        }
    }

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

            if (BENCH->type == TYPE_LATENCY &&
                RUN->msg_bytes >= LARGE_MESSAGE_SIZE) {
                options.iterations = options.iterations_large;
                options.skip = options.skip_large;
            }

            omb_handle_ddt_size(ctx);
            omb_prepare_graph_for_msg_size(ctx);
            if (options.buf_num == MULTIPLE) {
                OMB_CHECK_AND_RET(omb_alloc_multiple_buffers(ctx));
            }
            omb_set_pt2pt_buffers(ctx);

            omb_sync_inter_threads(ctx);

            omb_measure_kernel_lo(ctx);
            omb_run_benchmark_for_msg_size(ctx);

            if (options.buf_num == MULTIPLE) {
                omb_free_multiple_buffers(ctx);
            }

            omb_ddt_free(&(RUN->dtype));

            if (options.validate && RUN->errors) {
                break;
            }
        }
    }

    if (!RUN->is_pthread) {
        omb_plot_graphs(ctx);
        omb_teardown_benchmark(ctx);
    }

    return omb_check_and_report_errors(ctx);
#endif
}
