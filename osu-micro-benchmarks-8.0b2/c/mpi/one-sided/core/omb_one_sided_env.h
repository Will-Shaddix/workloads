/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#ifndef _OMB_ONE_SIDED_ENV_H_
#define _OMB_ONE_SIDED_ENV_H_

#include "omb_general_util.h"

enum rma_benchmark_type {
    PUT,
    GET,
    ACCUMULATE,
    COMPARE_AND_SWAP,
    FETCH_AND_OP,
    GET_ACCUMULATE
};

typedef struct omb_bench {
    /* Metadata */
    enum test_domain domain;
    enum test_type type;
    enum test_subtype subtype;
    const char *header;

    /* Type data */
    MPI_Datatype dtype_list[OMB_NUM_DATATYPES];
    omb_buffer_sizes_t buffer_sizes;
    int ntypes;

    /* Buffers */
    char *op_buf, *win_base, *rbuf;
    uint64_t *alt_op_buf, *tbuf, *cbuf, *alt_win_base;
    MPI_Aint disp;

    /* Performance Data */
    omb_graph_options_t graph_options;
    omb_graph_data_t *graph_data;
    int papi_eventset;
    double *measurement_samples;

    /* Benchmark operation */
    int (*op_ptr)(const void *origin_addr, int origin_count,
                  MPI_Datatype origin_datatype, int target_rank,
                  MPI_Aint target_disp, int target_count,
                  MPI_Datatype target_datatype, MPI_Win win);

    enum rma_benchmark_type benchmark_type;
    double latency_factor;
} omb_bench;

typedef struct omb_run {
    int errors, local_errors;
    size_t msg_bytes, elem_count;
    int validation_error_flag;

    /* Timing / Accumulation */
    double t_start, t_stop, timer_sec;
    double t_graph_start, t_graph_end;
    struct omb_stat_t latency_stats;

    /* MPI Group */
    MPI_Group comm_group, group;
    int destrank;

    /* Window Data */
    MPI_Win win_handle;
    MPI_Aint sdisp_remote;
    MPI_Aint sdisp_local;
    MPI_Status reqstat;

    /* Type Metadata */
    MPI_Datatype dtype;
    int dtype_index, dtype_size, dtype_name_len;
    char dtype_name[OMB_DATATYPE_STR_MAX_LEN];
} omb_run;

#endif /* _OMB_ONESIDED_ENV_H */
