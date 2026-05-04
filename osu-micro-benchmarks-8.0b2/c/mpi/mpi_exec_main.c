/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_general_util.h"

/* The entry symbol to invoke is provided per-target via -DOMB_ENTRY_SYM=... */
#ifndef OMB_ENTRY_SYM
#error                                                                         \
    "OMB_ENTRY_SYM must be defined to the entry function symbol (e.g., omb_entry_osu_allgather)"
#endif

extern int OMB_ENTRY_SYM(omb_env_t *ctx);

int main(int argc, char *argv[])
{
    omb_env_t ctx;
    int rc;

    omb_env_init(&ctx, &argc, &argv, OMB_OWNS_RESOURCES);
    rc = OMB_ENTRY_SYM(&ctx);
    omb_mpi_finalize(ctx.omb_init);
    if (!ctx.rank && EXIT_SUCCESS != rc && OMB_ERR_BAD_USAGE != rc) {
        fprintf(stderr, "%s exited with %d: %s\n", OMB_NAME, rc,
                omb_strerror(rc));
    }
    return normalize_exit_code(rc);
}
