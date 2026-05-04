/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "../omb_one_sided_core.h"

#ifndef OMB_BW_SYNC_SYM
#error "OMB_BW_SYNC_SYM must be defined"
#endif

#ifndef OMB_LAT_SYNC_SYM
#error "OMB_LAT_SYNC_SYM must be defined"
#endif

#ifndef OMB_BIBW_SYNC_SYM
#error "OMB_BIBW_SYNC_SYM must be defined"
#endif

#ifndef OMB_ACC_LAT_SYNC_SYM
#error "OMB_ACC_LAT_SYNC_SYM must be defined"
#endif

#ifndef OMB_GET_ACC_LATENCY_SYNC_SYM
#error "OMB_GET_ACC_LATENCY_SYNC_SYM must be defined"
#endif

#ifndef MPI_OP_PRE_SYNC
#define MPI_OP_PRE_SYNC(group, window) /* Nothing to do here */
#endif

#ifndef MPI_OP_POST_SYNC
#define MPI_OP_POST_SYNC(window) /* Nothing to do here */
#endif

#ifndef MPI_NON_OP_PRE_SYNC
#define MPI_NON_OP_PRE_SYNC(group, window) /* Nothing to do here */
#endif

#ifndef MPI_NON_OP_POST_SYNC
#define MPI_NON_OP_POST_SYNC(window) /* Nothing to do here */
#endif

#ifndef MPI_LOOP_LOCK
#define MPI_LOOP_LOCK(window) /* Nothing to do here */
#endif

#ifndef MPI_LOOP_UNLOCK
#define MPI_LOOP_UNLOCK(window) /* Nothing to do here */
#endif

#ifndef MPI_PING
#define MPI_PING(group, window) /* Nothing to do here */
#endif

#ifndef MPI_LAT_PONG
#define MPI_LAT_PONG(group, window) /* Nothing to do here */
#endif

#ifndef MPI_ACC_PONG
#define MPI_ACC_PONG(group, window) /* Nothing to do here */
#endif

#ifndef MPI_CAS_PONG
#define MPI_CAS_PONG(group, window) /* Nothing to do here */
#endif

#ifndef MPI_FOP_PONG
#define MPI_FOP_PONG(group, window) /* Nothing to do here */
#endif

#ifndef MPI_GET_ACC_PONG
#define MPI_GET_ACC_PONG(group, window) /* Nothing to do here */
#endif

#ifndef MPI_BIBW_PRE_SYNC
#define MPI_BIBW_PRE_SYNC(group, window) /* Nothing to do here */
#endif

#ifndef MPI_BIBW_POST_SYNC
#define MPI_BIBW_POST_SYNC(group, window) /* Nothing to do here */
#endif

void OMB_BW_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        MPI_LOOP_LOCK(RUN->win_handle);
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_OP_PRE_SYNC(RUN->group, RUN->win_handle);

            for (j = 0; j < options.window_size; j++) {
                MPI_CHECK(BENCH->op_ptr(
                    BENCH->op_buf + (j * RUN->msg_bytes), RUN->msg_bytes,
                    MPI_CHAR, RUN->destrank, BENCH->disp + (j * RUN->msg_bytes),
                    RUN->msg_bytes, MPI_CHAR, RUN->win_handle));
            }

            MPI_OP_POST_SYNC(RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->msg_bytes / 1e6) * options.window_size /
                        (RUN->t_graph_end - RUN->t_graph_start);
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->msg_bytes / 1e6) * options.window_size /
                        (RUN->t_graph_end - RUN->t_graph_start);
                }
            }
        }
        RUN->t_stop = MPI_Wtime();
        MPI_LOOP_UNLOCK(RUN->win_handle);
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }
            MPI_NON_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_NON_OP_POST_SYNC(RUN->win_handle);
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}

void OMB_LAT_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        MPI_LOOP_LOCK(RUN->win_handle);
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_CHECK(BENCH->op_ptr(BENCH->op_buf + (j * RUN->msg_bytes),
                                    RUN->msg_bytes, MPI_CHAR, RUN->destrank,
                                    BENCH->disp + (j * RUN->msg_bytes),
                                    RUN->msg_bytes, MPI_CHAR, RUN->win_handle));

            MPI_OP_POST_SYNC(RUN->win_handle);
            MPI_PING(RUN->group, RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6 / 2.0;
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6 / 2.0;
                }
            }
        }
        RUN->t_stop = MPI_Wtime();
        MPI_LOOP_UNLOCK(RUN->win_handle);
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }
            MPI_NON_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_NON_OP_POST_SYNC(RUN->win_handle);
            MPI_LAT_PONG(RUN->group, RUN->win_handle);
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}

