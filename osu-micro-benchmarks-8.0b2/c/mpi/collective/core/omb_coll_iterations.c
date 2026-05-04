/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_coll_env.h"
#include "omb_coll_buffers.h"

static void run_validation_warmups(omb_env_t *ctx, int iteration)
{
    for (size_t j = 0; j < options.warmup_validation; j++) {
        BENCH->latency_step(ctx);
    }

    omb_prepare_validation_buffers(ctx, iteration);
    MPI_CHECK(MPI_Barrier(ctx->comm));
}

static void run_iterations_core(omb_env_t *ctx, omb_env_fn body_fn,
                                omb_env_fn sum_fn)
{
    double latency_us = 0;
    RUN->elapsed_sec = RUN->compute_time_total_sec = RUN->test_total_sec =
        RUN->init_total_sec = RUN->wait_total_sec = 0;

    OMB_START_DYNAMIC_TUNING;
    for (size_t i = 0; i < options.iterations + options.skip; i++) {
        if (i == options.skip) {
            /* pause dynamic tuning so it does not impact performance */
            OMB_PAUSE_DYNAMIC_TUNING;
            omb_papi_start(&(BENCH->papi_eventset));
        }

        RUN->root_rank = omb_get_root_rank(i, ctx->numprocs);

        if (options.validate) {
            run_validation_warmups(ctx, i);
        }

        if (options.omb_enable_mpi_in_place == 1) {
            omb_apply_mpi_in_place(ctx);
        }

        body_fn(ctx);

        MPI_CHECK(MPI_Barrier(ctx->comm));
        if (options.validate) {
            omb_accumulate_local_errors(ctx, i);
        }

        if (i >= options.skip) {
            sum_fn(ctx);

            latency_us = (RUN->t_stop - RUN->t_start) * 1e6;
            if (options.omb_tail_lat) {
                BENCH->latency_us_samples[i - options.skip] = latency_us;
            }
            if (options.graph && 0 == ctx->rank) {
                BENCH->graph_data->data[i - options.skip] = latency_us;
            }
        }
    }
}

static void accumulate_elapsed_time(omb_env_t *ctx)
{
    RUN->elapsed_sec += RUN->t_stop - RUN->t_start;
}

static void run_blocking_for_msg_size(omb_env_t *ctx)
{
    double max_latency_us, min_latency_us;

    if (1 == options.omb_enable_mpi_in_place) {
        omb_apply_mpi_in_place(ctx);
    }

    if (BENCH->is_persistent) {
        BENCH->mpi_coll_init(ctx);
        MPI_CHECK(MPI_Barrier(ctx->comm));
    }

    /* Do the benchmarking */
    run_iterations_core(ctx, BENCH->latency_step, accumulate_elapsed_time);

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);

    RUN->avg_latency_us = (RUN->elapsed_sec * 1e6) / options.iterations;

    MPI_CHECK(MPI_Reduce(&(RUN->avg_latency_us), &min_latency_us, 1, MPI_DOUBLE,
                         MPI_MIN, 0, ctx->comm));
    MPI_CHECK(MPI_Reduce(&(RUN->avg_latency_us), &max_latency_us, 1, MPI_DOUBLE,
                         MPI_MAX, 0, ctx->comm));
    MPI_CHECK(MPI_Reduce(&(RUN->avg_latency_us), &(RUN->comm_avg_latency_sec),
                         1, MPI_DOUBLE, MPI_SUM, 0, ctx->comm));
    RUN->comm_avg_latency_sec = RUN->comm_avg_latency_sec / ctx->numprocs;

    RUN->latency_stats = omb_get_stats(BENCH->latency_us_samples);

    if (options.validate) {
        MPI_CHECK(MPI_Allreduce(&(RUN->local_errors), &(RUN->errors), 1,
                                MPI_INT, MPI_SUM, ctx->comm));
        print_stats_validate(ctx->rank, RUN->msg_bytes,
                             RUN->comm_avg_latency_sec, min_latency_us,
                             max_latency_us, RUN->errors, RUN->latency_stats);
    } else {
        print_stats(ctx->rank, RUN->msg_bytes, RUN->comm_avg_latency_sec,
                    min_latency_us, max_latency_us, RUN->latency_stats);
    }

    if (BENCH->is_persistent) {
        MPI_CHECK(MPI_Request_free(&(BENCH->request)));
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
}

static void accumulate_nonblocking_times(omb_env_t *ctx)
{
    RUN->elapsed_sec += RUN->t_stop - RUN->t_start;
    RUN->compute_time_total_sec += RUN->compute_time_sec;
    RUN->test_total_sec += RUN->test_time;
    RUN->init_total_sec += RUN->init_time;
    RUN->wait_total_sec += RUN->wait_time;
}

static void run_nonblocking_for_msg_size(omb_env_t *ctx)
{
    int errors_temp;

    run_iterations_core(ctx, BENCH->latency_step, accumulate_elapsed_time);

    omb_papi_stop_and_print(&(BENCH->papi_eventset), RUN->msg_bytes);
    if (options.validate) {
        MPI_CHECK(MPI_Allreduce(&(RUN->local_errors), &(RUN->errors), 1,
                                MPI_INT, MPI_SUM, ctx->comm));
    }

    RUN->avg_latency_us = (RUN->elapsed_sec * 1e6) / options.iterations;
    RUN->latency_sec = RUN->elapsed_sec / options.iterations;

    init_arrays(RUN->latency_sec);

    MPI_CHECK(MPI_Barrier(ctx->comm));

    run_iterations_core(ctx, BENCH->overlap_step, accumulate_nonblocking_times);

    if (options.validate) {
        MPI_CHECK(MPI_Allreduce(&(RUN->local_errors), &errors_temp, 1, MPI_INT,
                                MPI_SUM, ctx->comm));
        RUN->errors += errors_temp;
    }

    RUN->latency_stats = omb_get_stats(BENCH->latency_us_samples);
    RUN->comm_avg_latency_sec = calculate_and_print_stats(
        ctx->rank, RUN->msg_bytes, ctx->numprocs, RUN->elapsed_sec,
        RUN->avg_latency_us, RUN->test_total_sec, RUN->compute_time_total_sec,
        RUN->wait_total_sec, RUN->init_total_sec, RUN->errors,
        RUN->latency_stats);
}

void omb_run_benchmark_for_msg_size(omb_env_t *ctx)
{
    if (BENCH->is_nonblocking) {
        run_nonblocking_for_msg_size(ctx);
    } else {
        run_blocking_for_msg_size(ctx);
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
}
