/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_coll_buffers.h"

static void init_coll_validation_buffers(omb_env_t *ctx, int iter)
{
    switch (options.subtype) {
        case ALLTOALL:
        case ALLTOALL_P:
        case NBC_ALLTOALL: {
            if (0 == options.omb_enable_mpi_in_place) {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 ctx->rank, ctx->numprocs, options.accel, iter,
                                 RUN->dtype, BENCH->buffer_sizes.sendbuf_size);
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_RECVBUF, RUN->msg_bytes,
                                 ctx->rank, ctx->numprocs, options.accel, iter,
                                 RUN->dtype, BENCH->buffer_sizes.recvbuf_size);
            } else {
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 ctx->rank, ctx->numprocs, options.accel, iter,
                                 RUN->dtype, BENCH->buffer_sizes.recvbuf_size);
            }
        } break;
        case GATHER:
        case GATHER_P:
        case NBC_GATHER: {
            if (0 == options.omb_enable_mpi_in_place) {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 ctx->rank * ctx->numprocs, 1, options.accel,
                                 iter, RUN->dtype,
                                 BENCH->buffer_sizes.sendbuf_size);
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_RECVBUF, RUN->msg_bytes,
                                 ctx->rank, ctx->numprocs, options.accel, iter,
                                 RUN->dtype, BENCH->buffer_sizes.recvbuf_size);
            } else {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 ctx->rank * ctx->numprocs, 1, options.accel,
                                 iter, RUN->dtype,
                                 BENCH->buffer_sizes.sendbuf_size);
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_RECVBUF, RUN->msg_bytes,
                                 ctx->rank * ctx->numprocs, 1, options.accel,
                                 iter, RUN->dtype,
                                 BENCH->buffer_sizes.recvbuf_size);
            }
        } break;
        case ALL_GATHER:
        case ALL_GATHER_P:
        case NBC_ALL_GATHER: {
            if (0 == options.omb_enable_mpi_in_place) {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 ctx->rank * ctx->numprocs, 1, options.accel,
                                 iter, RUN->dtype,
                                 BENCH->buffer_sizes.sendbuf_size);
                if (0 == ctx->rank) {
                    set_buffer_dtype(BENCH->recvbuf, OMB_IS_RECVBUF,
                                     RUN->msg_bytes, ctx->rank, ctx->numprocs,
                                     options.accel, iter, RUN->dtype,
                                     BENCH->buffer_sizes.recvbuf_size);
                }
            } else {
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 ctx->rank * ctx->numprocs, 1, options.accel,
                                 iter, RUN->dtype,
                                 BENCH->buffer_sizes.recvbuf_size);
            }
        } break;
        case REDUCE:
        case ALL_REDUCE:
        case REDUCE_P:
        case ALL_REDUCE_P:
        case NBC_REDUCE:
        case NBC_ALL_REDUCE: {
            if (0 == options.omb_enable_mpi_in_place) {
                set_buffer_dtype_reduce(BENCH->sendbuf, OMB_IS_SENDBUF,
                                        RUN->msg_bytes, iter, options.accel,
                                        RUN->dtype);
                set_buffer_dtype_reduce(BENCH->recvbuf, OMB_IS_RECVBUF,
                                        RUN->msg_bytes, iter, options.accel,
                                        RUN->dtype);
            } else {
                set_buffer_dtype_reduce(BENCH->recvbuf, OMB_IS_SENDBUF,
                                        RUN->msg_bytes, iter, options.accel,
                                        RUN->dtype);
            }
            break;
        }
        case SCATTER:
        case SCATTER_P:
        case NBC_SCATTER: {
            if (0 == options.omb_enable_mpi_in_place) {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 0, ctx->numprocs, options.accel, iter,
                                 RUN->dtype, BENCH->buffer_sizes.sendbuf_size);
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_RECVBUF, RUN->msg_bytes,
                                 ctx->rank * ctx->numprocs, 1, options.accel,
                                 iter, RUN->dtype,
                                 BENCH->buffer_sizes.recvbuf_size);
            } else {
                set_buffer_dtype(BENCH->recvbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 0, ctx->numprocs, options.accel, iter,
                                 RUN->dtype, BENCH->buffer_sizes.recvbuf_size);
            }
        } break;
        case REDUCE_SCATTER:
        case REDUCE_SCATTER_P:
        case NBC_REDUCE_SCATTER: {
            if (0 == options.omb_enable_mpi_in_place) {
                set_buffer_dtype_reduce(BENCH->sendbuf, OMB_IS_SENDBUF,
                                        RUN->msg_bytes, iter, options.accel,
                                        RUN->dtype);
                set_buffer_dtype_reduce(BENCH->recvbuf, OMB_IS_RECVBUF,
                                        RUN->msg_bytes / ctx->numprocs + 1,
                                        iter, options.accel, RUN->dtype);
            } else {
                set_buffer_dtype_reduce(BENCH->recvbuf, OMB_IS_SENDBUF,
                                        RUN->msg_bytes, iter, options.accel,
                                        RUN->dtype);
            }
        } break;
        case BCAST:
        case BCAST_P:
        case NBC_BCAST: {
            if (0 == ctx->rank) {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_SENDBUF, RUN->msg_bytes,
                                 1, 1, options.accel, iter, RUN->dtype,
                                 BENCH->buffer_sizes.sendbuf_size);
            } else {
                set_buffer_dtype(BENCH->sendbuf, OMB_IS_RECVBUF, RUN->msg_bytes,
                                 1, 1, options.accel, iter, RUN->dtype,
                                 BENCH->buffer_sizes.recvbuf_size);
            }
        } break;
        default:
            break;
    }
}

