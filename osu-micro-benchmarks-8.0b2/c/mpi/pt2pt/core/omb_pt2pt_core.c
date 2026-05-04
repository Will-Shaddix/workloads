/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_pt2pt_core.h"
#include "omb_pt2pt_buffers.h"
#include "omb_bw_fan_util.h"
#include "osu_util.h"
#include <stdlib.h>

static inline void init_relationships(omb_env_t *ctx)
{
    options.pairs = BENCH->pair_count = ctx->numprocs / 2;
    BENCH->is_sender = ctx->rank < BENCH->pair_count;

    if (BENCH->is_sender) {
        BENCH->partner_rank = ctx->rank + BENCH->pair_count;
    } else {
        BENCH->partner_rank = ctx->rank - BENCH->pair_count;
    }

    MPI_CHECK(
        MPI_Comm_split(ctx->comm, BENCH->is_sender, 0, &BENCH->sender_comm));
}

static int check_num_procs(omb_env_t *ctx)
{
    if (BENCH->subtype == CONG_BW) {
    } else if (ctx->numprocs < 2) {
        if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
            fprintf(stderr, "This test requires at least two processes\n");
        }
        return OMB_ERR_PROCESS_DIST;
    } else if (BENCH->is_paired) {
        if (ctx->numprocs % 2) {
            if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
                fprintf(stderr,
                        "This test requires an even number of processes\n");
            }
            return OMB_ERR_PROCESS_DIST;
        }
    } else if (ctx->numprocs != 2) {
        if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }
        return OMB_ERR_PROCESS_DIST;
    }

    return EXIT_SUCCESS;
}

static void init_varied_window(omb_env_t *ctx)
{
    int window_array[] = WINDOW_SIZES;

    if (options.window_varied) {
        options.window_size = window_array[WINDOW_SIZES_COUNT - 1];
        BENCH->window_sizes_count = WINDOW_SIZES_COUNT;
    } else {
        BENCH->window_sizes_count = 1;
    }
}

static int check_thread_support(omb_env_t *ctx)
{
    int thread_support;

    if (BENCH->subtype == LAT_MT) {
        MPI_CHECK(MPI_Query_thread(&thread_support));
        if (thread_support != MPI_THREAD_MULTIPLE) {
            if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
                fprintf(stderr,
                        "MPI must be initialized with MPI_THREAD_MULTIPLE!\n");
            }
            not_launched_message = "This test must be explicitly passed";
            return OMB_ERR_NOT_LAUNCHED;
        }
    }

    return EXIT_SUCCESS;
}

int omb_bench_init(omb_env_t *ctx)
{
    int i, omb_ret;

    options.domain = BENCH->domain;
    options.subtype = BENCH->subtype;
    options.type = BENCH->type = get_type_from_subtype(BENCH->subtype);

    set_benchmark_name(ctx->title);
    set_header(BENCH->header);

    OMB_CHECK_AND_RET(check_num_procs(ctx));

    if (CONG_BW != BENCH->subtype) {
        init_relationships(ctx);
    }

    OMB_CHECK_AND_RET(omb_handle_option_processing(ctx));

    OMB_CHECK_AND_RET(check_thread_support(ctx));

    omb_populate_mpi_type_list(BENCH->dtype_list);
    omb_graph_options_init(&(BENCH->graph_options));
    OMB_CHECK_AND_RET(omb_alloc_benchmark_buffers(ctx));
    omb_papi_init(&(BENCH->papi_eventset));

    if (CONG_BW == BENCH->subtype) {
        OMB_CHECK_AND_RET(omb_fan_init(ctx));
    }

    if (options.subtype == LAT_MP && options.sender_processes != -1) {
        BENCH->multiprocess->num_processes_sender = options.sender_processes;
    }

    if (options.subtype == LAT_MT && options.validate &&
        options.num_threads != options.sender_thread) {
        if (!ctx->rank) {
            fprintf(stderr,
                    "Number of sender and receiver threads must be same"
                    " when validation is enabled. Use option -t to set\n");
        }
        return EINVAL;
    }

    init_varied_window(ctx);

    return EXIT_SUCCESS;
}

