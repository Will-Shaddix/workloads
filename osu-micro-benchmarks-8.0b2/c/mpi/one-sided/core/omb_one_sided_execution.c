/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_one_sided_core.h"
#include "omb_one_sided_buffers.h"
#include "osu_util.h"
#include <stdio.h>

extern void run_op_bw_with_lock(omb_env_t *ctx);
extern void run_op_bw_with_pscw(omb_env_t *ctx);
extern void run_op_bw_with_fence(omb_env_t *ctx);
#if _ENABLE_MPI3_
extern void run_op_bw_with_lock_all(omb_env_t *ctx);
extern void run_op_bw_with_flush_local(omb_env_t *ctx);
extern void run_op_bw_with_flush(omb_env_t *ctx);
#endif

extern void run_op_lat_with_lock(omb_env_t *ctx);
extern void run_op_lat_with_pscw(omb_env_t *ctx);
extern void run_op_lat_with_fence(omb_env_t *ctx);
#if _ENABLE_MPI3_
extern void run_op_lat_with_lock_all(omb_env_t *ctx);
extern void run_op_lat_with_flush_local(omb_env_t *ctx);
extern void run_op_lat_with_flush(omb_env_t *ctx);
#endif

extern void run_op_bibw_with_fence(omb_env_t *ctx);
extern void run_op_bibw_with_pscw(omb_env_t *ctx);

extern void run_op_acc_with_lock(omb_env_t *ctx);
extern void run_op_acc_with_pscw(omb_env_t *ctx);
extern void run_op_acc_with_fence(omb_env_t *ctx);
#if _ENABLE_MPI3_
extern void run_op_acc_with_lock_all(omb_env_t *ctx);
extern void run_op_acc_with_flush_local(omb_env_t *ctx);
extern void run_op_acc_with_flush(omb_env_t *ctx);
#endif

extern void run_op_cas_with_lock(omb_env_t *ctx);
extern void run_op_cas_with_pscw(omb_env_t *ctx);
extern void run_op_cas_with_fence(omb_env_t *ctx);
#if _ENABLE_MPI3_
extern void run_op_cas_with_lock_all(omb_env_t *ctx);
extern void run_op_cas_with_flush_local(omb_env_t *ctx);
extern void run_op_cas_with_flush(omb_env_t *ctx);
#endif

extern void run_op_fop_with_lock(omb_env_t *ctx);
extern void run_op_fop_with_pscw(omb_env_t *ctx);
extern void run_op_fop_with_fence(omb_env_t *ctx);
#if _ENABLE_MPI3_
extern void run_op_fop_with_lock_all(omb_env_t *ctx);
extern void run_op_fop_with_flush_local(omb_env_t *ctx);
extern void run_op_fop_with_flush(omb_env_t *ctx);
#endif

extern void run_op_get_acc_with_lock(omb_env_t *ctx);
extern void run_op_get_acc_with_pscw(omb_env_t *ctx);
extern void run_op_get_acc_with_fence(omb_env_t *ctx);
#if _ENABLE_MPI3_
extern void run_op_get_acc_with_lock_all(omb_env_t *ctx);
extern void run_op_get_acc_with_flush_local(omb_env_t *ctx);
extern void run_op_get_acc_with_flush(omb_env_t *ctx);
#endif