void omb_set_buffer_sizes(omb_env_t *ctx)
{
    switch (options.type) {
        case TYPE_ALL_REDUCE:
        case TYPE_BCAST:
        case TYPE_REDUCE_SCATTER:
        case TYPE_REDUCE:
            BENCH->buffer_sizes.sendbuf_size = options.max_message_size;
            BENCH->buffer_sizes.recvbuf_size = options.max_message_size;
            break;
        case TYPE_ALL_GATHER:
        case TYPE_NHBR_GATHER:
        case TYPE_GATHER:
            BENCH->buffer_sizes.sendbuf_size = options.max_message_size;
            BENCH->buffer_sizes.recvbuf_size =
                options.max_message_size * ctx->numprocs;
            break;
        case TYPE_ALLTOALL:
        case TYPE_NHBR_ALLTOALL:
        case TYPE_SCATTER:
            BENCH->buffer_sizes.sendbuf_size =
                options.max_message_size * ctx->numprocs;
            BENCH->buffer_sizes.recvbuf_size =
                options.max_message_size * ctx->numprocs;
            break;
        case TYPE_BARRIER:
            BENCH->buffer_sizes.sendbuf_size = 0;
            BENCH->buffer_sizes.recvbuf_size = 0;
            break;
        default:
            fprintf(stderr, "Unknown options.type encountered while "
                            "calculating buffer size\n");
            exit(EXIT_FAILURE);
            break;
    }
}

void omb_apply_mpi_in_place(omb_env_t *ctx)
{
    switch (options.type) {
        case TYPE_SCATTER:
            if (RUN->root_rank == ctx->rank) {
                RUN->sendbuf_active = BENCH->recvbuf;
                RUN->recvbuf_active = MPI_IN_PLACE;
            }
            break;

        case TYPE_ALLTOALL:
        case TYPE_ALL_GATHER:
        case TYPE_ALL_REDUCE:
        case TYPE_REDUCE_SCATTER:
            RUN->sendbuf_active = MPI_IN_PLACE;
            break;

        case TYPE_GATHER:
            if (RUN->root_rank == ctx->rank) {
                RUN->sendbuf_active = MPI_IN_PLACE;
                RUN->recvbuf_active = BENCH->recvbuf;
            } else {
                RUN->recvbuf_active = NULL;
                RUN->recvbuf_active = BENCH->recvbuf;
            }
            break;

        case TYPE_REDUCE:
            if (RUN->root_rank == ctx->rank) {
                RUN->sendbuf_active = MPI_IN_PLACE;
                RUN->recvbuf_active = BENCH->recvbuf;
            } else {
                RUN->sendbuf_active = BENCH->recvbuf;
                RUN->recvbuf_active = BENCH->recvbuf;
            }
            break;

        default:
            break;
    }
}

