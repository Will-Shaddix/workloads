/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_pt2pt_execution.h"

#define PING_TAG 1
#define PONG_TAG 2

static double iteration_latency(omb_env_t *ctx)
{
    if ((options.src == 'M' && options.MMsrc == 'D') &&
        (options.dst == 'M' && options.MMdst == 'D')) {
        return (RUN->t_stop - RUN->t_start) -
               (2 * RUN->t_lo * options.window_size);
    } else if ((options.src == 'M' && options.MMsrc == 'D') ||
               (options.dst == 'M' && options.MMdst == 'D')) {
        return (RUN->t_stop - RUN->t_start) - (RUN->t_lo * options.window_size);
    } else {
        return (RUN->t_stop - RUN->t_start);
    }
}

static void update_measurements(omb_env_t *ctx, int iteration)
{
    double metric, iteration_sec;
    size_t transmit_bytes =
        options.omb_enable_ddt ? RUN->ddt_transmit_size : RUN->msg_bytes;

    transmit_bytes =
        options.omb_enable_ddt ? RUN->ddt_transmit_size : RUN->msg_bytes;

    /* All tests need iteration latency */
    iteration_sec = iteration_latency(ctx);

    switch (BENCH->type) {
        case TYPE_LATENCY:
            metric = (iteration_sec * 1e6) / 2.0;
            break;
        case TYPE_BANDWIDTH:
            metric =
                (transmit_bytes * options.window_size) / (iteration_sec * 1e6);
            break;
        case TYPE_BI_BANDWIDTH:
            metric = (transmit_bytes * options.window_size * 2) /
                     (iteration_sec * 1e6);
            break;
        default:
            printf("Unknown type '%d' in update_measurements\n",
                   options.subtype);
    }

    /* Measurement Guide
     * Latency Units: avg ping-pong us
     * Bandwidth Units: MB/s
     */

    RUN->metric_total_rank += metric;
    RUN->t_timer_total += iteration_sec;

    if (options.omb_tail_lat) {
        BENCH->measurement_samples[iteration - options.skip] = metric;
    }
    if (options.graph) {
        BENCH->graph_data->data[iteration - options.skip] = metric;
    }
}

#define TOUCH_SRC(_op) /* Empty */
#define TOUCH_DST(_op) /* Empty */

#ifdef _ENABLE_CUDA_KERNEL_
#undef TOUCH_SRC
#define TOUCH_SRC(_op)                                                         \
    if (options.buf_num == MULTIPLE) {                                         \
        touch_managed_src_window(BENCH->multiple_sendbuf, RUN->msg_bytes,      \
                                 options.window_size, _op);                    \
    } else {                                                                   \
        touch_managed_src_no_window(BENCH->sendbuf, RUN->msg_bytes, _op);      \
    }

#undef TOUCH_DST
#define TOUCH_DST(_op)                                                         \
    if (options.buf_num == MULTIPLE) {                                         \
        touch_managed_dst_window(BENCH->multiple_recvbuf, RUN->msg_bytes,      \
                                 options.window_size, _op);                    \
    } else {                                                                   \
        touch_managed_dst_no_window(BENCH->recvbuf, RUN->msg_bytes, _op);      \
    }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */

#define SEND(_comm, _dest, _tag, _elem_count, _dtype, ...)                     \
    if (SINGLE == options.buf_num) {                                           \
        MPI_CHECK(MPI_Send(BENCH->sendbuf, _elem_count, _dtype, _dest, _tag,   \
                           _comm));                                            \
    } else {                                                                   \
        /* Variadic stuff here for optional buffer index */                    \
        MPI_CHECK(MPI_Send(BENCH->multiple_sendbuf[(                           \
                               sizeof(#__VA_ARGS__) > 1 ? __VA_ARGS__ : 0)],   \
                           _elem_count, _dtype, _dest, _tag, ctx->comm));      \
    }

#define RECV(_comm, _src, _tag, _elem_count, _dtype, ...)                      \
    if (SINGLE == options.buf_num) {                                           \
        MPI_CHECK(MPI_Recv(BENCH->recvbuf, _elem_count, _dtype, _src, _tag,    \
                           _comm, &reqstat[0]));                               \
    } else {                                                                   \
        /* Variadic stuff here for optional buffer index */                    \
        MPI_CHECK(MPI_Recv(BENCH->multiple_recvbuf[(                           \
                               sizeof(#__VA_ARGS__) > 1 ? __VA_ARGS__ : 0)],   \
                           _elem_count, _dtype, _src, _tag, ctx->comm,         \
                           &reqstat[0]));                                      \
    }

#define ISEND(_comm, _dest, _tag, _elem_count, _dtype, _indx)                  \
    if (SINGLE == options.buf_num) {                                           \
        MPI_CHECK(MPI_Isend(BENCH->sendbuf, _elem_count, _dtype, _dest, _tag,  \
                            _comm, send_request + _indx));                     \
    } else {                                                                   \
        MPI_CHECK(MPI_Isend(BENCH->multiple_sendbuf[_indx], _elem_count,       \
                            _dtype, _dest, _tag, _comm,                        \
                            send_request + _indx));                            \
    }

#define IRECV(_comm, _src, _tag, _elem_count, _dtype, _indx)                   \
    if (SINGLE == options.buf_num) {                                           \
        MPI_CHECK(MPI_Irecv(BENCH->recvbuf, _elem_count, _dtype, _src, _tag,   \
                            _comm, recv_request + _indx));                     \
    } else {                                                                   \
        /* Variadic stuff here for optional buffer index */                    \
        MPI_CHECK(MPI_Irecv(BENCH->multiple_recvbuf[_indx], _elem_count,       \
                            _dtype, _src, _tag, _comm, recv_request + _indx)); \
    }

/* Latency */
static void latency_sender(omb_env_t *ctx)
{
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    SEND(ctx->comm, BENCH->partner_rank, PING_TAG, RUN->elem_count, RUN->dtype);
    RECV(ctx->comm, BENCH->partner_rank, PONG_TAG, RUN->elem_count, RUN->dtype);

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void latency_receiver(omb_env_t *ctx)
{
    TOUCH_DST(ADD);
    RECV(ctx->comm, BENCH->partner_rank, PING_TAG, RUN->elem_count, RUN->dtype);

    TOUCH_DST(SUB);
    SEND(ctx->comm, BENCH->partner_rank, PONG_TAG, RUN->elem_count, RUN->dtype);
}

static void latency_sender_persistent(omb_env_t *ctx)
{
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    MPI_CHECK(MPI_Start(send_request));
    MPI_CHECK(MPI_Wait(send_request, reqstat));
    MPI_CHECK(MPI_Start(recv_request));
    MPI_CHECK(MPI_Wait(recv_request, reqstat));

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void latency_receiver_persistent(omb_env_t *ctx)
{
    TOUCH_DST(ADD);

    MPI_CHECK(MPI_Start(recv_request));
    MPI_CHECK(MPI_Wait(recv_request, reqstat));
    MPI_CHECK(MPI_Start(send_request));
    MPI_CHECK(MPI_Wait(send_request, reqstat));

    TOUCH_DST(SUB);
}

static void partitioned_latency_sender(omb_env_t *ctx)
{
#ifdef _MPI4_PARTITION_PT2PT_
    int p;
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    MPI_Start(&send_request[0]);
    for (p = 0; p < options.num_partitions; ++p) {
        MPI_Pready(p, send_request[0]);
    }
    MPI_Wait(&send_request[0], reqstat);

    MPI_Start(&recv_request[0]);
    MPI_Wait(&recv_request[0], reqstat);

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
#endif /* _MPI4_PARTITION_PT2PT_ */
}

static void partitioned_latency_receiver(omb_env_t *ctx)
{
#ifdef _MPI4_PARTITION_PT2PT_
    int p;
    TOUCH_DST(ADD);

    MPI_Start(&recv_request[0]);
    MPI_Wait(&recv_request[0], reqstat);

    MPI_Start(&send_request[0]);
    for (p = 0; p < options.num_partitions; ++p) {
        MPI_Pready(p, send_request[0]);
    }
    MPI_Wait(&send_request[0], reqstat);

    TOUCH_DST(SUB);
#endif /* _MPI4_PARTITION_PT2PT_ */
}

void partitioned_latency_overlap(omb_env_t *ctx, int iteration)
{
#ifdef _MPI4_PARTITION_PT2PT_
    int p, j;
    double init_time, tcomp, tmp_time, wait_time, test_time;

    MPI_Request *first_req = BENCH->is_sender ? send_request : recv_request;
    MPI_Request *second_req = BENCH->is_sender ? recv_request : send_request;

    for (j = 0; j <= options.warmup_validation; j++) {
        RUN->t_start = MPI_Wtime();

        TOUCH_SRC(ADD);

        if (BENCH->is_sender) {
            MPI_Start(send_request);
            for (p = 0; p < options.num_partitions; ++p) {
                MPI_Pready(p, send_request[0]);
            }
        } else {
            MPI_Start(recv_request);
        }
        init_time = MPI_Wtime() - RUN->t_start;

        tcomp = MPI_Wtime();
        test_time = dummy_compute(RUN->latency_in_secs, first_req);
        tcomp = MPI_Wtime() - tcomp;

        wait_time = MPI_Wtime();
        MPI_Wait(first_req, reqstat);
        wait_time = MPI_Wtime() - wait_time;

        tmp_time = MPI_Wtime();
        if (BENCH->is_sender) {
            MPI_Start(recv_request);
        } else {
            MPI_Start(send_request);
            for (p = 0; p < options.num_partitions; ++p) {
                MPI_Pready(p, send_request[0]);
            }
        }
        init_time += MPI_Wtime() - tmp_time;

        tmp_time = MPI_Wtime();
        test_time += dummy_compute(RUN->latency_in_secs, second_req);
        tcomp += MPI_Wtime() - tmp_time;

        tmp_time = MPI_Wtime();
        MPI_Wait(second_req, reqstat);
        wait_time += MPI_Wtime() - tmp_time;

        TOUCH_SRC(SUB);

        RUN->t_stop = MPI_Wtime();
        MPI_CHECK(MPI_Barrier(ctx->comm));
        if (iteration >= options.skip && j == options.warmup_validation) {
            RUN->t_init_total += init_time / 2;
            RUN->t_wait_total += wait_time / 2;
            RUN->t_comp_total += tcomp / 2;
            RUN->t_test_total += test_time / 2;
            RUN->t_timer_total += (RUN->t_stop - RUN->t_start) / 2;
        }
    }
#endif /* _MPI4_PARTITION_PT2PT_ */
}

/* Bandwidth */
static void bandwidth_sender(omb_env_t *ctx)
{
    int i;
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    for (i = 0; i < options.window_size; i++) {
        ISEND(ctx->comm, BENCH->partner_rank, PING_TAG, RUN->elem_count,
              RUN->dtype, i);
    }
    MPI_CHECK(MPI_Waitall(options.window_size, send_request, reqstat));

    RECV(ctx->comm, BENCH->partner_rank, PONG_TAG, 1, MPI_CHAR);

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void bandwidth_receiver(omb_env_t *ctx)
{
    int i;

    TOUCH_DST(ADD);

    for (i = 0; i < options.window_size; i++) {
        IRECV(ctx->comm, BENCH->partner_rank, PING_TAG, RUN->elem_count,
              RUN->dtype, i);
    }
    MPI_CHECK(MPI_Waitall(options.window_size, recv_request, reqstat));

    TOUCH_DST(SUB);

    SEND(ctx->comm, BENCH->partner_rank, PONG_TAG, 1, MPI_CHAR);
}

static void congestion_bandwidth_sender(omb_env_t *ctx)
{
    int i, n;
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    for (n = 0; n < FAN->queue_len; n++) {
        for (i = 0; i < options.window_size; i++) {
            ISEND(FAN->fan_comm, FAN->ranks_queue[n], PING_TAG, RUN->elem_count,
                  RUN->dtype, i + (n * options.window_size));
        }
    }

    MPI_CHECK(MPI_Waitall(FAN->queue_len * options.window_size, send_request,
                          reqstat));

    for (n = 0; n < FAN->queue_len; n++) {
        RECV(FAN->fan_comm, FAN->ranks_queue[n], PONG_TAG, 1, MPI_CHAR);
    }

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void congestion_bandwidth_receiver(omb_env_t *ctx)
{
    int i, n;

    TOUCH_DST(ADD);

    for (n = 0; n < FAN->queue_len; n++) {
        for (i = 0; i < options.window_size; i++) {
            IRECV(FAN->fan_comm, FAN->ranks_queue[n], PING_TAG, RUN->elem_count,
                  RUN->dtype, i + (n * options.window_size));
        }
    }

    MPI_CHECK(MPI_Waitall(FAN->queue_len * options.window_size, recv_request,
                          reqstat));

    for (n = 0; n < FAN->queue_len; n++) {
        SEND(FAN->fan_comm, FAN->ranks_queue[n], PONG_TAG, 1, MPI_CHAR);
    }

    TOUCH_DST(SUB);
}

static void bandwidth_sender_persistent(omb_env_t *ctx)
{
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    MPI_CHECK(MPI_Startall(options.window_size, send_request));
    MPI_CHECK(MPI_Waitall(options.window_size, send_request, reqstat));

    RECV(ctx->comm, BENCH->partner_rank, PONG_TAG, 1, MPI_CHAR);

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void bandwidth_receiver_persistent(omb_env_t *ctx)
{
    TOUCH_DST(ADD);

    MPI_CHECK(MPI_Startall(options.window_size, recv_request));
    MPI_CHECK(MPI_Waitall(options.window_size, recv_request, reqstat));

    TOUCH_DST(SUB);

    SEND(ctx->comm, BENCH->partner_rank, PONG_TAG, 1, MPI_CHAR);
}

/* Bi-Bandwidth */
static void bi_bandwidth_sender(omb_env_t *ctx)
{
    int i;

    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    for (i = 0; i < options.window_size; i++) {
        ISEND(ctx->comm, BENCH->partner_rank, PING_TAG, RUN->elem_count,
              RUN->dtype, i);
        IRECV(ctx->comm, BENCH->partner_rank, PONG_TAG, RUN->elem_count,
              RUN->dtype, i);
    }

    MPI_CHECK(MPI_Waitall(options.window_size, send_request, reqstat));
    MPI_CHECK(MPI_Waitall(options.window_size, recv_request, reqstat));

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void bi_bandwidth_receiver(omb_env_t *ctx)
{
    int i;

    TOUCH_DST(ADD);

    for (i = 0; i < options.window_size; i++) {
        IRECV(ctx->comm, BENCH->partner_rank, PING_TAG, RUN->elem_count,
              RUN->dtype, i);
        ISEND(ctx->comm, BENCH->partner_rank, PONG_TAG, RUN->elem_count,
              RUN->dtype, i);
    }

    MPI_CHECK(MPI_Waitall(options.window_size, recv_request, reqstat));

    TOUCH_DST(SUB);

    MPI_CHECK(MPI_Waitall(options.window_size, send_request, reqstat));
}

static void bi_bandwidth_sender_persistent(omb_env_t *ctx)
{
    RUN->t_start = MPI_Wtime();

    TOUCH_SRC(ADD);

    MPI_CHECK(MPI_Startall(options.window_size, send_request));
    MPI_CHECK(MPI_Startall(options.window_size, recv_request));

    MPI_CHECK(MPI_Waitall(options.window_size, send_request, reqstat));
    MPI_CHECK(MPI_Waitall(options.window_size, recv_request, reqstat));

    TOUCH_SRC(SUB);

    RUN->t_stop = MPI_Wtime();
}

static void bi_bandwidth_receiver_persistent(omb_env_t *ctx)
{
    TOUCH_DST(ADD);

    MPI_CHECK(MPI_Startall(options.window_size, recv_request));
    MPI_CHECK(MPI_Startall(options.window_size, send_request));

    MPI_CHECK(MPI_Waitall(options.window_size, recv_request, reqstat));
    MPI_CHECK(MPI_Waitall(options.window_size, send_request, reqstat));

    TOUCH_DST(SUB);
}

/* Senders calculate and update metrics */
void omb_sender_core(omb_env_t *ctx, int i)
{
    int j;
    omb_env_fn sender_fn = NULL;

    switch (BENCH->type) {
        case TYPE_LATENCY:
            if (PART_LAT == BENCH->subtype) {
                sender_fn = partitioned_latency_sender;
            } else if (BENCH->is_persistent) {
                sender_fn = latency_sender_persistent;
            } else {
                sender_fn = latency_sender;
            }
            break;
        case TYPE_BANDWIDTH:
            if (CONG_BW == BENCH->subtype) {
                sender_fn = congestion_bandwidth_sender;
            } else if (BENCH->is_persistent) {
                sender_fn = bandwidth_sender_persistent;
            } else {
                sender_fn = bandwidth_sender;
            }
            break;
        case TYPE_BI_BANDWIDTH:
            if (BENCH->is_persistent) {
                sender_fn = bi_bandwidth_sender_persistent;
            } else {
                sender_fn = bi_bandwidth_sender;
            }
            break;
        default:
            fprintf(stderr, "Unexpected type '%d' in omb_sender_core\n",
                    options.type);
            fflush(stderr);
            return;
    }

    for (j = 0; j <= options.warmup_validation; j++) {
        sender_fn(ctx);

        TOUCH_SRC(ADD);

        if (i >= options.skip && j == options.warmup_validation) {
            update_measurements(ctx, i);
        }

        if (options.src == 'M' && options.MMsrc == 'D' && options.validate) {
            TOUCH_SRC(SUB);
        }
    }
}

void omb_receiver_core(omb_env_t *ctx, int i)
{
    int j;
    omb_env_fn receiver_fn = NULL;

    switch (BENCH->type) {
        case TYPE_LATENCY:
            if (PART_LAT == BENCH->subtype) {
                receiver_fn = partitioned_latency_receiver;
            } else if (BENCH->is_persistent) {
                receiver_fn = latency_receiver_persistent;
            } else {
                receiver_fn = latency_receiver;
            }
            break;
        case TYPE_BANDWIDTH:
            if (CONG_BW == BENCH->subtype) {
                receiver_fn = congestion_bandwidth_receiver;
            } else if (BENCH->is_persistent) {
                receiver_fn = bandwidth_receiver_persistent;
            } else {
                receiver_fn = bandwidth_receiver;
            }
            break;
        case TYPE_BI_BANDWIDTH:
            if (BENCH->is_persistent) {
                receiver_fn = bi_bandwidth_receiver_persistent;
            } else {
                receiver_fn = bi_bandwidth_receiver;
            }
            break;
        default:
            fprintf(stderr, "Unexpected type '%d' in omb_receiver_core\n",
                    options.type);
            fflush(stderr);
            return;
    }

    for (j = 0; j <= options.warmup_validation; j++) {
        receiver_fn(ctx);
    }
}

void init_persistent_pt2pt(omb_env_t *ctx)
{
    int i;

    if (BENCH->subtype == PART_LAT) {
#ifdef _MPI4_PARTITION_PT2PT_
        if (MULTIPLE == options.buf_num) {
            for (i = 0; i < options.window_size; i++) {
                MPI_CHECK(MPI_Psend_init(
                    BENCH->multiple_sendbuf[i], options.num_partitions,
                    RUN->elem_count, RUN->dtype, BENCH->partner_rank, PING_TAG,
                    ctx->comm, MPI_INFO_NULL, &send_request[i]));
                MPI_CHECK(MPI_Precv_init(
                    BENCH->multiple_recvbuf[i], options.num_partitions,
                    RUN->elem_count, RUN->dtype, BENCH->partner_rank, PING_TAG,
                    ctx->comm, MPI_INFO_NULL, &recv_request[i]));
            }
        } else if (SINGLE == options.buf_num) {
            for (i = 0; i < options.window_size; i++) {
                MPI_CHECK(MPI_Psend_init(
                    BENCH->sendbuf, options.num_partitions, RUN->elem_count,
                    RUN->dtype, BENCH->partner_rank, PING_TAG, ctx->comm,
                    MPI_INFO_NULL, &send_request[i]));
                MPI_CHECK(MPI_Precv_init(
                    BENCH->recvbuf, options.num_partitions, RUN->elem_count,
                    RUN->dtype, BENCH->partner_rank, PING_TAG, ctx->comm,
                    MPI_INFO_NULL, &recv_request[i]));
            }
        }
#endif /* _MPI4_PARTITION_PT2PT_ */
    } else {
        if (MULTIPLE == options.buf_num) {
            for (i = 0; i < options.window_size; i++) {
                MPI_CHECK(MPI_Send_init(BENCH->multiple_sendbuf[i],
                                        RUN->elem_count, RUN->dtype,
                                        BENCH->partner_rank, PING_TAG,
                                        ctx->comm, &send_request[i]));
                MPI_CHECK(MPI_Recv_init(BENCH->multiple_recvbuf[i],
                                        RUN->elem_count, RUN->dtype,
                                        BENCH->partner_rank, PING_TAG,
                                        ctx->comm, &recv_request[i]));
            }
        } else if (SINGLE == options.buf_num) {
            for (i = 0; i < options.window_size; i++) {
                MPI_CHECK(MPI_Send_init(BENCH->sendbuf, RUN->elem_count,
                                        RUN->dtype, BENCH->partner_rank,
                                        PING_TAG, ctx->comm, &send_request[i]));
                MPI_CHECK(MPI_Recv_init(BENCH->recvbuf, RUN->elem_count,
                                        RUN->dtype, BENCH->partner_rank,
                                        PING_TAG, ctx->comm, &recv_request[i]));
            }
        }
    }
}

void free_persistent_pt2pt(omb_env_t *ctx)
{
    int i;
    for (i = 0; i < options.window_size; i++) {
        MPI_Request_free(&send_request[i]);
        MPI_Request_free(&recv_request[i]);
    }
}