void OMB_BIBW_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_BIBW_PRE_SYNC(RUN->group, RUN->win_handle);

            for (j = 0; j < options.window_size; j++) {
                MPI_CHECK(BENCH->op_ptr(
                    BENCH->op_buf + (j * RUN->msg_bytes), RUN->msg_bytes,
                    MPI_CHAR, 1, BENCH->disp + (j * RUN->msg_bytes),
                    RUN->msg_bytes, MPI_CHAR, RUN->win_handle));
            }

            MPI_BIBW_POST_SYNC(RUN->group, RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->msg_bytes / 1e6) * options.window_size /
                        (RUN->t_graph_end - RUN->t_graph_start) * 2.0;
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->msg_bytes / 1e6) * options.window_size /
                        (RUN->t_graph_end - RUN->t_graph_start) * 2.0;
                }
            }
        }
        RUN->t_stop = MPI_Wtime();
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }
            MPI_BIBW_PRE_SYNC(RUN->group, RUN->win_handle);

            for (j = 0; j < options.window_size; j++) {
                MPI_CHECK(BENCH->op_ptr(
                    BENCH->op_buf + (j * RUN->msg_bytes), RUN->msg_bytes,
                    MPI_CHAR, 0, BENCH->disp + (j * RUN->msg_bytes),
                    RUN->msg_bytes, MPI_CHAR, RUN->win_handle));
            }

            MPI_BIBW_POST_SYNC(RUN->group, RUN->win_handle);
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}

void OMB_ACC_LAT_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));
        MPI_LOOP_LOCK(RUN->win_handle);

        if (options.validate) {
            atomic_data_validation_setup(RUN->dtype, ctx->rank, BENCH->op_buf,
                                         RUN->msg_bytes);
            MPI_CHECK(MPI_Barrier(ctx->comm));
        }

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_CHECK(MPI_Accumulate(BENCH->op_buf, RUN->elem_count, RUN->dtype,
                                     1, BENCH->disp, RUN->elem_count,
                                     RUN->dtype, MPI_SUM, RUN->win_handle));
            MPI_OP_POST_SYNC(RUN->win_handle);
            MPI_PING(RUN->group, RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
            }
        }
        if (options.validate) {
            MPI_CHECK(MPI_Barrier(ctx->comm));
            MPI_CHECK(MPI_Recv(&(RUN->validation_error_flag), 1, MPI_INT, 1, 0,
                               ctx->comm, MPI_STATUS_IGNORE));
        }
        RUN->t_stop = MPI_Wtime();
        MPI_LOOP_UNLOCK(RUN->win_handle);
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));
        if (options.validate) {
            atomic_data_validation_setup(RUN->dtype, ctx->rank, BENCH->win_base,
                                         RUN->msg_bytes);
            MPI_CHECK(MPI_Barrier(ctx->comm));
        }
        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }

            MPI_NON_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_NON_OP_POST_SYNC(RUN->win_handle);
            MPI_ACC_PONG(RUN->group, RUN->win_handle);
        }
        if (options.validate) {
            MPI_CHECK(MPI_Barrier(ctx->comm));
            atomic_data_validation_check(RUN->dtype, MPI_SUM, ctx->rank,
                                         BENCH->win_base, NULL, RUN->msg_bytes,
                                         1, 0, &(RUN->validation_error_flag));
            MPI_CHECK(MPI_Send(&(RUN->validation_error_flag), 1, MPI_INT, 0, 0,
                               ctx->comm));
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}

void OMB_CAS_LAT_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));
        MPI_LOOP_LOCK(RUN->win_handle);

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                if (options.validate) {
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->alt_op_buf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->tbuf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->cbuf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->alt_win_base,
                                                 options.max_message_size);
                }
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_CHECK(MPI_Compare_and_swap(BENCH->alt_op_buf, BENCH->cbuf,
                                           BENCH->tbuf, RUN->dtype, 1,
                                           BENCH->disp, RUN->win_handle));
            MPI_OP_POST_SYNC(RUN->win_handle);
            MPI_PING(RUN->group, RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
            }
        }
        RUN->t_stop = MPI_Wtime();
        MPI_LOOP_UNLOCK(RUN->win_handle);
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }
            if (i >= options.skip && options.validate) {
                atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                             BENCH->alt_op_buf,
                                             options.max_message_size);
                atomic_data_validation_setup(RUN->dtype, ctx->rank, BENCH->tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(RUN->dtype, ctx->rank, BENCH->cbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                             BENCH->alt_win_base,
                                             options.max_message_size);
            }

            MPI_NON_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_NON_OP_POST_SYNC(RUN->win_handle);
            MPI_CAS_PONG(RUN->group, RUN->win_handle);

            if (i >= options.skip && options.validate) {
                atomic_data_validation_check(RUN->dtype, (MPI_Op)-1, ctx->rank,
                                             BENCH->alt_win_base, BENCH->tbuf,
                                             options.max_message_size, 1, 1,
                                             &(RUN->validation_error_flag));
            }
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}

