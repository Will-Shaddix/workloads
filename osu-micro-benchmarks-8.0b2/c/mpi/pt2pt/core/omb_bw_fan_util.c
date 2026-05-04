/*
 * Copyright (C) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_bw_fan_util.h"

int omb_fan_init(omb_env_t *ctx)
{
    int i;

    OMB_CHECK_AND_RET(omb_get_node_information(ctx, true));

    BENCH->is_sender = (BENCH->is_fan_out == !NODE.id);
    BENCH->pair_count = OMB_NOT_PAIRED;

    MPI_CHECK(
        MPI_Comm_split(ctx->comm, BENCH->is_sender, 0, &BENCH->sender_comm));

    MPI_CHECK(
        MPI_Comm_split(ctx->comm, NODE.local_rank, NODE.id, &FAN->fan_comm));

    if (!NODE.id) {
        /* Parent */
        FAN->ranks_queue = calloc(NODE.world_node_count - 1, sizeof(int));
        FAN->queue_len = NODE.world_node_count - 1;
        for (i = 0; i < FAN->queue_len; i++) {
            FAN->ranks_queue[i] = i + 1;
        }
    } else {
        /* Child */
        FAN->ranks_queue = calloc(1, sizeof(int));
        FAN->queue_len = 1;
        FAN->ranks_queue[0] = 0;
    }

    return EXIT_SUCCESS;
}

void omb_finalize_fan(omb_env_t *ctx)
{
    free(FAN->ranks_queue);
    MPI_CHECK(MPI_Comm_free(&FAN->fan_comm));
}