void omb_select_active_dtype(omb_env_t *ctx)
{
    RUN->dtype = BENCH->dtype_list[RUN->dtype_index];
    OMB_MPI_REDUCE_CHAR_CHECK(RUN->dtype);
    MPI_CHECK(MPI_Type_size(RUN->dtype, &(RUN->dtype_size)));
    MPI_CHECK(
        MPI_Type_get_name(RUN->dtype, RUN->dtype_name, &(RUN->dtype_name_len)));
}

void omb_print_dtype_and_header(omb_env_t *ctx)
{
    int i;
    int window_array[] = WINDOW_SIZES;

    if (ctx->rank || RUN->tid) {
        return;
    }

    if (options.show_dtype) {
        fprintf(stdout, "# Datatype: %s.\n", RUN->dtype_name);
        fflush(stdout);
    }

    if (options.window_varied) {
        fprintf(stdout, "%-*s", 10, "#");

        for (i = 0; i < BENCH->window_sizes_count; i++) {
            fprintf(stdout, "%*d", TIGHT_FIELD_WIDTH, window_array[i]);
        }

        fprintf(stdout, "\n");
        fflush(stdout);
    } else {
        omb_print_header(ctx->rank, BENCH->subtype == PART_LAT);
    }
}

size_t omb_compute_iters_for_msg_size(omb_env_t *ctx)
{
    RUN->elem_count = RUN->msg_bytes / RUN->dtype_size;
    if (BENCH->subtype == PART_LAT) {
        RUN->elem_count = RUN->elem_count / options.num_partitions;
    }
    if (RUN->elem_count) {
        get_iterations(RUN->msg_bytes);
        get_skips(RUN->msg_bytes);
    }
    return RUN->elem_count;
}

void omb_prepare_graph_for_msg_size(omb_env_t *ctx)
{
    omb_graph_allocate_and_get_data_buffer(&(BENCH->graph_data),
                                           &(BENCH->graph_options),
                                           RUN->msg_bytes, options.iterations);
}

void omb_handle_ddt_size(omb_env_t *ctx)
{
    RUN->ddt_transmit_size =
        omb_ddt_assign(&(RUN->dtype), BENCH->dtype_list[RUN->dtype_index],
                       RUN->elem_count) *
        RUN->dtype_size;
    RUN->elem_count = omb_ddt_get_size(RUN->elem_count);
}

void omb_plot_graphs(omb_env_t *ctx)
{
    if (0 == ctx->rank && options.graph) {
        omb_graph_plot(&(BENCH->graph_options), ctx->title);
    }
    omb_graph_combined_plot(&(BENCH->graph_options), ctx->title);
}

void omb_teardown_benchmark(omb_env_t *ctx)
{
    omb_free_benchmark_buffers(ctx);

    MPI_Comm_free(&BENCH->sender_comm);

    if (CONG_BW == BENCH->subtype) {
        omb_finalize_fan(ctx);
    }

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }
}

void omb_measure_kernel_lo(omb_env_t *ctx)
{
#ifdef _ENABLE_CUDA_KERNEL_
    if (options.buf_num == MULTIPLE) {
        if (options.dst == 'M' && options.MMdst == 'D') {
            RUN->t_lo = measure_kernel_lo_window(
                BENCH->multiple_sendbuf, RUN->msg_bytes, options.window_size);
        }
    } else {
        if ((options.src == 'M' && options.MMsrc == 'D') ||
            (options.dst == 'M' && options.MMdst == 'D')) {
            RUN->t_lo =
                measure_kernel_lo_no_window(BENCH->sendbuf, RUN->msg_bytes);
        }
    }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
}

int omb_check_and_report_errors(omb_env_t *ctx)
{
    if (options.validate && RUN->errors) {
        if (!ctx->rank) {
            fprintf(stderr,
                    "DATA VALIDATION ERROR: %s exited with %d errors on"
                    " message size %lu.\n",
                    ctx->argv[0], RUN->errors, RUN->msg_bytes);
        }
        return OMB_ERR_VALIDATION_FAILURE;
    }
    return EXIT_SUCCESS;
}
