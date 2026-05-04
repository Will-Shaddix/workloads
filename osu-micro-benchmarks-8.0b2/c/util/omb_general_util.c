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
#include <errno.h>

char *omb_strerror(int __errnum)
{
    switch (__errnum) {
        case OMB_ERR_BAD_USAGE:
            return "Failed to process options";
        case OMB_ERR_HELP_MESSAGE:
            return "-h or --help was passed";
        case OMB_ERR_VERSION_MESSAGE:
            return "-v or --version was passed";
        case OMB_ERR_VALIDATION_FAILURE:
            return "Errors detected in validation";
        case OMB_ERR_PROCESS_DIST:
            return "Invalid processes count or distribution";
        case OMB_ERR_NOT_LAUNCHED:
            return "This test was not launched";
        case OMB_ERR_INVALID_SYNC:
            return "Specified sync type is unavailable";
        default:
            return strerror(__errnum);
    }
}

int normalize_exit_code(int rc)
{
    switch (rc) {
        case OMB_ERR_HELP_MESSAGE:
        case OMB_ERR_VERSION_MESSAGE:
            return EXIT_SUCCESS;
        default:
            return rc;
    }
}

void omb_env_init(omb_env_t *ctx, int *argc, char **argv[],
                  omb_resource_owner_t owner)
{
    ctx->argc = *argc;
    ctx->argv = *argv;
    ctx->owns_mpi = owner;

    ctx->omb_init = omb_mpi_init(argc, argv);
    ctx->comm = ctx->omb_init.omb_comm;

    if (MPI_COMM_NULL == ctx->comm) {
        OMB_ERROR_EXIT("Cant create communicator");
    }

    ctx->title = "OSU Micro-Benchmarks";
    ctx->omb_bench = NULL;
    ctx->omb_run = NULL;

    MPI_CHECK(MPI_Comm_rank(ctx->comm, &(ctx->rank)));
    MPI_CHECK(MPI_Comm_size(ctx->comm, &(ctx->numprocs)));

    MPI_CHECK(MPI_Barrier(ctx->comm));
}

int omb_get_node_information(omb_env_t *ctx, bool validate_for_fan)
{
    int node_id, node_count, local_rank, ppn;
    int err_code = EXIT_SUCCESS, ppn_min = 0, ppn_max = 0;
    MPI_Comm node = MPI_COMM_NULL, local_roots = MPI_COMM_NULL;

    /* Per node comm */
    MPI_CHECK(MPI_Comm_split_type(ctx->comm, MPI_COMM_TYPE_SHARED, 0,
                                  MPI_INFO_NULL, &node));

    MPI_CHECK(MPI_Comm_rank(node, &local_rank));
    MPI_CHECK(MPI_Comm_size(node, &ppn));

    /* Enumerate nodes */
    MPI_CHECK(MPI_Comm_split(ctx->comm, local_rank ? MPI_UNDEFINED : 0,
                             ctx->rank, &local_roots));

    if (!local_rank) {
        MPI_CHECK(MPI_Comm_rank(local_roots, &node_id));
        MPI_CHECK(MPI_Comm_size(local_roots, &node_count));
        MPI_CHECK(MPI_Comm_free(&local_roots));
    }

    /* Broadcast info within each node */
    MPI_CHECK(MPI_Bcast(&node_id, 1, MPI_INT, 0, node));
    MPI_CHECK(MPI_Bcast(&node_count, 1, MPI_INT, 0, node));
    MPI_CHECK(MPI_Comm_free(&node));

    if (validate_for_fan) {
        if (node_count < 2) {
            err_code = OMB_ERR_PROCESS_DIST;
            if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
                fprintf(stderr,
                        "Please run this benchmark on more than 1 node\n");
            }
        }

        MPI_CHECK(
            MPI_Allreduce(&ppn, &ppn_min, 1, MPI_INT, MPI_MIN, ctx->comm));
        MPI_CHECK(
            MPI_Allreduce(&ppn, &ppn_max, 1, MPI_INT, MPI_MAX, ctx->comm));

        if (ppn_min != ppn_max) {
            err_code = OMB_ERR_PROCESS_DIST;
            if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
                fprintf(
                    stderr,
                    "This benchmark requires all nodes to have the same ppn "
                    "(min=%d, max=%d)\n",
                    ppn_min, ppn_max);
            }
        }

        MPI_Bcast(&err_code, 1, MPI_INT, 0, ctx->comm);
        OMB_CHECK_AND_RET(err_code);
    }

    /* Store results */
    MPI_CHECK(MPI_Get_processor_name(NODE.name, &NODE.name_len));
    NODE.id = node_id;
    NODE.ppn = ppn;
    NODE.local_rank = local_rank;
    NODE.world_node_count = node_count;

    return EXIT_SUCCESS;
}

void print_bench_preamble(int rank, bool is_nonblocking)
{
    if (is_nonblocking) {
        print_preamble_nbc(rank);
    } else {
        print_preamble(rank);
    }
}

void omb_print_header(int rank, bool is_nonblocking)
{
    if (is_nonblocking) {
        print_only_header_nbc(rank);
    } else {
        print_only_header(rank);
    }
}

void omb_print_pair_info(omb_env_t *ctx)
{
    if (ctx->rank) {
        return;
    }

    if (options.window_varied) {
        fprintf(stdout, "# [ pairs: %d ] [ window size: varied ]\n",
                options.pairs);
    } else {
        fprintf(stdout, "# [ pairs: %d ] [ window size: %d ]\n", options.pairs,
                options.window_size);
    }
}

int omb_handle_option_processing(omb_env_t *ctx)
{
    int po_ret = process_options(ctx->argc, ctx->argv);

    switch (po_ret) {
        case EXIT_SUCCESS:
            break;

        case OMB_ERR_CUDA_NOT_AVAIL:
            if (!ctx->rank) {
                fprintf(stderr, "CUDA support not enabled. Please recompile "
                                "benchmark with CUDA support.\n");
            }
            break;

        case OMB_ERR_OPENACC_NOT_AVAIL:
            if (!ctx->rank) {
                fprintf(stderr, "OPENACC support not enabled. Please "
                                "recompile benchmark with OPENACC support.\n");
            }
            break;

        case OMB_ERR_BAD_USAGE:
            if (ctx->owns_mpi || options.type == TYPE_LAUNCHER) {
                print_bad_usage_message(ctx->rank);
            }
            break;

        case OMB_ERR_HELP_MESSAGE:
            print_help_message(ctx->rank);
            break;

        case OMB_ERR_VERSION_MESSAGE:
            print_version_message(ctx->rank);
            break;

        default:
            if (!ctx->rank) {
                fprintf(stderr,
                        "process_options returned unrecognized value: '%d'\n",
                        po_ret);
            }
            return EXIT_FAILURE;
            break;
    }

    if (NONE != options.accel) {
        if (init_accel()) {
            fprintf(stderr, "Error initializing device\n");
            return ENODEV;
        }
    }

    return po_ret;
}
