/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_coll_core.h"
#include "omb_coll_buffers.h"

int omb_bench_init(omb_env_t *ctx)
{
    int omb_ret;

    options.domain = BENCH->domain;
    options.subtype = BENCH->subtype;
    options.type = BENCH->type = get_type_from_subtype(BENCH->subtype);

    set_benchmark_name(ctx->title);
    set_header(BENCH->header);

    /* derived bench flags */
    if (BENCH->is_neighborhood && BENCH->is_generalized_coll) {
        BENCH->use_byte_displs = 1;
    }

    if (ctx->numprocs < 2) {
        if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
            fprintf(stderr,
                    "Collective tests require at least two processes\n");
        }
        return OMB_ERR_PROCESS_DIST;
    }

    OMB_CHECK_AND_RET(omb_handle_option_processing(ctx));

    omb_populate_mpi_type_list(BENCH->dtype_list);
    omb_graph_options_init(&(BENCH->graph_options));
    OMB_CHECK_AND_RET(omb_alloc_benchmark_buffers(ctx));
    omb_papi_init(&(BENCH->papi_eventset));

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
    if (options.show_dtype && !ctx->rank) {
        fprintf(stdout, "# Datatype: %s.\n", RUN->dtype_name);
        fflush(stdout);
    }
    omb_print_header(ctx->rank, BENCH->is_nonblocking);
}

size_t omb_compute_iters_for_msg_size(omb_env_t *ctx)
{
    RUN->elem_count = RUN->msg_bytes / RUN->dtype_size;
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
    MPI_CHECK(MPI_Barrier(ctx->comm));
}

void omb_handle_ddt_size(omb_env_t *ctx)
{
    switch (options.type) {
        case TYPE_REDUCE:
        case TYPE_ALL_REDUCE:
        case TYPE_REDUCE_SCATTER:
            break;
        default:
            RUN->ddt_transmit_size =
                omb_ddt_assign(&(RUN->dtype),
                               BENCH->dtype_list[RUN->dtype_index],
                               RUN->elem_count) *
                RUN->dtype_size;
            RUN->elem_count = omb_ddt_get_size(RUN->elem_count);
            break;
    }
}

void omb_init_irregular_args(omb_env_t *ctx)
{
    BENCH->portion = RUN->elem_count / ctx->numprocs;
    int remainder = RUN->elem_count % ctx->numprocs;
    int displacement_mult = BENCH->is_generalized_coll ? RUN->dtype_size : 1;
    int i, disp = 0;

    if (BENCH->irregular_block) {
        for (i = 0; i < ctx->numprocs; i++) {
            BENCH->recv_counts[i] = 0;
            if (RUN->elem_count > ctx->numprocs) {
                BENCH->recv_counts[i] += BENCH->portion;
            }
        }
    } else if (BENCH->irregular_args) {
        switch (options.type) {
            case TYPE_REDUCE_SCATTER:
                for (i = 0; i < ctx->numprocs; i++) {
                    BENCH->recv_counts[i] = 0;
                    if (RUN->elem_count < ctx->numprocs) {
                        if (i < RUN->elem_count) {
                            BENCH->recv_counts[i] = 1;
                        }
                    } else {
                        if ((remainder != 0) && (i < remainder)) {
                            BENCH->recv_counts[i] += 1;
                        }
                        BENCH->recv_counts[i] += BENCH->portion;
                    }
                }
                break;

            default:
                for (i = 0; i < ctx->numprocs; i++) {
                    BENCH->recv_counts[i] = RUN->elem_count;
                    BENCH->send_counts[i] = RUN->elem_count;
                    if (BENCH->use_byte_displs) {
                        BENCH->recv_displs_bytes[i] = disp * displacement_mult;
                        BENCH->send_displs_bytes[i] = disp * displacement_mult;
                    } else {
                        BENCH->recv_displs[i] = disp * displacement_mult;
                        BENCH->send_displs[i] = disp * displacement_mult;
                    }
                    if (BENCH->is_generalized_coll) {
                        BENCH->send_types[i] = RUN->dtype;
                        BENCH->recv_types[i] = RUN->dtype;
                    }
                    disp += RUN->elem_count;
                }
                break;
        }
    }
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

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }
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
