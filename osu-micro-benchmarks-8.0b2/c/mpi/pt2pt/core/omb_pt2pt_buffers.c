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
#include "omb_pt2pt_multi.h"

static int init_pt2pt_validation_buffers(omb_env_t *ctx, void *s_buf,
                                         void *r_buf, int iter)
{
    int val = 0;
    void *temp_s_buffer, *temp_r_buffer;
    char buf_type = 'H';

    MALLOC_CHECK_AND_RET(temp_s_buffer, RUN->msg_bytes, ctx->rank);
    MALLOC_CHECK_AND_RET(temp_r_buffer, RUN->msg_bytes, ctx->rank);

    for (int i = 0; i < RUN->elem_count; i++) {
        val = (CHAR_VALIDATION_MULTIPLIER * (i + 1) + RUN->msg_bytes + iter);
        omb_assign_element_for_validation(temp_s_buffer, i, val, RUN->dtype);
        omb_assign_element_for_validation(temp_r_buffer, i, 0, RUN->dtype);
    }

    if (options.subtype == MBW_MR) {
        buf_type = (ctx->rank < options.pairs) ? options.src : options.dst;
    } else {
        buf_type = (ctx->rank == 0) ? options.src : options.dst;
    }

    switch (buf_type) {
        case 'H':
            memcpy(s_buf, temp_s_buffer, RUN->msg_bytes);
            memcpy(r_buf, temp_r_buffer, RUN->msg_bytes);
            break;
        case 'D':
        case 'M':
#ifdef _ENABLE_OPENACC_
            if (type == OPENACC) {
                size_t i;
                char *p = (char *)s_buf;
#pragma acc parallel loop deviceptr(p)
                for (i = 0; i < RUN->msg_bytes; i++) {
                    p[i] = temp_char_s_buffer[i];
                }
                p = (char *)r_buf;
#pragma acc parallel loop deviceptr(p)
                for (i = 0; i < RUN->msg_bytes; i++) {
                    p[i] = temp_char_r_buffer[i];
                }
                break;
            } else
#endif
#ifdef _ENABLE_CUDA_
            {
                CUDA_CHECK(cudaMemcpy(s_buf, temp_s_buffer, RUN->msg_bytes,
                                      cudaMemcpyHostToDevice));
                CUDA_CHECK(cudaMemcpy(r_buf, temp_r_buffer, RUN->msg_bytes,
                                      cudaMemcpyHostToDevice));
                CUDA_CHECK(cudaDeviceSynchronize());
            }
#endif
#ifdef _ENABLE_ROCM_
            {
                ROCM_CHECK(hipMemcpy(s_buf, temp_s_buffer, RUN->msg_bytes,
                                     hipMemcpyHostToDevice));
                ROCM_CHECK(hipMemcpy(r_buf, temp_r_buffer, RUN->msg_bytes,
                                     hipMemcpyHostToDevice));
                ROCM_CHECK(hipDeviceSynchronize());
            }
#endif
#ifdef _ENABLE_SYCL_
            {
                syclMemcpy(s_buf, temp_s_buffer, RUN->msg_bytes);
                syclMemcpy(r_buf, temp_r_buffer, RUN->msg_bytes);
                syclDeviceSynchronize();
            }
#endif
            break;
    }

    free(temp_s_buffer);
    free(temp_r_buffer);

    return EXIT_SUCCESS;
}

void omb_prepare_validation_buffers(omb_env_t *ctx, int iteration)
{
    int i;

    if (options.buf_num == MULTIPLE) {
        for (i = 0; i < options.window_size; i++) {
            init_pt2pt_validation_buffers(ctx, BENCH->multiple_sendbuf[i],
                                          BENCH->multiple_recvbuf[i],
                                          iteration + i);
        }
    } else {
        init_pt2pt_validation_buffers(ctx, BENCH->sendbuf, BENCH->recvbuf,
                                      iteration);
    }

    omb_sync_inter_threads(ctx);
}

void omb_update_validation_errors(omb_env_t *ctx, int iteration)
{
    int i;

    /* Unidirectional bandwidth sender can have no errors */
    if (BENCH->type == TYPE_BANDWIDTH && BENCH->is_sender) {
        RUN->errors = 0;
        return;
    }

    if (SINGLE == options.buf_num) {
        RUN->errors += validate_data(BENCH->recvbuf, RUN->msg_bytes, 1,
                                     options.accel, iteration, RUN->dtype);
    } else {
        for (i = 0; i < options.window_size; i++) {
            RUN->errors +=
                validate_data(BENCH->multiple_recvbuf[i], RUN->msg_bytes, 1,
                              options.accel, iteration + i, RUN->dtype);
        }
    }
}

static void enforce_cuda_buf_num()
{
#ifdef _ENABLE_CUDA_KERNEL_
    if (options.src == 'M' || options.dst == 'M') {
        if (options.buf_num == SINGLE) {
            fprintf(stderr, "Warning: Tests involving managed buffers will use"
                            " multiple buffers by default\n");
        }
        options.buf_num = MULTIPLE;
    }
#endif /* #ifdef _ENABLE_CUDA_KERNEL_ */
}

