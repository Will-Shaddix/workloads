/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#ifdef OMB_SYNC_fence
#define OMB_BW_SYNC_SYM                    run_op_bw_with_fence
#define OMB_LAT_SYNC_SYM                   run_op_lat_with_fence
#define OMB_BIBW_SYNC_SYM                  run_op_bibw_with_fence
#define OMB_ACC_LAT_SYNC_SYM               run_op_acc_with_fence
#define OMB_CAS_LAT_SYNC_SYM               run_op_cas_with_fence
#define OMB_FOP_LAT_SYNC_SYM               run_op_fop_with_fence
#define OMB_GET_ACC_LATENCY_SYNC_SYM       run_op_get_acc_with_fence
#define MPI_OP_PRE_SYNC(group, window)     MPI_CHECK(MPI_Win_fence(0, window));
#define MPI_NON_OP_PRE_SYNC(group, window) MPI_CHECK(MPI_Win_fence(0, window));
#define MPI_OP_POST_SYNC(window)           MPI_CHECK(MPI_Win_fence(0, window));
#define MPI_NON_OP_POST_SYNC(window)       MPI_CHECK(MPI_Win_fence(0, window));
#define MPI_PING(group, window)            MPI_CHECK(MPI_Win_fence(0, window));
#define MPI_LAT_PONG(group, window)                                            \
    MPI_CHECK(BENCH->op_ptr(BENCH->op_buf + (j * RUN->msg_bytes),              \
                            RUN->msg_bytes, MPI_CHAR, RUN->destrank,           \
                            BENCH->disp + (j * RUN->msg_bytes),                \
                            RUN->msg_bytes, MPI_CHAR, RUN->win_handle));       \
    MPI_OP_POST_SYNC(window);
#define MPI_ACC_PONG(group, window)                                            \
    MPI_CHECK(MPI_Accumulate(BENCH->op_buf, RUN->elem_count, RUN->dtype,       \
                             RUN->destrank, BENCH->disp, RUN->elem_count,      \
                             RUN->dtype, MPI_SUM, RUN->win_handle));           \
    MPI_OP_POST_SYNC(window);
#define MPI_CAS_PONG(group, window)                                            \
    MPI_CHECK(MPI_Compare_and_swap(BENCH->alt_op_buf, BENCH->cbuf,             \
                                   BENCH->tbuf, RUN->dtype, RUN->destrank,     \
                                   BENCH->disp, RUN->win_handle));             \
    MPI_OP_POST_SYNC(window);
#define MPI_FOP_PONG(group, window)                                            \
    MPI_CHECK(MPI_Fetch_and_op(BENCH->alt_op_buf, BENCH->tbuf, RUN->dtype,     \
                               RUN->destrank, BENCH->disp, MPI_SUM,            \
                               RUN->win_handle));                              \
    MPI_OP_POST_SYNC(window);
#define MPI_GET_ACC_PONG(group, window)                                        \
    MPI_CHECK(MPI_Get_accumulate(BENCH->op_buf, RUN->msg_bytes, MPI_CHAR,      \
                                 BENCH->cbuf, RUN->msg_bytes, MPI_CHAR,        \
                                 RUN->destrank, BENCH->disp, RUN->msg_bytes,   \
                                 MPI_CHAR, MPI_SUM, RUN->win_handle));         \
    MPI_OP_POST_SYNC(window);
#define MPI_BIBW_PRE_SYNC(group, window)  MPI_OP_PRE_SYNC(group, window);
#define MPI_BIBW_POST_SYNC(group, window) MPI_OP_POST_SYNC(window);
#endif

#ifdef OMB_SYNC_lock
#define OMB_BW_SYNC_SYM              run_op_bw_with_lock
#define OMB_LAT_SYNC_SYM             run_op_lat_with_lock
#define OMB_BIBW_SYNC_SYM            run_op_bibw_with_lock
#define OMB_ACC_LAT_SYNC_SYM         run_op_acc_with_lock
#define OMB_CAS_LAT_SYNC_SYM         run_op_cas_with_lock
#define OMB_FOP_LAT_SYNC_SYM         run_op_fop_with_lock
#define OMB_GET_ACC_LATENCY_SYNC_SYM run_op_get_acc_with_lock
#define MPI_OP_PRE_SYNC(group, window)                                         \
    MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, window));
#define MPI_OP_POST_SYNC(window) MPI_CHECK(MPI_Win_unlock(1, window));
#endif

#ifdef OMB_SYNC_pscw
#define OMB_BW_SYNC_SYM              run_op_bw_with_pscw
#define OMB_LAT_SYNC_SYM             run_op_lat_with_pscw
#define OMB_BIBW_SYNC_SYM            run_op_bibw_with_pscw
#define OMB_ACC_LAT_SYNC_SYM         run_op_acc_with_pscw
#define OMB_CAS_LAT_SYNC_SYM         run_op_cas_with_pscw
#define OMB_FOP_LAT_SYNC_SYM         run_op_fop_with_pscw
#define OMB_GET_ACC_LATENCY_SYNC_SYM run_op_get_acc_with_pscw
#define MPI_OP_PRE_SYNC(group, window)                                         \
    MPI_CHECK(MPI_Win_start(group, 0, window));
#define MPI_NON_OP_PRE_SYNC(group, window)                                     \
    MPI_CHECK(MPI_Win_post(group, 0, window));
#define MPI_OP_POST_SYNC(window)     MPI_CHECK(MPI_Win_complete(window));
#define MPI_NON_OP_POST_SYNC(window) MPI_CHECK(MPI_Win_wait(window));
#define MPI_PING(group, window)                                                \
    MPI_CHECK(MPI_Win_post(group, 0, window));                                 \
    MPI_CHECK(MPI_Win_wait(window));