void OMB_FOP_LAT_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));
        MPI_LOOP_LOCK(RUN->win_handle);

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                if (options.validate) {
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->alt_op_buf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->tbuf,
                                                 options.max_message_size);
                    atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                                 BENCH->alt_win_base,
                                                 options.max_message_size);
                }
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_CHECK(MPI_Fetch_and_op(BENCH->alt_op_buf, BENCH->tbuf,
                                       RUN->dtype, 1, BENCH->disp, MPI_SUM,
                                       RUN->win_handle));
            MPI_OP_POST_SYNC(RUN->win_handle);
            MPI_PING(RUN->group, RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
            }
        }
        RUN->t_stop = MPI_Wtime();
        MPI_LOOP_UNLOCK(RUN->win_handle);
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }
            if (i >= options.skip && options.validate) {
                atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                             BENCH->alt_op_buf,
                                             options.max_message_size);
                atomic_data_validation_setup(RUN->dtype, ctx->rank, BENCH->tbuf,
                                             options.max_message_size);
                atomic_data_validation_setup(RUN->dtype, ctx->rank,
                                             BENCH->alt_win_base,
                                             options.max_message_size);
            }

            MPI_NON_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_NON_OP_POST_SYNC(RUN->win_handle);
            MPI_FOP_PONG(RUN->group, RUN->win_handle);

            if (i >= options.skip && options.validate) {
                atomic_data_validation_check(RUN->dtype, MPI_SUM, ctx->rank,
                                             BENCH->alt_win_base, BENCH->tbuf,
                                             options.max_message_size, 1, 1,
                                             &(RUN->validation_error_flag));
            }
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}

void OMB_GET_ACC_LATENCY_SYNC_SYM(omb_env_t *ctx)
{
    int i, j;

    MPI_CHECK(MPI_Barrier(ctx->comm));

    if (0 == ctx->rank) {
        RUN->destrank = 1;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));
        MPI_LOOP_LOCK(RUN->win_handle);

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
                RUN->t_start = MPI_Wtime();
            }
            if (i >= options.skip) {
                RUN->t_graph_start = MPI_Wtime();
            }

            MPI_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_CHECK(MPI_Get_accumulate(
                BENCH->op_buf, RUN->msg_bytes, MPI_CHAR, BENCH->cbuf,
                RUN->msg_bytes, MPI_CHAR, RUN->destrank, BENCH->disp,
                RUN->msg_bytes, MPI_CHAR, MPI_SUM, RUN->win_handle));
            MPI_OP_POST_SYNC(RUN->win_handle);
            MPI_PING(RUN->group, RUN->win_handle);

            if (i >= options.skip) {
                RUN->t_graph_end = MPI_Wtime();
                if (options.omb_tail_lat) {
                    BENCH->measurement_samples[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
                if (options.graph) {
                    BENCH->graph_data->data[i - options.skip] =
                        (RUN->t_graph_end - RUN->t_graph_start) * 1.0e6;
                }
            }
        }
        RUN->t_stop = MPI_Wtime();
        MPI_LOOP_UNLOCK(RUN->win_handle);
        RUN->timer_sec = RUN->t_stop - RUN->t_start;
    } else {
        RUN->destrank = 0;
        MPI_CHECK(MPI_Group_incl(RUN->comm_group, 1, &(RUN->destrank),
                                 &(RUN->group)));

        for (i = 0; i < options.skip + options.iterations; i++) {
            if (i == options.skip) {
                omb_papi_start(&(BENCH->papi_eventset));
            }

            MPI_NON_OP_PRE_SYNC(RUN->group, RUN->win_handle);
            MPI_NON_OP_POST_SYNC(RUN->win_handle);
            MPI_GET_ACC_PONG(RUN->group, RUN->win_handle);
        }
    }

    MPI_CHECK(MPI_Barrier(ctx->comm));
    MPI_CHECK(MPI_Group_free(&(RUN->group)));
}
