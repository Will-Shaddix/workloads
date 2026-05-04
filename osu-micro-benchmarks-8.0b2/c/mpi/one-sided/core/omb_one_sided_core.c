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

static void allocate_and_align_memory(omb_env_t *ctx)
{
    int page_size = getpagesize();
    assert(page_size <= MAX_ALIGNMENT);
    RUN->msg_bytes = options.max_message_size;

    CHECK(posix_memalign((void **)&(BENCH->op_buf), page_size, RUN->msg_bytes));
    memset(BENCH->op_buf, 0, RUN->msg_bytes);

    if (options.win != WIN_ALLOCATE) {
        CHECK(
            posix_memalign((void **)&(BENCH->rbuf), page_size, RUN->msg_bytes));
        memset(BENCH->op_buf, 0, RUN->msg_bytes);
    }

    CHECK(posix_memalign((void **)&(BENCH->cbuf), page_size, RUN->msg_bytes));
    memset(BENCH->cbuf, 0, RUN->msg_bytes);
}

static void set_op_ptr(omb_env_t *ctx)
{
    switch (BENCH->benchmark_type) {
        case PUT:
            BENCH->op_ptr = MPI_Put;
            break;
        case GET:
            BENCH->op_ptr =
                (int (*)(const void *, int, MPI_Datatype, int, MPI_Aint, int,
                         MPI_Datatype, MPI_Win))MPI_Get;
            break;
        default:
            BENCH->op_ptr = NULL;
            break;
    }
}

int validate_sync_type(omb_env_t *ctx)
{
    if (BI_BW == BENCH->subtype && PSCW != options.sync &&
        FENCE != options.sync) {
        return OMB_ERR_INVALID_SYNC;
    }

    switch (options.sync) {
        case LOCK:
        case PSCW:
        case FENCE:
#if _ENABLE_MPI3_
        case LOCK_ALL:
        case FLUSH_LOCAL:
        case FLUSH:
#endif
            return EXIT_SUCCESS;
        default:
            return OMB_ERR_INVALID_SYNC;
    }
}

void set_default_sync_type(omb_env_t *ctx)
{
#if _ENABLE_MPI3_
    options.win = WIN_ALLOCATE;
    options.sync = FLUSH;
#else
    options.win = WIN_CREATE;
    options.sync = LOCK;
#endif

    /* osu_put_bibw only supports PSCW & FENCE */
    if (PUT == BENCH->benchmark_type && BI_BW == BENCH->subtype) {
        printf("Hello\n");
        options.sync = PSCW;
    }
}

int omb_bench_init(omb_env_t *ctx)
{
    options.domain = BENCH->domain;
    options.subtype = BENCH->subtype;
    set_benchmark_name(ctx->title);
    set_header(BENCH->header);

    set_op_ptr(ctx);

    if (ctx->numprocs != 2) {
        if (0 == ctx->rank) {
            fprintf(stderr, "This test requires exactly two processes\n");
        }
        return OMB_ERR_PROCESS_DIST;
    }

    set_default_sync_type(ctx);

    OMB_CHECK_AND_RET(omb_handle_option_processing(ctx));

    OMB_CHECK_AND_RET(validate_sync_type(ctx));

    if (FETCH_AND_OP == BENCH->benchmark_type ||
        COMPARE_AND_SWAP == BENCH->benchmark_type) {
        options.max_message_size = 1;
    }

    if (options.omb_tail_lat) {
        MALLOC_CHECK_AND_RET(BENCH->measurement_samples,
                             options.iterations * sizeof(double), ctx->rank);
    }

    /* Takes ping-pong sync types into account when calculating latency */
    if (FENCE == options.sync || PSCW == options.sync) {
        BENCH->latency_factor = 0.5;
    } else {
        BENCH->latency_factor = 1;
    }

    BENCH->disp = 0;
#if _ENABLE_MPI3_
    if (WIN_DYNAMIC == options.win) {
        BENCH->disp = disp_remote;
    }
#endif

    if (GET_ACCUMULATE == BENCH->benchmark_type) {
        allocate_and_align_memory(ctx);
    }

    omb_populate_mpi_type_list(BENCH->dtype_list);
    omb_graph_options_init(&BENCH->graph_options);
    omb_papi_init(&BENCH->papi_eventset);

    MPI_CHECK(MPI_Comm_group(ctx->comm, &RUN->comm_group));

    return EXIT_SUCCESS;
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

void omb_select_active_dtype(omb_env_t *ctx)
{
    RUN->dtype = BENCH->dtype_list[RUN->dtype_index];
    MPI_CHECK(MPI_Type_size(RUN->dtype, &(RUN->dtype_size)));
}

void omb_teardown_benchmark(omb_env_t *ctx)
{
    omb_graph_combined_plot(&(BENCH->graph_options), benchmark_name);
    omb_graph_free_data_buffers(&(BENCH->graph_options));
    omb_papi_free(&(BENCH->papi_eventset));

    if (GET_ACCUMULATE == BENCH->benchmark_type) {
        free(BENCH->op_buf);
        if (options.win != WIN_ALLOCATE) {
            free(BENCH->rbuf);
        }
        free(BENCH->cbuf);
    }

    free(BENCH->measurement_samples);
    MPI_CHECK(MPI_Group_free(&(RUN->comm_group)));

    if (NONE != options.accel) {
        if (cleanup_accel()) {
            fprintf(stderr, "Error cleaning up device\n");
            exit(EXIT_FAILURE);
        }
    }
}
