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

void init_persistent_pt2pt(omb_env_t *ctx);
void free_persistent_pt2pt(omb_env_t *ctx);
void partitioned_latency_overlap(omb_env_t *ctx, int iteration);
void omb_sender_core(omb_env_t *ctx, int i);
void omb_receiver_core(omb_env_t *ctx, int i);
