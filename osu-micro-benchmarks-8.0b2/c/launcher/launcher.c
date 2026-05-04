/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#ifdef PACKAGE_VERSION
#define HEADER "# " BENCHMARK " v" PACKAGE_VERSION "\n"
#else
#define HEADER "# " BENCHMARK "\n"
#endif

#ifndef OMB_ENTRIES_HEADER
#error "OMB_ENTRIES_HEADER must be defined to the domain's entry-list header"
#endif

#include OMB_ENTRIES_HEADER
#include "omb_launcher_util.h"

#define DECLARE(suite, name, symbol) extern int symbol(omb_env_t *);
OMB_FOR_EACH_ENTRY_ALL(DECLARE)
#undef DECLARE

#define ROW(suite, name, symbol) {suite, name, symbol},
static entry_t entries[] = {OMB_FOR_EACH_ENTRY_ALL(ROW)};
#undef ROW

static bad_opt_rec_t bad_opt_tests[OMB_ENTRY_COUNT];
static test_rc_t failed_tests[OMB_ENTRY_COUNT];
static test_message_t unlaunched_tests[OMB_ENTRY_COUNT];

int main(int argc, char *argv[])
{
    omb_launcher_ctx_t ctx = {0};
    int omb_ret;

    omb_env_init(&ctx.omb_env, &argc, &argv, OMB_SHARES_RESOURCES);

    ctx.entries = entries;
    ctx.entry_count = OMB_ENTRY_COUNT;
    ctx.bad_opt_tests = bad_opt_tests;
    ctx.failed_tests = failed_tests;
    ctx.unlaunched_tests = unlaunched_tests;

    options.subtype = LAUNCHER;
    benchmark_name = EXECUTABLE_NAME;
    set_header(HEADER);

    omb_ret = omb_handle_option_processing(&ctx.omb_env);
    if (EXIT_SUCCESS != omb_ret) {
        omb_mpi_finalize(ctx.omb_env.omb_init);
        return normalize_exit_code(omb_ret);
    }

    omb_run_requested_tests(&ctx, argc, argv);

    if (!ctx.omb_env.rank) {
        omb_print_result_summary(&ctx);
    }

    omb_mpi_finalize(ctx.omb_env.omb_init);
    return EXIT_SUCCESS;
}