/* Buffer API */
int omb_alloc_benchmark_buffers(omb_env_t *ctx)
{
    if (options.buf_num == SINGLE) {
        BENCH->buffer_sizes.recvbuf_size = options.max_message_size;
        BENCH->buffer_sizes.sendbuf_size = options.max_message_size;
        OMB_CHECK_AND_RET(
            allocate_memory_pt2pt(&BENCH->sendbuf, &BENCH->recvbuf, ctx->rank));
    } else if (options.buf_num == MULTIPLE) {
        enforce_cuda_buf_num();

        BENCH->buffer_sizes.recvbuf_size = sizeof(char *) * options.window_size;
        BENCH->buffer_sizes.sendbuf_size = sizeof(char *) * options.window_size;

        MALLOC_CHECK_AND_RET(BENCH->multiple_sendbuf,
                             BENCH->buffer_sizes.sendbuf_size, ctx->rank);
        MALLOC_CHECK_AND_RET(BENCH->multiple_recvbuf,
                             BENCH->buffer_sizes.recvbuf_size, ctx->rank);
    }

    if (RUN->is_pthread) {
        return EXIT_SUCCESS;
    }

    if (options.omb_tail_lat) {
        MALLOC_CHECK_AND_RET(BENCH->measurement_samples,
                             options.iterations * sizeof(double), ctx->rank);
    }

    if (options.subtype == LAT_MP) {
        MALLOC_CHECK_AND_RET(BENCH->multiprocess, sizeof(*BENCH->multiprocess),
                             ctx->rank);
    } else if (options.subtype == LAT_MT) {
        MALLOC_CHECK_AND_RET(BENCH->multithread, sizeof(*BENCH->multithread),
                             ctx->rank);
    } else if (options.subtype == CONG_BW) {
        MALLOC_CHECK_AND_RET(BENCH->fan_info, sizeof(*BENCH->fan_info),
                             ctx->rank);
    }

    return EXIT_SUCCESS;
}

void omb_free_benchmark_buffers(omb_env_t *ctx)
{
    if (options.buf_num == SINGLE) {
        free_memory(BENCH->sendbuf, BENCH->recvbuf, ctx->rank);
    } else {
        free_memory(BENCH->multiple_sendbuf, BENCH->multiple_recvbuf,
                    ctx->rank);
    }

    if (RUN->is_pthread) {
        return;
    }

    omb_graph_free_data_buffers(&(BENCH->graph_options));
    omb_papi_free(&(BENCH->papi_eventset));
    if (options.omb_tail_lat) {
        free(BENCH->measurement_samples);
    }

    if (options.subtype == LAT_MP) {
        free(BENCH->multiprocess);
    } else if (options.subtype == LAT_MT) {
        free(BENCH->multithread);
    } else if (options.subtype == CONG_BW) {
        free(BENCH->fan_info);
    }
}

int omb_alloc_multiple_buffers(omb_env_t *ctx)
{
    int i = 0;
    for (i = 0; i < options.window_size; i++) {
        if (BENCH->is_paired) {
            OMB_CHECK_AND_RET(allocate_memory_pt2pt_mul_size(
                &(BENCH->multiple_sendbuf[i]), &(BENCH->multiple_recvbuf[i]),
                ctx->rank, BENCH->pair_count, RUN->msg_bytes));
        } else {
            OMB_CHECK_AND_RET(allocate_memory_pt2pt_size(
                &(BENCH->multiple_sendbuf[i]), &(BENCH->multiple_recvbuf[i]),
                ctx->rank, RUN->msg_bytes));
        }
    }

    return EXIT_SUCCESS;
}

void omb_set_pt2pt_buffers(omb_env_t *ctx)
{
    int i;
    if (options.buf_num == SINGLE) {
        set_buffer_pt2pt(BENCH->sendbuf, ctx->rank, options.accel, 'a',
                         RUN->msg_bytes);
        set_buffer_pt2pt(BENCH->recvbuf, ctx->rank, options.accel, 'b',
                         RUN->msg_bytes);
    } else {
        for (i = 0; i < options.window_size; i++) {
            set_buffer_pt2pt(BENCH->multiple_sendbuf[i], ctx->rank,
                             options.accel, 'a', RUN->msg_bytes);
            set_buffer_pt2pt(BENCH->multiple_recvbuf[i], ctx->rank,
                             options.accel, 'b', RUN->msg_bytes);
        }
    }
}

void omb_free_multiple_buffers(omb_env_t *ctx)
{
    int i;
    for (i = 0; i < options.window_size; i++) {
        if (BENCH->is_paired) {
            free_memory_pt2pt_mul(BENCH->multiple_sendbuf[i],
                                  BENCH->multiple_recvbuf[i], ctx->rank,
                                  BENCH->pair_count);
        } else {
            free_memory(BENCH->multiple_sendbuf[i], BENCH->multiple_recvbuf[i],
                        ctx->rank);
        }
    }
}