void omb_prepare_validation_buffers(omb_env_t *ctx, int iteration)
{
    if (BENCH->is_neighborhood) {
        init_nbhr_validation_buffers(
            BENCH->sendbuf, BENCH->recvbuf, BENCH->indegree, BENCH->sources,
            BENCH->outdegree, BENCH->destinations, RUN->msg_bytes,
            options.accel, iteration, RUN->dtype);
    } else {
        init_coll_validation_buffers(ctx, iteration);
    }
}

void omb_accumulate_local_errors(omb_env_t *ctx, int iteration)
{
    int iter_errors = 0;
    switch (options.type) {
        case TYPE_BARRIER:
            break;

        case TYPE_BCAST:
            iter_errors =
                validate_data(BENCH->sendbuf, RUN->msg_bytes, ctx->numprocs,
                              options.accel, iteration, RUN->dtype);
            break;

        case TYPE_SCATTER:
            if (RUN->root_rank == ctx->rank &&
                1 == options.omb_enable_mpi_in_place) {
                omb_scatter_offset_copy(BENCH->recvbuf, RUN->root_rank,
                                        RUN->msg_bytes);
            }
            iter_errors =
                validate_data(BENCH->recvbuf, RUN->msg_bytes, ctx->numprocs,
                              options.accel, iteration, RUN->dtype);
            break;

        case TYPE_REDUCE_SCATTER:
            if (BENCH->recv_counts[ctx->rank] != 0) {
                iter_errors = validate_reduce_scatter(
                    BENCH->recvbuf, RUN->msg_bytes, BENCH->recv_counts,
                    ctx->rank, ctx->numprocs, options.accel, iteration,
                    RUN->dtype);
            }
            break;

        case TYPE_NHBR_GATHER:
        case TYPE_NHBR_ALLTOALL:
            iter_errors = omb_validate_neighborhood_col(
                BENCH->dist_graph_comm, RUN->recvbuf_active, BENCH->indegree,
                BENCH->outdegree, RUN->msg_bytes, options.accel, iteration,
                RUN->dtype);
            break;

        case TYPE_REDUCE:
        case TYPE_GATHER:
            if (RUN->root_rank != ctx->rank) {
                break;
            }
        default:
            iter_errors =
                validate_data(BENCH->recvbuf, RUN->msg_bytes, ctx->numprocs,
                              options.accel, iteration, RUN->dtype);
            break;
    }

    RUN->local_errors += iter_errors;
}

#define CHECK_COLL_ALLOC(ptr, size, type)                                      \
    do {                                                                       \
        int _ret = allocate_memory_coll((void **)&ptr, size, type);            \
        if (EXIT_SUCCESS != _ret) {                                            \
            fprintf(stderr, "Could Not Allocate Memory [rank %d]\n",           \
                    ctx->rank);                                                \
            return _ret;                                                       \
        }                                                                      \
    } while (0)

/* Irregular Buffers */

static int allocate_irregular_buffer(omb_env_t *ctx)
{
    CHECK_COLL_ALLOC(BENCH->recv_counts, ctx->numprocs * sizeof(int), NONE);
    CHECK_COLL_ALLOC(BENCH->send_counts, ctx->numprocs * sizeof(int), NONE);
    if (BENCH->use_byte_displs) {
        CHECK_COLL_ALLOC(BENCH->recv_displs_bytes,
                         ctx->numprocs * sizeof(MPI_Aint), NONE);
        CHECK_COLL_ALLOC(BENCH->send_displs_bytes,
                         ctx->numprocs * sizeof(MPI_Aint), NONE);
    } else {
        CHECK_COLL_ALLOC(BENCH->recv_displs, ctx->numprocs * sizeof(int), NONE);
        CHECK_COLL_ALLOC(BENCH->send_displs, ctx->numprocs * sizeof(int), NONE);
    }
    if (BENCH->is_generalized_coll) {
        CHECK_COLL_ALLOC(BENCH->recv_types,
                         ctx->numprocs * sizeof(MPI_Datatype), NONE);
        CHECK_COLL_ALLOC(BENCH->send_types,
                         ctx->numprocs * sizeof(MPI_Datatype), NONE);
    }
    return EXIT_SUCCESS;
}

