/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#ifndef _OMB_COLL_ENV_H_
#define _OMB_COLL_ENV_H_

#include "omb_general_util.h"

/* ---- per-benchmark persistent state & knobs ---- */
typedef struct omb_bench {
    /* Metadata */
    enum test_domain domain;
    enum test_type type;
    enum test_subtype subtype;
    const char *header;

    /* Macro defined calls */
    omb_env_ret_fn mpi_coll_init;
    omb_env_fn latency_step;
    omb_env_fn overlap_step;

    /* Type Data */
    MPI_Datatype dtype_list[OMB_NUM_DATATYPES];
    omb_buffer_sizes_t buffer_sizes;

    /* Irregular Info */
    int *recv_counts, *send_counts;
    int *recv_displs, *send_displs;
    MPI_Aint *recv_displs_bytes, *send_displs_bytes;
    MPI_Datatype *send_types, *recv_types;
    int portion;

    /* Buffers */
    char *sendbuf, *recvbuf;

    /* Performance Data */
    omb_graph_options_t graph_options;
    omb_graph_data_t *graph_data;
    int papi_eventset;
    double *latency_us_samples;

    /* Neighborhood State */
    double time_gca_total_us;
    MPI_Comm dist_graph_comm;
    int indegree, outdegree;
    int *sources, *sourceweights;
    int *destinations, *destweights;

    /* Request/Status */
    MPI_Request request;
    MPI_Status status;

    /* Runtime Feature Flags */
    bool is_nonblocking;
    bool is_persistent;
    bool is_neighborhood;
    bool irregular_args;
    bool irregular_block;
    bool is_generalized_coll;

    /* Flags Set During omb_bench_init() */
    bool use_byte_displs;
} omb_bench_t;

/* ---- per-run (iteration) scratch ---- */
typedef struct omb_run {
    int errors, local_errors;
    size_t msg_bytes, elem_count;
    int root_rank;

    /* dynamic buffers */
    char *sendbuf_active, *recvbuf_active;

    /* Timing / Accumulation */
    double elapsed_sec, avg_latency_us, t_start, t_stop;
    double compute_time_sec, compute_time_total_sec, latency_sec;
    double test_time, test_total_sec;
    double wait_time, init_time;
    double init_total_sec, wait_total_sec;
    double comm_avg_latency_sec;

    struct omb_stat_t latency_stats;

    /* Type Metadata */
    MPI_Datatype dtype;
    int dtype_index, dtype_size, dtype_name_len;
    char dtype_name[OMB_DATATYPE_STR_MAX_LEN];
    size_t ddt_transmit_size;
} omb_run_t;

#endif /* _OMB_COLL_ENV_H */