#define MPI_LAT_PONG(group, window)                                            \
    MPI_OP_PRE_SYNC(group, window);                                            \
    MPI_CHECK(BENCH->op_ptr(BENCH->op_buf + (j * RUN->msg_bytes),              \
                            RUN->msg_bytes, MPI_CHAR, RUN->destrank,           \
                            BENCH->disp + (j * RUN->msg_bytes),                \
                            RUN->msg_bytes, MPI_CHAR, RUN->win_handle));       \
    MPI_OP_POST_SYNC(window);
#define MPI_ACC_PONG(group, window)                                            \
    MPI_OP_PRE_SYNC(group, window);                                            \
    MPI_CHECK(MPI_Accumulate(BENCH->op_buf, RUN->elem_count, RUN->dtype,       \
                             RUN->destrank, BENCH->disp, RUN->elem_count,      \
                             RUN->dtype, MPI_SUM, RUN->win_handle));           \
    MPI_OP_POST_SYNC(window);
#define MPI_CAS_PONG(group, window)                                            \
    MPI_OP_PRE_SYNC(group, window);                                            \
    MPI_CHECK(MPI_Compare_and_swap(BENCH->alt_op_buf, BENCH->cbuf,             \
                                   BENCH->tbuf, RUN->dtype, RUN->destrank,     \
                                   BENCH->disp, RUN->win_handle));             \
    MPI_OP_POST_SYNC(window);
#define MPI_FOP_PONG(group, window)                                            \
    MPI_OP_PRE_SYNC(group, window);                                            \
    MPI_CHECK(MPI_Fetch_and_op(BENCH->alt_op_buf, BENCH->tbuf, RUN->dtype,     \
                               RUN->destrank, BENCH->disp, MPI_SUM,            \
                               RUN->win_handle));                              \
    MPI_OP_POST_SYNC(window);
#define MPI_GET_ACC_PONG(group, window)                                        \
    MPI_OP_PRE_SYNC(group, window);                                            \
    MPI_CHECK(MPI_Get_accumulate(BENCH->op_buf, RUN->msg_bytes, MPI_CHAR,      \
                                 BENCH->cbuf, RUN->msg_bytes, MPI_CHAR,        \
                                 RUN->destrank, BENCH->disp, RUN->msg_bytes,   \
                                 MPI_CHAR, MPI_SUM, RUN->win_handle));         \
    MPI_OP_POST_SYNC(window);
#define MPI_BIBW_PRE_SYNC(group, window)                                       \
    MPI_NON_OP_PRE_SYNC(group, window);                                        \
    MPI_OP_PRE_SYNC(group, window);
#define MPI_BIBW_POST_SYNC(group, window)                                      \
    MPI_OP_POST_SYNC(window);                                                  \
    MPI_NON_OP_POST_SYNC(window);
#endif

#ifdef OMB_SYNC_lock_all
#define OMB_BW_SYNC_SYM                run_op_bw_with_lock_all
#define OMB_LAT_SYNC_SYM               run_op_lat_with_lock_all
#define OMB_BIBW_SYNC_SYM              run_op_bibw_with_lock_all
#define OMB_ACC_LAT_SYNC_SYM           run_op_acc_with_lock_all
#define OMB_CAS_LAT_SYNC_SYM           run_op_cas_with_lock_all
#define OMB_FOP_LAT_SYNC_SYM           run_op_fop_with_lock_all
#define OMB_GET_ACC_LATENCY_SYNC_SYM   run_op_get_acc_with_lock_all
#define MPI_OP_PRE_SYNC(group, window) MPI_CHECK(MPI_Win_lock_all(0, window));
#define MPI_OP_POST_SYNC(window)       MPI_CHECK(MPI_Win_unlock_all(window));
#endif

#ifdef OMB_SYNC_flush
#define OMB_BW_SYNC_SYM              run_op_bw_with_flush
#define OMB_LAT_SYNC_SYM             run_op_lat_with_flush
#define OMB_BIBW_SYNC_SYM            run_op_bibw_with_flush
#define OMB_ACC_LAT_SYNC_SYM         run_op_acc_with_flush
#define OMB_CAS_LAT_SYNC_SYM         run_op_cas_with_flush
#define OMB_FOP_LAT_SYNC_SYM         run_op_fop_with_flush
#define OMB_GET_ACC_LATENCY_SYNC_SYM run_op_get_acc_with_flush
#define MPI_LOOP_LOCK(window)                                                  \
    MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, window));
#define MPI_OP_POST_SYNC(window) MPI_CHECK(MPI_Win_flush(1, window));
#define MPI_LOOP_UNLOCK(window)  MPI_CHECK(MPI_Win_unlock(1, window));
#endif

#ifdef OMB_SYNC_flush_local
#define OMB_BW_SYNC_SYM              run_op_bw_with_flush_local
#define OMB_LAT_SYNC_SYM             run_op_lat_with_flush_local
#define OMB_BIBW_SYNC_SYM            run_op_bibw_with_flush_local
#define OMB_ACC_LAT_SYNC_SYM         run_op_acc_with_flush_local
#define OMB_CAS_LAT_SYNC_SYM         run_op_cas_with_flush_local
#define OMB_FOP_LAT_SYNC_SYM         run_op_fop_with_flush_local
#define OMB_GET_ACC_LATENCY_SYNC_SYM run_op_get_acc_with_flush_local
#define MPI_LOOP_LOCK(window)                                                  \
    MPI_CHECK(MPI_Win_lock(MPI_LOCK_SHARED, 1, 0, window));
#define MPI_OP_POST_SYNC(window) MPI_CHECK(MPI_Win_flush_local(1, window));
#define MPI_LOOP_UNLOCK(window)  MPI_CHECK(MPI_Win_unlock(1, window));
#endif