static void free_irregular_buffers(omb_env_t *ctx)
{
    free_buffer(BENCH->recv_counts, NONE);
    free_buffer(BENCH->send_counts, NONE);
    if (BENCH->use_byte_displs) {
        free_buffer(BENCH->recv_displs_bytes, NONE);
        free_buffer(BENCH->send_displs_bytes, NONE);
    } else {
        free_buffer(BENCH->recv_displs, NONE);
        free_buffer(BENCH->send_displs, NONE);
    }
    if (BENCH->is_generalized_coll) {
        free_buffer(BENCH->recv_types, NONE);
        free_buffer(BENCH->send_types, NONE);
    }
}

/* Neighborhood Buffers */

static int omb_create_graph_neighborhood(omb_env_t *ctx)
{
    int i, src_itr, dest_itr, src_temp, dest_temp;
    FILE *fp = NULL;
    char *token = NULL;
    char line[OMB_NHBRHD_FILE_LINE_MAX_LENGTH];

    MALLOC_CHECK_AND_RET(BENCH->sources,
                         OMB_NHBRHD_ADJ_EDGES_MAX_NUM * sizeof(int), ctx->rank);
    MALLOC_CHECK_AND_RET(BENCH->destinations,
                         OMB_NHBRHD_ADJ_EDGES_MAX_NUM * sizeof(int), ctx->rank);

    fp = fopen(options.nhbrhd_type_parameters.filepath, "r");
    if (NULL == fp) {
        if (!ctx->rank) {
            fprintf(stderr, "Unable to open graph adjacency list file.\n");
        }
        return ENOENT;
    }

    src_itr = dest_itr = 0;
    while (fgets(line, OMB_NHBRHD_FILE_LINE_MAX_LENGTH, fp)) {
        src_temp = dest_temp = -1;
        if ('#' == line[0]) {
            continue;
        }
        if (OMB_NHBRHD_ADJ_EDGES_MAX_NUM < src_itr ||
            OMB_NHBRHD_ADJ_EDGES_MAX_NUM < dest_itr) {
            fprintf(stderr,
                    "ERROR: Max allowed number of edges is:%d\n"
                    "To increase the max allowed edges limit, update"
                    " OMB_NHBRHD_ADJ_EDGES_MAX_NUM in"
                    " c/util/osu_util_mpi.h.\n",
                    OMB_NHBRHD_ADJ_EDGES_MAX_NUM);
            fflush(stderr);
            fclose(fp);
            return EINVAL;
        }

        /* Get source/dest pair */
        token = strtok(line, ",");
        if (NULL == token) {
            OMB_ERR_PRINT_RANK_ZERO("Unable to recognise the pattern. Check "
                                    "graph adjacency list file.\n")
        }
        src_temp = atoi(token);

        token = strtok(NULL, ",");
        if (NULL == token) {
            OMB_ERR_PRINT_RANK_ZERO("Unable to recognise the pattern. Check "
                                    "graph adjacency list file.\n")
        }
        dest_temp = atoi(token);

        if (dest_temp >= ctx->numprocs || src_temp >= ctx->numprocs) {
            fprintf(stderr, "Number of processes is less than graph nodes. "
                            "Please increase number of processes.\n");
            fclose(fp);
            return OMB_ERR_PROCESS_DIST;
        }

        if (src_temp == ctx->rank) {
            BENCH->destinations[dest_itr++] = dest_temp;
        }
        if (dest_temp == ctx->rank) {
            BENCH->sources[src_itr++] = src_temp;
        }
    }
    fclose(fp);

    BENCH->indegree = src_itr;
    BENCH->outdegree = dest_itr;
    MALLOC_CHECK_AND_RET(BENCH->sourceweights, BENCH->indegree * sizeof(int),
                         ctx->rank);
    MALLOC_CHECK_AND_RET(BENCH->destweights, BENCH->outdegree * sizeof(int),
                         ctx->rank);

    for (i = 0; i < BENCH->indegree; i++) {
        BENCH->sourceweights[i] = 1;
    }
    for (i = 0; i < BENCH->outdegree; i++) {
        BENCH->destweights[i] = 1;
    }

    return EXIT_SUCCESS;
}

