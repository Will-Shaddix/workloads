/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_coll_env.h"

int omb_alloc_benchmark_buffers(omb_env_t *ctx);
void omb_free_benchmark_buffers(omb_env_t *ctx);

void omb_set_buffer_sizes(omb_env_t *ctx);

void omb_apply_mpi_in_place(omb_env_t *ctx);

void omb_prepare_validation_buffers(omb_env_t *ctx, int iteration);
void omb_accumulate_local_errors(omb_env_t *ctx, int iteration);
