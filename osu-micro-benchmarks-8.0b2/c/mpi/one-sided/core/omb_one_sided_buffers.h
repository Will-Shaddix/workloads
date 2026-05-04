/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_one_sided_env.h"

int omb_alloc_benchmark_buffers(omb_env_t *ctx);
int omb_alloc_run_buffers(omb_env_t *ctx);
void allocate_memory_get_acc_lat(omb_env_t *ctx);

void omb_allocate_one_sided_memory(omb_env_t *ctx);
void omb_free_one_sided_memory(omb_env_t *ctx);
