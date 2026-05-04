#include "omb_color.h"

#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Internal state */
static bool g_out_enabled = false;
static bool g_err_enabled = false;

/* ---------- helpers ---------- */

static inline bool env_on(const char *name)
{
    const char *v = getenv(name);
    return v && *v && strcmp(v, "0") != 0;
}

static inline bool env_eq(const char *name, const char *val)
{
    const char *v = getenv(name);
    return v && strcmp(v, val) == 0;
}

/* https://no-color.org/ */
static bool env_disables_color(void) { return env_on("NO_COLOR"); }

/* hard override */
static bool env_forces_color(void)
{
    return env_on("CLICOLOR_FORCE") || env_on("FORCE_COLOR");
}

/* soft opt-in */
static bool env_prefers_color(void) { return env_eq("CLICOLOR", "1"); }

static bool terminal_likely_supports_color(void)
{
    const char *t = getenv("TERM");
    if (!t || !*t || strcmp(t, "dumb") == 0) {
        return false;
    }
    /* Heuristic: most common TERM values support ANSI SGR. Good-enough. */
    return true;
}

enum fd_kind {
    FD_UNKNOWN = 0,
    FD_CHAR = 1,
    FD_PIPE = 2,
    FD_SOCK = 3,
    FD_REG = 4
};

static enum fd_kind fd_kind_of(int fd)
{
    struct stat st;
    if (fd < 0 || fstat(fd, &st) != 0) {
        return FD_UNKNOWN;
    }
    if (S_ISCHR(st.st_mode)) {
        return FD_CHAR;
    }
    if (S_ISFIFO(st.st_mode)) {
        return FD_PIPE;
    }
    if (S_ISSOCK(st.st_mode)) {
        return FD_SOCK;
    }
    if (S_ISREG(st.st_mode)) {
        return FD_REG;
    }
    return FD_UNKNOWN;
}

static bool interactive_session_hint(void)
{
    /* Signals that the job was launched from a real terminal. */
    if (getenv("SSH_TTY")) {
        return true;
    }
    if (getenv("SSH_CONNECTION")) {
        return true;
    }

    /* Enviroment hint */
    if (env_prefers_color()) {
        return true;
    }

    return false;
}

static bool policy_is(const char *s)
{
    return strcmp(OMB_COLOR_POLICY, s) == 0;
}

static bool decide_enabled_for_stream(FILE *s)
{
    /* Environment precedence */
    if (env_disables_color()) {
        return false;
    }
    if (env_forces_color()) {
        return true;
    }

    /* Configure default */
    if (policy_is("never")) {
        return false;
    }
    if (policy_is("always")) {
        return true;
    }

    /* "auto" (default) */
    int fd = fileno(s);
    if (fd < 0) {
        return false;
    }

    bool term_ok = terminal_likely_supports_color();

    if (isatty(fd)) {
        return term_ok;
    }

    /* Not a TTY: reason about kind + context */
    enum fd_kind kind = fd_kind_of(fd);
    if (kind != FD_PIPE && kind != FD_SOCK) {
        /* Do not automatically color regular files or unknown kinds. */
        return false;
    }

    /* stdout: conservative by default (protect tees/files). */
    if (s == stdout) {
        return term_ok && env_prefers_color();
    }

    /* stderr: be helpful under launchers that relay to a terminal. */
    if (s == stderr) {
        return term_ok && interactive_session_hint();
    }

    return false;
}

/* ---------- public API ---------- */

void omb_color_init(void)
{
    g_out_enabled = decide_enabled_for_stream(stdout);
    g_err_enabled = decide_enabled_for_stream(stderr);

    if (env_on("OMB_COLOR_DEBUG")) {
        omb_color_debug_dump();
    }
}

bool omb_color_enabled_stdout(void) { return g_out_enabled; }
bool omb_color_enabled_stderr(void) { return g_err_enabled; }

void omb_color_debug_dump(void)
{
    fprintf(stderr,
            "[OMB color] policy=%s TERM=%s SSH_TTY=%s CLICOLOR=%s "
            "CLICOLOR_FORCE=%s | "
            "stdout: enabled=%d isatty=%d kind=%d | "
            "stderr: enabled=%d isatty=%d kind=%d\n",
            OMB_COLOR_POLICY, getenv("TERM") ?: "", getenv("SSH_TTY") ?: "",
            getenv("CLICOLOR") ?: "", getenv("CLICOLOR_FORCE") ?: "",
            g_out_enabled, isatty(fileno(stdout)),
            (int)fd_kind_of(fileno(stdout)), g_err_enabled,
            isatty(fileno(stderr)), (int)fd_kind_of(fileno(stderr)));
}
