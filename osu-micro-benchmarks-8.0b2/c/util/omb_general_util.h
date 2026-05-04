/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#ifndef _OMB_GENERAL_UTIL_H_
#define _OMB_GENERAL_UTIL_H_

#include "osu_util_mpi.h"
#include "omb_macros.h"
#include <errno.h>

/* Keep this in sync with omb_general_util.c:omb_strerror */
#define OMB_ERR_BAD_USAGE          200
#define OMB_ERR_HELP_MESSAGE       201
#define OMB_ERR_VERSION_MESSAGE    202
#define OMB_ERR_VALIDATION_FAILURE 203
#define OMB_ERR_CUDA_NOT_AVAIL     204
#define OMB_ERR_OPENACC_NOT_AVAIL  205
#define OMB_ERR_PROCESS_DIST       206
#define OMB_ERR_NOT_LAUNCHED       207
#define OMB_ERR_INVALID_SYNC       208

/* Extension of strerror, with support for the custom errnos above */
char *omb_strerror(int __errnum);
int normalize_exit_code(int rc);

/* Forward decs, defined in each domain */
struct omb_bench;
struct omb_run;

typedef enum {
    OMB_SHARES_RESOURCES = 0,
    OMB_OWNS_RESOURCES = 1,
} omb_resource_owner_t;

typedef struct node_info {
    char name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    int id;
    int ppn;
    int local_rank;
    int world_node_count;
} node_info_t;

/* -- top-level environment -- */
typedef struct omb_env {
    /* args */
    int argc;
    char **argv;

    /* Metadata */
    char *title;

    /* Ownership toggle */
    omb_resource_owner_t owns_mpi;

    /* MPI init/state */
    node_info_t node_info;
    omb_mpi_init_data omb_init;
    MPI_Comm comm;
    int rank, numprocs;

    /* nested contexts */
    struct omb_bench *omb_bench;
    struct omb_run *omb_run;
} omb_env_t;

/* ---- entry signature for per-benchmark initializers ---- */
typedef int (*omb_entry_fn)(omb_env_t *);

typedef void (*omb_env_fn)(omb_env_t *);
typedef int (*omb_env_ret_fn)(omb_env_t *);

/* Prototypes */
void omb_env_init(omb_env_t *ctx, int *argc, char **argv[],
                  omb_resource_owner_t owner);

int omb_get_node_information(omb_env_t *ctx, bool validate_for_fan);

void print_bench_preamble(int rank, bool is_nonblocking);
void omb_print_header(int rank, bool is_nonblocking);

int omb_handle_option_processing(omb_env_t *ctx);

void omb_print_pair_info(omb_env_t *ctx);

#endif /* _OMB_GENERAL_UTIL_H_ */