static void print_lat(omb_env_t *ctx)
{
    double tmp;

    if (ctx->rank == 0) {
        tmp =
            RUN->timer_sec * 1.0e6 * BENCH->latency_factor / options.iterations;
        fprintf(stdout, "%-*zu%*.*f", 10, RUN->msg_bytes, FIELD_WIDTH,
                FLOAT_PRECISION, tmp);
        if (options.omb_tail_lat) {
            OMB_ITR_PRINT_STAT(RUN->latency_stats.res_arr);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

static void print_bw(omb_env_t *ctx)
{
    double tmp;

    if (ctx->rank == 0) {
        tmp = RUN->msg_bytes / 1e6 * options.iterations * options.window_size;

        fprintf(stdout, "%-*zu%*.*f", 10, RUN->msg_bytes, FIELD_WIDTH,
                FLOAT_PRECISION, tmp / RUN->timer_sec);
        if (options.omb_tail_lat) {
            OMB_ITR_PRINT_STAT(RUN->latency_stats.res_arr);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

static void print_bi_bw(omb_env_t *ctx)
{
    if (0 == ctx->rank) {
        double tmp =
            RUN->msg_bytes / 1e6 * options.iterations * options.window_size;

        fprintf(stdout, "%-*zu%*.*f", 10, RUN->msg_bytes, FIELD_WIDTH,
                FLOAT_PRECISION, (tmp / RUN->timer_sec) * 2);
        if (options.omb_tail_lat) {
            OMB_ITR_PRINT_STAT(RUN->latency_stats.res_arr);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

static void print_latency_get_acc_lat(omb_env_t *ctx)
{
    if (ctx->rank == 0) {
        fprintf(stdout, "%-*zu%*.*f", 10, RUN->msg_bytes, FIELD_WIDTH,
                FLOAT_PRECISION,
                (RUN->timer_sec) * 1.0e6 / options.iterations *
                    BENCH->latency_factor);
        if (options.omb_tail_lat) {
            OMB_ITR_PRINT_STAT(RUN->latency_stats.res_arr);
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
}

static void print_atomic_validation_latency(omb_env_t *ctx)
{
    char *validation_string;

    if (0 != ctx->rank) {
        return;
    }

    if (options.validate) {
        if (2 & RUN->validation_error_flag) {
            validation_string = "skipped";
        } else if (1 & RUN->validation_error_flag) {
            validation_string = "failed";
        } else {
            validation_string = "passed";
        }

        fprintf(stdout, "%-*zu%*.*f%*s", 10, RUN->msg_bytes, FIELD_WIDTH,
                FLOAT_PRECISION,
                RUN->timer_sec * 1.0e6 * BENCH->latency_factor /
                    options.iterations,
                FIELD_WIDTH, validation_string);
    } else {
        fprintf(stdout, "%-*zu%*.*f", 10, RUN->msg_bytes, FIELD_WIDTH,
                FLOAT_PRECISION,
                RUN->timer_sec * 1.0e6 * BENCH->latency_factor /
                    options.iterations);
    }
    if (options.omb_tail_lat) {
        OMB_ITR_PRINT_STAT(RUN->latency_stats.res_arr);
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

void omb_plot_graphs(omb_env_t *ctx)
{
    if (0 == ctx->rank && options.graph) {
        BENCH->graph_data->avg =
            RUN->timer_sec * 1.0e6 * BENCH->latency_factor / options.iterations;
        omb_graph_plot(&(BENCH->graph_options), ctx->title);
    }
}

/* Benchmarks */

static void omb_run_one_sided_acc_lat(omb_env_t *ctx)
{
    switch (options.sync) {
        case LOCK:
            run_op_acc_with_lock(ctx);
            break;
        case PSCW:
            run_op_acc_with_pscw(ctx);
            break;
        case FENCE:
            run_op_acc_with_fence(ctx);
            break;
#if _ENABLE_MPI3_
        case LOCK_ALL:
            run_op_acc_with_lock_all(ctx);
            break;
        case FLUSH_LOCAL:
            run_op_acc_with_flush_local(ctx);
            break;
        case FLUSH:
            run_op_acc_with_flush(ctx);
            break;
#endif
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unknown sync_type %d\n", options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_atomic_validation_latency(ctx);
    RUN->validation_error_flag = 0;

    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_cas_lat(omb_env_t *ctx)
{
    switch (options.sync) {
        case LOCK:
            run_op_cas_with_lock(ctx);
            break;
        case PSCW:
            run_op_cas_with_pscw(ctx);
            break;
        case FENCE:
            run_op_cas_with_fence(ctx);
            break;
#if _ENABLE_MPI3_
        case LOCK_ALL:
            run_op_cas_with_lock_all(ctx);
            break;
        case FLUSH_LOCAL:
            run_op_cas_with_flush_local(ctx);
            break;
        case FLUSH:
            run_op_cas_with_flush(ctx);
            break;
#endif
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unknown sync_type %d\n", options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), 8);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_atomic_validation_latency(ctx);

    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_fop_lat(omb_env_t *ctx)
{
    switch (options.sync) {
        case LOCK:
            run_op_fop_with_lock(ctx);
            break;
        case PSCW:
            run_op_fop_with_pscw(ctx);
            break;
        case FENCE:
            run_op_fop_with_fence(ctx);
            break;
#if _ENABLE_MPI3_
        case LOCK_ALL:
            run_op_fop_with_lock_all(ctx);
            break;
        case FLUSH_LOCAL:
            run_op_fop_with_flush_local(ctx);
            break;
        case FLUSH:
            run_op_fop_with_flush(ctx);
            break;
#endif
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unknown sync_type %d\n", options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->dtype_size);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_atomic_validation_latency(ctx);

    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_get_acc_lat(omb_env_t *ctx)
{
    switch (options.sync) {
        case LOCK:
            run_op_get_acc_with_lock(ctx);
            break;
        case PSCW:
            run_op_get_acc_with_pscw(ctx);
            break;
        case FENCE:
            run_op_get_acc_with_fence(ctx);
            break;
#if _ENABLE_MPI3_
        case LOCK_ALL:
            run_op_get_acc_with_lock_all(ctx);
            break;
        case FLUSH_LOCAL:
            run_op_get_acc_with_flush_local(ctx);
            break;
        case FLUSH:
            run_op_get_acc_with_flush(ctx);
            break;
#endif
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unknown sync_type %d\n", options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->dtype_size);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_latency_get_acc_lat(ctx);

    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_bw(omb_env_t *ctx)
{
    switch (options.sync) {
        case LOCK:
            run_op_bw_with_lock(ctx);
            break;
        case PSCW:
            run_op_bw_with_pscw(ctx);
            break;
        case FENCE:
            run_op_bw_with_fence(ctx);
            break;
#if _ENABLE_MPI3_
        case LOCK_ALL:
            run_op_bw_with_lock_all(ctx);
            break;
        case FLUSH_LOCAL:
            run_op_bw_with_flush_local(ctx);
            break;
        case FLUSH:
            run_op_bw_with_flush(ctx);
            break;
#endif
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unknown sync_type %d\n", options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_bw(ctx);

    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_bi_bw(omb_env_t *ctx)
{
    switch (options.sync) {
        case FENCE:
            run_op_bibw_with_fence(ctx);
            break;
        case PSCW:
            run_op_bibw_with_pscw(ctx);
            break;
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unsuported sync_type %d\n",
                        options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_bi_bw(ctx);
    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_lat(omb_env_t *ctx)
{
    switch (options.sync) {
        case LOCK:
            run_op_lat_with_lock(ctx);
            break;
        case PSCW:
            run_op_lat_with_pscw(ctx);
            break;
        case FENCE:
            run_op_lat_with_fence(ctx);
            break;
#if _ENABLE_MPI3_
        case LOCK_ALL:
            run_op_lat_with_lock_all(ctx);
            break;
        case FLUSH_LOCAL:
            run_op_lat_with_flush_local(ctx);
            break;
        case FLUSH:
            run_op_lat_with_flush(ctx);
            break;
#endif
        default:
            if (!ctx->rank) {
                fprintf(stderr, "ERROR: Unknown sync_type %d\n", options.sync);
            }
            return;
    }

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);
    RUN->latency_stats =
        omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
    print_lat(ctx);

    omb_plot_graphs(ctx);
}

static void omb_run_one_sided_standard(omb_env_t *ctx)
{
    switch (BENCH->subtype) {
        case BW:
            omb_run_one_sided_bw(ctx);
            break;
        case BI_BW:
            omb_run_one_sided_bi_bw(ctx);
            break;
        case LAT:
            omb_run_one_sided_lat(ctx);
            break;
        default:
            if (0 == ctx->rank) {
                fprintf(stdout, "Error: Invalid subtype\n");
            }
    }
}

void omb_run_benchmark_for_msg_size(omb_env_t *ctx)
{
    switch (BENCH->benchmark_type) {
        case PUT:
        case GET:
            omb_run_one_sided_standard(ctx);
            break;
        case COMPARE_AND_SWAP:
            omb_run_one_sided_cas_lat(ctx);
            break;
        case FETCH_AND_OP:
            omb_run_one_sided_fop_lat(ctx);
            break;
        case ACCUMULATE:
            omb_run_one_sided_acc_lat(ctx);
            break;
        case GET_ACCUMULATE:
            omb_run_one_sided_get_acc_lat(ctx);
            break;
    }
}