static int omb_neighborhood_create(omb_env_t *ctx)
{
    int i = 0, inidx = 0, outidx = 0, comm_size = 0;
    int nbr_rank = 0, min_dim = 0;

    int overflow = 0;
    int *dims = NULL, *periods = NULL, *my_coords = NULL;
    int *nbr_coords = NULL, *disp_vec = NULL;

    int dim, rad;
    int src_itr = 0, dest_itr = 0, src_temp = 0, dest_temp = 0;
    FILE *fp = NULL;
    char *token = NULL;
    char line[OMB_NHBRHD_FILE_LINE_MAX_LENGTH];
    MPI_Comm cart_comm;

    if (OMB_NHBRHD_TYPE_GRAPH == options.nhbrhd_type) {
        return omb_create_graph_neighborhood(ctx);
    }

    dim = options.nhbrhd_type_parameters.dim;
    rad = options.nhbrhd_type_parameters.rad;

    MALLOC_CHECK_AND_RET(dims, dim * sizeof(int), ctx->rank);
    MALLOC_CHECK_AND_RET(periods, dim * sizeof(int), ctx->rank);
    MALLOC_CHECK_AND_RET(my_coords, dim * sizeof(int), ctx->rank);
    MALLOC_CHECK_AND_RET(nbr_coords, dim * sizeof(int), ctx->rank);
    MALLOC_CHECK_AND_RET(disp_vec, dim * sizeof(int), ctx->rank);

    for (i = 0; i < dim; i++) {
        periods[i] = 1;
        dims[i] = 0;
    }
    MPI_CHECK(MPI_Dims_create(ctx->numprocs, dim, dims));
    MPI_CHECK(MPI_Cart_create(ctx->comm, dim, dims, periods, 0, &cart_comm));
    MPI_Barrier(MPI_COMM_WORLD);
    if (0 == ctx->rank) {
        fprintf(stdout, "Dimensions size = ");
        for (i = 0; i < dim; i++) {
            fprintf(stdout, "%d ", dims[i]);
        }
        fprintf(stdout, "\n");
    }
    /*Find max valid rad based on minimum dimension size*/
    min_dim = ctx->numprocs;
    for (i = 0; i < dim; i++) {
        if (dims[i] < min_dim) {
            min_dim = dims[i];
        }
    }
    /*Divided by 2 to avoid duplicate neighbors*/
    if (rad > ((min_dim - 1) / 2)) {
        if (OMB_OWNS_RESOURCES == ctx->owns_mpi && !ctx->rank) {
            fprintf(stderr,
                    "ERROR: the given neighborhood radius (r = %d)"
                    "is greater than the half of the minimum dimension"
                    " size %d. Increase the number of processes or reduce"
                    " dim and rad. Aborting!\n",
                    rad, min_dim);
            fflush(stderr);
        }
        MPI_Comm_free(&cart_comm);
        return OMB_ERR_PROCESS_DIST;
    }
    /*Calculate number of neighbors*/
    BENCH->outdegree = pow((2 * rad + 1), dim) - 1;
    BENCH->indegree = pow((2 * rad + 1), dim) - 1;

    MALLOC_CHECK_AND_RET(BENCH->sources, BENCH->indegree * sizeof(int),
                         ctx->rank);
    MALLOC_CHECK_AND_RET(BENCH->sourceweights, BENCH->indegree * sizeof(int),
                         ctx->rank);

    MALLOC_CHECK_AND_RET(BENCH->destinations, BENCH->outdegree * sizeof(int),
                         ctx->rank);
    MALLOC_CHECK_AND_RET(BENCH->destweights, BENCH->outdegree * sizeof(int),
                         ctx->rank);

    for (i = 0; i < BENCH->indegree; i++) {
        BENCH->sourceweights[i] = 1;
    }
    for (i = 0; i < BENCH->outdegree; i++) {
        BENCH->destweights[i] = 1;
    }
    for (i = 0; i < dim; i++) {
        disp_vec[i] = -rad;
    }

    MPI_CHECK(MPI_Cart_coords(cart_comm, ctx->rank, dim, my_coords));
    while (!overflow) {
        for (i = 0; i < dim; i++) {
            nbr_coords[i] = my_coords[i] + disp_vec[i];
        }
        MPI_Cart_rank(cart_comm, nbr_coords, &nbr_rank);
        if (nbr_rank != ctx->rank) {
            BENCH->destinations[outidx] = nbr_rank;
            BENCH->sources[inidx] = nbr_rank;
            outidx++;
            inidx++;
        }
        for (i = dim - 1; i >= -1; i--) {
            if (-1 == i) {
                overflow = 1;
                break;
            }
            if (rad == disp_vec[i]) {
                disp_vec[i] = -rad;
            } else {
                disp_vec[i]++;
                break;
            }
        }
    }

    MPI_Comm_free(&cart_comm);
    free(dims);
    free(periods);
    free(my_coords);
    free(nbr_coords);
    free(disp_vec);

    return EXIT_SUCCESS;
}

