/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level directory.
 */

#include "omb_general_util.h"

#define MAX_TEST_NAME_LENGTH 50

typedef struct entry {
    const char *suite;
    const char *name;
    omb_env_ret_fn entry_fn;
} entry_t;

typedef struct bad_opt_rec {
    const char *name;
    char opt;
    const char *message;
} bad_opt_rec_t;

typedef struct test_rc {
    const char *name;
    int code;
} test_rc_t;

typedef struct test_message {
    const char *name;
    const char *message;
} test_message_t;

typedef struct omb_launcher_ctx {
    int argc;
    char **argv;
    omb_env_t omb_env;

    int entry_count;
    entry_t *entries;

    bad_opt_rec_t *bad_opt_tests;
    int bad_opt_count;

    test_rc_t *failed_tests;
    int failed_count;

    test_message_t *unlaunched_tests;
    int unlaunched_count;
} omb_launcher_ctx_t;

void omb_run_requested_tests(omb_launcher_ctx_t *C, int argc, char **argv);
void omb_print_result_summary(omb_launcher_ctx_t *C);
