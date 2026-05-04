/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level directory.
 */

#include "omb_launcher_util.h"

static void record_bad_opt(omb_launcher_ctx_t *ctx)
{
    if (ctx->bad_opt_count < ctx->entry_count) {
        ctx->bad_opt_tests[ctx->bad_opt_count++] =
            (bad_opt_rec_t){.name = ctx->omb_env.title,
                            .opt = bad_usage.opt,
                            .message = bad_usage.message};
    }
}

static void print_bad_opt_summary(omb_launcher_ctx_t *ctx)
{
    int i;
    if (!ctx->bad_opt_count) {
        return;
    }
    fprintf(stderr,
            "\n%s%d test(s) could not run due to unsupported options:\n",
            OMB_ERR_RED, ctx->bad_opt_count);
    fprintf(stderr, "%s                            test  option  message%s\n",
            OMB_ERR_CYAN, OMB_ERR_RESET);
    fprintf(stderr, "  ------------------------------  ------  "
                    "------------------------------\n");
    for (i = 0; i < ctx->bad_opt_count; i++) {
        bad_opt_rec_t bad_opt = ctx->bad_opt_tests[i];
        fprintf(stderr, "  %30s  %s%6c  %s%s%s\n", bad_opt.name, OMB_ERR_YELLOW,
                bad_opt.opt, OMB_ERR_RED, bad_opt.message, OMB_ERR_RESET);
    }
}

static void record_failure(int rc, omb_launcher_ctx_t *ctx)
{
    if (ctx->failed_count < ctx->entry_count) {
        ctx->failed_tests[ctx->failed_count++] =
            (test_rc_t){.name = ctx->omb_env.title, .code = rc};
    }
}

static void print_failure_summary(omb_launcher_ctx_t *ctx)
{
    int i;
    if (!ctx->failed_count) {
        return;
    }
    fprintf(stderr, "\n%s%d test(s) exited with failure:\n", OMB_ERR_RED,
            ctx->failed_count);
    fprintf(stderr, "%s                            test  status%s\n",
            OMB_ERR_CYAN, OMB_ERR_RESET);
    fprintf(
        stderr,
        "  ------------------------------  ------------------------------\n");
    for (i = 0; i < ctx->failed_count; i++) {
        fprintf(stderr, "  %30s  %s%s%s\n", ctx->failed_tests[i].name,
                OMB_ERR_RED, omb_strerror(ctx->failed_tests[i].code),
                OMB_ERR_RESET);
    }
}

static void record_unlaunched(omb_launcher_ctx_t *ctx)
{
    if (ctx->unlaunched_count < ctx->entry_count) {
        ctx->unlaunched_tests[ctx->unlaunched_count++] = (test_message_t){
            .name = ctx->omb_env.title, .message = not_launched_message};
    }
}

static void print_unlaunched_summary(omb_launcher_ctx_t *ctx)
{
    int i;
    if (!ctx->unlaunched_count) {
        return;
    }
    fprintf(stderr, "\n%s%d test(s) were not launched:\n", OMB_ERR_BLUE,
            ctx->unlaunched_count);
    fprintf(stderr, "%s                            test  status%s\n",
            OMB_ERR_CYAN, OMB_ERR_RESET);
    fprintf(
        stderr,
        "  ------------------------------  ------------------------------\n");
    for (i = 0; i < ctx->unlaunched_count; i++) {
        fprintf(stderr, "  %30s  %s%s%s\n", ctx->unlaunched_tests[i].name,
                OMB_ERR_BLUE, ctx->unlaunched_tests[i].message, OMB_ERR_RESET);
    }
}

static void handle_test_return_code(int rc, omb_launcher_ctx_t *ctx)
{
    if (ctx->omb_env.rank) {
        return;
    }

    switch (rc) {
        case EXIT_SUCCESS:
            break;

        case OMB_ERR_BAD_USAGE:
            record_bad_opt(ctx);
            break;

        case OMB_ERR_NOT_LAUNCHED:
            record_unlaunched(ctx);
            break;

        default:
            record_failure(rc, ctx);
    }
}

static int find_entry_index(const entry_t *entries, int n, const char *name)
{
    int i;
    char osu_name[MAX_TEST_NAME_LENGTH] = "osu_";
    strcat(osu_name, name);

    for (i = 0; i < n; i++) {
        if (strcmp(entries[i].name, name) == 0 ||
            strcmp(entries[i].name, osu_name) == 0) {
            return i;
        }
    }
    return -1;
}

/* Runs and handles return of test if it exists,
 * otherwise adds it to unlaunched_tests.
 */
static void run_test(omb_launcher_ctx_t *ctx, const char *test)
{
    int rc, idx = find_entry_index(ctx->entries, ctx->entry_count, test);

    if (idx >= 0) {
        rc = ctx->entries[idx].entry_fn(&ctx->omb_env);
        handle_test_return_code(rc, ctx);
        MPI_CHECK(MPI_Barrier(ctx->omb_env.comm));
    } else if (!ctx->omb_env.rank &&
               (ctx->unlaunched_count < ctx->entry_count)) {
        ctx->unlaunched_tests[ctx->unlaunched_count++] = (test_message_t){
            .name = test, .message = "This test was not found and was skipped"};
    }
}

/* Runs and handles return of all tests with matching suite.
 * Adds warning to unlaunched_tests if none match.
 */
static void run_suite(omb_launcher_ctx_t *ctx, const char *suite)
{
    int rc, i, found_one = 0;
    for (i = 0; i < ctx->entry_count; i++) {
        if (strcmp(ctx->entries[i].suite, suite) == 0) {
            found_one = 1;
            rc = ctx->entries[i].entry_fn(&ctx->omb_env);
            handle_test_return_code(rc, ctx);
            MPI_CHECK(MPI_Barrier(ctx->omb_env.comm));
        }
    }
    if (!found_one && !ctx->omb_env.rank &&
        (ctx->unlaunched_count < ctx->entry_count)) {
        ctx->unlaunched_tests[ctx->unlaunched_count++] = (test_message_t){
            .name = suite, .message = "No tests found under this suite"};
    }
}

/* Run all tests unless "--" is present. */
void omb_run_requested_tests(omb_launcher_ctx_t *ctx, int argc, char **argv)
{
    int i, rc;
    char *want;
    const int sep = omb_find_sep_index(argc, argv);

    /* No filter */
    if (sep == SEPARATOR_NONE) {
        for (i = 0; i < ctx->entry_count; ++i) {
            int rc = ctx->entries[i].entry_fn(&ctx->omb_env);
            handle_test_return_code(rc, ctx);
            MPI_CHECK(MPI_Barrier(ctx->omb_env.comm));
        }
        return;
    }

    /* Run user listed tests */
    for (i = sep + 1; i < argc; i++) {
        want = argv[i];
        if (strncmp(want, "s:", 2) == 0) {
            run_suite(ctx, want + 2);
        } else {
            run_test(ctx, want);
        }
    }
}

void omb_print_result_summary(omb_launcher_ctx_t *ctx)
{
    print_unlaunched_summary(ctx);
    print_bad_opt_summary(ctx);
    print_failure_summary(ctx);
    fprintf(stderr, "\n");
    fflush(stderr);
}
