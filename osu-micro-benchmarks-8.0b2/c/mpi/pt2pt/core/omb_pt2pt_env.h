/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#ifndef _OMB_PT2PT_ENV_H_
#define _OMB_PT2PT_ENV_H_

#include "omb_general_util.h"

#define OMB_NOT_PAIRED -1

typedef struct omb_fan_info {
    int *ranks_queue;
    int queue_len;

    /* Ranks with local rank N, from all nodes */
    MPI_Comm fan_comm;
} omb_fan_info_t;

typedef struct omb_multiprocessed {
    int num_processes_sender, is_child;
    pid_t sr_processes[MAX_NUM_PROCESSES];
} omb_multiprocessed_t;

typedef struct omb_env_tid {
    int id;
    omb_env_t *omb_env;
} omb_env_tid_t;

typedef struct omb_multithreaded {
    pthread_barrier_t intra_barrier;

    int num_threads_recv, num_threads_send, my_num_threads;

    pthread_t pthreads[MAX_NUM_THREADS];
    omb_env_tid_t contexts[MAX_NUM_THREADS];
} omb_multithreaded_t;

/* ---- per-benchmark persistent state & knobs ---- */
typedef struct omb_bench {
    /* Metadata */
    enum test_domain domain;
    enum test_type type;
    enum test_subtype subtype;
    const char *header;

    /* Process Partnering */
    int pair_count, partner_rank, is_sender;
    MPI_Comm sender_comm;

    /* Varied Windows */
    int window_sizes_count;

    /* Optional substructs */
    omb_multiprocessed_t *multiprocess;
    omb_multithreaded_t *multithread;
    omb_fan_info_t *fan_info;

    /* Type Data */
    MPI_Datatype dtype_list[OMB_NUM_DATATYPES];
    omb_buffer_sizes_t buffer_sizes;

    /* Buffers */
    char **multiple_sendbuf, **multiple_recvbuf;
    char *sendbuf, *recvbuf;

    /* Performance Data */
    omb_graph_options_t graph_options;
    omb_graph_data_t *graph_data;
    int papi_eventset;
    double *measurement_samples;

    /* Runtime Feature Flags */
    bool is_persistent;
    bool is_fan_out;
    bool is_paired;
} omb_bench_t;

/* ---- per-run (iteration) scratch ---- */
typedef struct omb_run {
    int errors, local_errors;
    size_t msg_bytes, elem_count;

    /* Timing / Accumulation */
    double t_start, t_stop, t_lo;
    double metric_total_rank, metric_total_comm;
    struct omb_stat_t metric_stats;

    double varied_window_metric_total_comm[WINDOW_SIZES_COUNT];

    /* Overlap Timing */
    double t_timer_total, t_comp_total, t_wait_total, t_init_total,
        t_test_total, latency_in_secs;

    /* Thread Indicators */
    bool is_pthread;
    int tid;

    /* Type Metadata */
    MPI_Datatype dtype;
    int dtype_index, dtype_size, dtype_name_len;
    char dtype_name[OMB_DATATYPE_STR_MAX_LEN];
    size_t ddt_transmit_size;
} omb_run_t;

#endif /* _OMB_PT2PT_ENV_H_ */
