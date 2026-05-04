/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_one_sided_buffers.h"

int omb_alloc_benchmark_buffers(omb_env_t *ctx)
{
    if (options.omb_tail_lat) {
        MALLOC_CHECK_AND_RET(BENCH->measurement_samples,
                             options.iterations * sizeof(double), ctx->rank);
    }

    return EXIT_SUCCESS;
}

void omb_allocate_one_sided_memory(omb_env_t *ctx)
{
    switch (BENCH->benchmark_type) {
        case GET_ACCUMULATE:
            allocate_memory_get_acc_lat(ctx);
            break;
        case COMPARE_AND_SWAP:
        case FETCH_AND_OP:
            allocate_atomic_memory(
                ctx->rank, (char **)&(BENCH->alt_op_buf),
                (char **)&(BENCH->tbuf), (char **)&(BENCH->cbuf),
                (char **)&(BENCH->alt_win_base), RUN->msg_bytes, options.win,
                &(RUN->win_handle));
            break;
        default:
            allocate_memory_one_sided(ctx->rank, &(BENCH->op_buf),
                                      &(BENCH->win_base),
                                      RUN->msg_bytes * options.window_size,
                                      options.win, &(RUN->win_handle));
            break;
    }
}

void omb_free_one_sided_memory(omb_env_t *ctx)
{
    switch (BENCH->benchmark_type) {
        case GET_ACCUMULATE:
            MPI_Win_free(&(RUN->win_handle));
            break;
        case COMPARE_AND_SWAP:
        case FETCH_AND_OP:
            free_atomic_memory(BENCH->alt_op_buf, BENCH->alt_win_base,
                               BENCH->tbuf, BENCH->cbuf, options.win,
                               RUN->win_handle, ctx->rank);
            break;
        default:
            free_memory_one_sided(BENCH->op_buf, BENCH->win_base, options.win,
                                  RUN->win_handle, ctx->rank);
            break;
    }
}

int omb_alloc_run_buffers(omb_env_t *ctx)
{
    allocate_memory_one_sided(ctx->rank, &(BENCH->op_buf), &(BENCH->win_base),
                              RUN->msg_bytes * options.window_size, options.win,
                              &(RUN->win_handle));
}

void allocate_memory_get_acc_lat(omb_env_t *ctx)
{
    switch (options.win) {
        case WIN_DYNAMIC:
            MPI_CHECK(MPI_Win_create_dynamic(MPI_INFO_NULL, ctx->comm,
                                             &(RUN->win_handle)));
            MPI_CHECK(MPI_Win_attach(RUN->win_handle, (void *)BENCH->rbuf,
                                     RUN->msg_bytes));
            MPI_CHECK(MPI_Get_address(BENCH->rbuf, &(RUN->sdisp_local)));
            if (ctx->rank == 0) {
                MPI_CHECK(MPI_Send(&(RUN->sdisp_local), 1, MPI_AINT, 1, 1,
                                   ctx->comm));
                MPI_CHECK(MPI_Recv(&(RUN->sdisp_remote), 1, MPI_AINT, 1, 1,
                                   ctx->comm, &(RUN->reqstat)));
            } else {
                MPI_CHECK(MPI_Recv(&(RUN->sdisp_remote), 1, MPI_AINT, 0, 1,
                                   ctx->comm, &(RUN->reqstat)));
                MPI_CHECK(MPI_Send(&(RUN->sdisp_local), 1, MPI_AINT, 0, 1,
                                   ctx->comm));
            }
            break;
        case WIN_CREATE:
            MPI_CHECK(MPI_Win_create(BENCH->rbuf, RUN->msg_bytes, 1,
                                     MPI_INFO_NULL, ctx->comm,
                                     &(RUN->win_handle)));
            break;
        default:
            MPI_CHECK(MPI_Win_allocate(RUN->msg_bytes, 1, MPI_INFO_NULL,
                                       ctx->comm, (void *)&(BENCH->rbuf),
                                       &(RUN->win_handle)));
            break;
    }
}
