/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_pt2pt_buffers.h"
#include "omb_pt2pt_execution.h"
#include "omb_pt2pt_multi.h"

static void print_metrics_for_message_size(omb_env_t *ctx)
{
    double metric_avg, message_rate;
    int i;

    metric_avg = RUN->metric_total_comm / options.iterations;

    if (BENCH->subtype != CONG_BW &&
        (TYPE_BANDWIDTH != BENCH->type || !BENCH->is_paired)) {
        metric_avg /= BENCH->pair_count;
    }

    if (BENCH->subtype == PART_LAT) {
        double latency = RUN->metric_total_rank / options.iterations;
        double timer = (RUN->t_timer_total * 1e6) / options.iterations;
        double comp = (RUN->t_comp_total * 1e6) / options.iterations;
        double wait = (RUN->t_wait_total * 1e6) / options.iterations;
        double test = (RUN->t_test_total * 1e6) / options.iterations;
        double init = (RUN->t_init_total * 1e6) / options.iterations;

        print_stats_nbc(ctx->rank, RUN->msg_bytes, timer, comp, latency,
                        latency, latency, wait, init, test, RUN->errors,
                        RUN->metric_stats);
    } else if (!ctx->rank && !RUN->tid) {
        fprintf(stdout, "%-*ld", 10, RUN->msg_bytes);

        if (options.window_varied) {
            for (i = 0; i < BENCH->window_sizes_count; i++) {
                fprintf(stdout, "%*.*f", TIGHT_FIELD_WIDTH, FLOAT_PRECISION,
                        RUN->varied_window_metric_total_comm[i] /
                            (options.iterations * BENCH->pair_count));
            }
        } else {
            fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION, metric_avg);

            if (options.print_rate) {
                message_rate =
                    (1e6 * metric_avg) / (options.omb_enable_ddt ?
                                              RUN->ddt_transmit_size :
                                              RUN->msg_bytes);
                fprintf(stdout, "%*.*f", FIELD_WIDTH, FLOAT_PRECISION,
                        message_rate);
            }

            if (options.validate) {
                fprintf(stdout, "%*s", FIELD_WIDTH,
                        VALIDATION_STATUS(RUN->errors));
            }

            if (options.omb_enable_ddt) {
                fprintf(stdout, "%*zu", FIELD_WIDTH, RUN->ddt_transmit_size);
            }
        }

        fprintf(stdout, "\n");
        fflush(stdout);
    }

    if (options.omb_tail_lat) {
        RUN->metric_stats =
            omb_calculate_tail_lat(BENCH->measurement_samples, ctx->rank, 1);
        OMB_ITR_PRINT_STAT(RUN->metric_stats.res_arr);
    }

    if (options.graph && 0 == ctx->rank) {
        BENCH->graph_data->avg = metric_avg;
    }
}

static inline void omb_touch_managed(omb_env_t *ctx)
{
#ifdef _ENABLE_CUDA_KERNEL_
    int i;

    if (options.src == 'M' && options.MMsrc == 'D' && options.dst == 'M' &&
        options.MMdst == 'D') {
        return;
    }

    if (options.src == 'M' && options.MMsrc == 'D') {
        if (options.buf_num == MULTIPLE) {
            for (i = 0; i < options.window_size; i++) {
                touch_managed(BENCH->multiple_recvbuf[i], RUN->msg_bytes, SUB);
            }
        } else {
            touch_managed(BENCH->recvbuf, RUN->msg_bytes, SUB);
        }
    } else if (options.dst == 'M' && options.MMdst == 'D') {
        if (options.buf_num == MULTIPLE) {
            for (i = 0; i < options.window_size; i++) {
                touch_managed(BENCH->multiple_recvbuf[i], RUN->msg_bytes, ADD);
            }
        } else {
            touch_managed(BENCH->recvbuf, RUN->msg_bytes, ADD);
        }
    }

    synchronize_stream();
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
}

static inline void default_iterations_loop(omb_env_t *ctx)
{
    for (size_t i = 0; i < options.iterations + options.skip; i++) {
        if (i == options.skip) {
            omb_papi_start(&(BENCH->papi_eventset));
        }

        if (options.validate) {
            omb_prepare_validation_buffers(ctx, i);
        }

        if (BENCH->is_sender) {
            omb_sender_core(ctx, i);
        } else {
            omb_receiver_core(ctx, i);
        }

        if (options.validate) {
            omb_touch_managed(ctx);
            omb_update_validation_errors(ctx, i);
        }
    }
}

static inline void partitioned_iteration_loops(omb_env_t *ctx)
{
    size_t i;

    default_iterations_loop(ctx);

    RUN->latency_in_secs = RUN->t_timer_total / (2.0 * options.iterations);
    RUN->t_timer_total = RUN->t_comp_total = RUN->t_wait_total =
        RUN->t_init_total = RUN->t_test_total = 0;

    for (i = 0; i < options.iterations + options.skip; i++) {
        if (i == options.skip) {
            omb_papi_start(&(BENCH->papi_eventset));
        }

        if (options.validate) {
            omb_prepare_validation_buffers(ctx, i);
        }

        partitioned_latency_overlap(ctx, i);

        if (options.validate) {
            omb_touch_managed(ctx);
            omb_update_validation_errors(ctx, i);
        }
    }
}

static void run_iterations_for_msg_size(omb_env_t *ctx)
{
    int reduced_errors;
    int window_array[] = WINDOW_SIZES;
    int i;

    for (i = 0; i < BENCH->window_sizes_count; i++) {
        RUN->t_timer_total = RUN->metric_total_rank = RUN->metric_total_comm =
            RUN->errors = 0;

        if (options.window_varied) {
            options.window_size = window_array[i];
        }

        if (BENCH->is_persistent) {
            init_persistent_pt2pt(ctx);
        }

        switch (BENCH->subtype) {
            case PART_LAT:
                partitioned_iteration_loops(ctx);
                break;
            default:
                default_iterations_loop(ctx);
                break;
        }

        if (BENCH->is_persistent) {
            free_persistent_pt2pt(ctx);
        }

        if (options.validate && !RUN->is_pthread) {
            MPI_CHECK(MPI_Allreduce(&RUN->errors, &reduced_errors, 1, MPI_INT,
                                    MPI_SUM, ctx->comm));
            RUN->errors = reduced_errors;
        }

        if (!RUN->is_pthread) {
            MPI_CHECK(MPI_Reduce(&RUN->metric_total_rank,
                                 &RUN->metric_total_comm, 1, MPI_DOUBLE,
                                 MPI_SUM, 0, ctx->comm));
        } else {
            RUN->metric_total_comm = RUN->metric_total_rank;
        }

        if (options.window_varied) {
            RUN->varied_window_metric_total_comm[i] = RUN->metric_total_comm;
        }
    }
}

void omb_run_benchmark_for_msg_size(omb_env_t *ctx)
{
    double avg_measurement;

    /* Do the benchmarking */
    run_iterations_for_msg_size(ctx);

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);

    print_metrics_for_message_size(ctx);

    omb_sync_inter_threads(ctx);
}
