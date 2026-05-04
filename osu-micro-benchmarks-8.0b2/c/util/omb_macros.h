/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#define BENCH (ctx->omb_bench)
#define RUN   (ctx->omb_run)

#define NODE (ctx->node_info)
#define FAN  (ctx->omb_bench->fan_info)

#define OMB_PRINT_RANK_ZERO(_str)                                              \
    if (0 == ctx->rank) {                                                      \
        fprintf(stdout, _str);                                                 \
    }

#define OMB_ERR_PRINT_RANK_ZERO(_str)                                          \
    if (0 == ctx->rank) {                                                      \
        fprintf(stderr, _str);                                                 \
    }
