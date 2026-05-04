/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_pt2pt_env.h"

int omb_bench_init(omb_env_t *ctx);

void omb_select_active_dtype(omb_env_t *ctx);
void omb_print_dtype_and_header(omb_env_t *ctx);

size_t omb_compute_iters_for_msg_size(omb_env_t *ctx);
void omb_prepare_graph_for_msg_size(omb_env_t *ctx);

void omb_handle_ddt_size(omb_env_t *ctx);

void omb_plot_graphs(omb_env_t *ctx);
void omb_teardown_benchmark(omb_env_t *ctx);

int omb_check_and_report_errors(omb_env_t *ctx);

void omb_measure_kernel_lo(omb_env_t *ctx);
