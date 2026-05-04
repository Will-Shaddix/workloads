#ifndef _OMB_COLOR_H_
#define _OMB_COLOR_H_

#include <stdbool.h>
#include <stdio.h>

/* Configure-time default */
#ifndef OMB_COLOR_POLICY
#define OMB_COLOR_POLICY "auto"
#endif

/* Initialize color policy. */
void omb_color_init(void);

bool omb_color_enabled_stdout(void);
bool omb_color_enabled_stderr(void);

void omb_color_debug_dump(void);

/* ======= Stream-aware escape sequences ======= */
#define OMB_ERR_RESET   (omb_color_enabled_stderr() ? "\x1b[0m" : "")
#define OMB_ERR_BOLD    (omb_color_enabled_stderr() ? "\x1b[1m" : "")
#define OMB_ERR_DIM     (omb_color_enabled_stderr() ? "\x1b[2m" : "")
#define OMB_ERR_RED     (omb_color_enabled_stderr() ? "\x1b[31m" : "")
#define OMB_ERR_GREEN   (omb_color_enabled_stderr() ? "\x1b[32m" : "")
#define OMB_ERR_YELLOW  (omb_color_enabled_stderr() ? "\x1b[33m" : "")
#define OMB_ERR_BLUE    (omb_color_enabled_stderr() ? "\x1b[34m" : "")
#define OMB_ERR_MAGENTA (omb_color_enabled_stderr() ? "\x1b[35m" : "")
#define OMB_ERR_CYAN    (omb_color_enabled_stderr() ? "\x1b[36m" : "")

#define OMB_OUT_RESET   (omb_color_enabled_stdout() ? "\x1b[0m" : "")
#define OMB_OUT_BOLD    (omb_color_enabled_stdout() ? "\x1b[1m" : "")
#define OMB_OUT_DIM     (omb_color_enabled_stdout() ? "\x1b[2m" : "")
#define OMB_OUT_RED     (omb_color_enabled_stdout() ? "\x1b[31m" : "")
#define OMB_OUT_GREEN   (omb_color_enabled_stdout() ? "\x1b[32m" : "")
#define OMB_OUT_YELLOW  (omb_color_enabled_stdout() ? "\x1b[33m" : "")
#define OMB_OUT_BLUE    (omb_color_enabled_stdout() ? "\x1b[34m" : "")
#define OMB_OUT_MAGENTA (omb_color_enabled_stdout() ? "\x1b[35m" : "")
#define OMB_OUT_CYAN    (omb_color_enabled_stdout() ? "\x1b[36m" : "")

#endif /* _OMB_COLOR_H_ */