static int create_dist_graph_and_measure(omb_env_t *ctx)
{
    double t_gca;

    OMB_CHECK_AND_RET(omb_neighborhood_create(ctx));

    t_gca = MPI_Wtime();
    MPI_CHECK(MPI_Dist_graph_create_adjacent(
        ctx->comm, BENCH->indegree, BENCH->sources, BENCH->sourceweights,
        BENCH->outdegree, BENCH->destinations, BENCH->destweights,
        MPI_INFO_NULL, 0, &(BENCH->dist_graph_comm)));
    t_gca = MPI_Wtime() - t_gca;

    MPI_CHECK(MPI_Reduce(&t_gca, &(BENCH->time_gca_total_us), 1, MPI_DOUBLE,
                         MPI_SUM, 0, ctx->comm));

    return EXIT_SUCCESS;
}

static void free_neighborhood(omb_env_t *ctx)
{
    free(BENCH->destweights);
    free(BENCH->destinations);
    free(BENCH->sources);
    free(BENCH->sourceweights);
    MPI_CHECK(MPI_Comm_free(&(BENCH->dist_graph_comm)));
}

/* Buffer API */

int omb_alloc_benchmark_buffers(omb_env_t *ctx)
{
    omb_set_buffer_sizes(ctx);

    CHECK_COLL_ALLOC(BENCH->sendbuf, BENCH->buffer_sizes.sendbuf_size,
                     options.accel);
    set_buffer(BENCH->sendbuf, options.accel, OMB_IS_SENDBUF,
               BENCH->buffer_sizes.sendbuf_size);
    RUN->sendbuf_active = BENCH->sendbuf;

    CHECK_COLL_ALLOC(BENCH->recvbuf, BENCH->buffer_sizes.recvbuf_size,
                     options.accel);
    set_buffer(BENCH->recvbuf, options.accel, OMB_IS_RECVBUF,
               BENCH->buffer_sizes.recvbuf_size);
    RUN->recvbuf_active = BENCH->recvbuf;

    if (options.omb_tail_lat) {
        MALLOC_CHECK_AND_RET(BENCH->latency_us_samples,
                             options.iterations * sizeof(double), ctx->rank);
    }

    if (BENCH->irregular_args) {
        OMB_CHECK_AND_RET(allocate_irregular_buffer(ctx));
    }

    if (BENCH->is_neighborhood) {
        OMB_CHECK_AND_RET(create_dist_graph_and_measure(ctx));
    }

    return EXIT_SUCCESS;
}

void omb_free_benchmark_buffers(omb_env_t *ctx)
{
    free_buffer(BENCH->recvbuf, options.accel);
    free_buffer(BENCH->sendbuf, options.accel);

    omb_graph_free_data_buffers(&(BENCH->graph_options));
    omb_papi_free(&(BENCH->papi_eventset));

    if (BENCH->irregular_args) {
        free_irregular_buffers(ctx);
    }

    if (BENCH->is_neighborhood) {
        free_neighborhood(ctx);
    }

    if (options.omb_tail_lat) {
        free(BENCH->latency_us_samples);
    }
}
