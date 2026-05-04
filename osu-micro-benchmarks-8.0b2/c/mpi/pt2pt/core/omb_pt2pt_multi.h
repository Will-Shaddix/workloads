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

void omb_handle_multiprocess(omb_env_t *ctx);
int omb_handle_multithread(omb_env_t *ctx, void *(*threaded_entry)(void *));
void omb_sync_inter_threads(omb_env_t *ctx);
