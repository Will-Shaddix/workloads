/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "core/omb_one_sided_core.h"
#include "core/omb_one_sided_buffers.h"

#ifndef OMB_NAME
#error "OMB_NAME must be defined"
#endif

#ifndef OMB_SUBTYPE
#error "OMB_SUBTYPE must be defined"
#endif

#ifndef OMB_ENTRY_SYM
#error "OMB_ENTRY_SYM must be defined"
#endif

#ifndef OMB_MPI_OP
#error "OMB_MPI_OP must be defined"
#endif

void omb_run_benchmark_for_msg_size(omb_env_t *ctx);

int OMB_ENTRY_SYM(omb_env_t *ctx)
{
    int init_ret;

    /* malloc and initialize states */
    MALLOC_CHECK_AND_RET(BENCH, sizeof(omb_bench), ctx->rank);
    MALLOC_CHECK_AND_RET(RUN, sizeof(omb_run), ctx->rank);
    memset(BENCH, 0, sizeof(omb_bench));
    memset(RUN, 0, sizeof(omb_run));

    ctx->title = OMB_NAME;
    BENCH->domain = ONE_SIDED;
    BENCH->subtype = OMB_SUBTYPE;
    BENCH->header = HEADER;

    BENCH->benchmark_type = OMB_MPI_OP;

    OMB_CHECK_AND_RET(omb_bench_init(ctx));

    for (RUN->dtype_index = 0; RUN->dtype_index < options.omb_dtype_itr;
         RUN->dtype_index++) {
        omb_select_active_dtype(ctx);
        print_header_one_sided(ctx->rank, options.win, options.sync,
                               RUN->dtype);

        for (RUN->msg_bytes = options.min_message_size;
             RUN->msg_bytes <= options.max_message_size;
             RUN->msg_bytes *= options.message_size_incr) {
            if (omb_compute_iters_for_msg_size(ctx) == 0) {
                continue;
            }

            omb_allocate_one_sided_memory(ctx);

            if (COMPARE_AND_SWAP == BENCH->benchmark_type ||
                FETCH_AND_OP == BENCH->benchmark_type) {
                BENCH->graph_options.number_of_graphs = 0;
            }

            omb_graph_allocate_and_get_data_buffer(
                &(BENCH->graph_data), &(BENCH->graph_options), RUN->msg_bytes,
                options.iterations);

            omb_run_benchmark_for_msg_size(ctx);

            omb_free_one_sided_memory(ctx);
        }
    }

    omb_teardown_benchmark(ctx);

    return EXIT_SUCCESS;
}
