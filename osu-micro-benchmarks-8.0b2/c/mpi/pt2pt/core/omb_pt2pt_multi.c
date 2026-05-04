/*
 * Copyright (c) 2002-2025 the Network-Based Computing Laboratory
 * (NBCL), The Ohio State University.
 *
 * Contact: Dr. D. K. Panda (panda@cse.ohio-state.edu)
 *
 * For detailed copyright and licensing information, please refer to the
 * copyright file COPYRIGHT in the top level OMB directory.
 */

#include "omb_pt2pt_multi.h"
#include "omb_general_util.h"
#include "omb_pt2pt_core.h"

#define THREADED (BENCH->multithread)

/* Multi-process */

void omb_handle_multiprocess(omb_env_t *ctx)
{
    int i;
    if (BENCH->is_sender) {
        if (BENCH->subtype == LAT_MP) {
            fprintf(stdout, "# Number of forked processes in sender: %d\n",
                    BENCH->multiprocess->num_processes_sender);
            fprintf(stdout, "# Number of forked processes in receiver: %d\n",
                    options.num_processes);
            fflush(stdout);
        }

        for (i = 0; i < BENCH->multiprocess->num_processes_sender; i++) {
            BENCH->multiprocess->sr_processes[i] = fork();
            if (BENCH->multiprocess->sr_processes[i] == 0) {
                BENCH->multiprocess->is_child = 1;
                break;
            }
        }
        if (BENCH->multiprocess->is_child) {
            sleep(CHILD_SLEEP_SECONDS);
            exit(EXIT_SUCCESS);
        }
    } else {
        for (i = 0; i < options.num_processes; i++) {
            BENCH->multiprocess->sr_processes[i] = fork();
            if (BENCH->multiprocess->sr_processes[i] == 0) {
                BENCH->multiprocess->is_child = 1;
                break;
            }
        }
        if (BENCH->multiprocess->is_child) {
            sleep(CHILD_SLEEP_SECONDS);
            exit(EXIT_SUCCESS);
        }
    }
}

/* Multi-threaded */
int omb_handle_multithread(omb_env_t *ctx, void *(*threaded_entry)(void *))
{
    int ret, i;

    THREADED->num_threads_send =
        (options.sender_thread != -1) ? options.sender_thread : 1;
    THREADED->num_threads_recv = options.num_threads;

    THREADED->my_num_threads = (BENCH->is_sender) ? THREADED->num_threads_send :
                                                    THREADED->num_threads_recv;

    /* pthread initialization */
    pthread_barrier_init(&THREADED->intra_barrier, NULL,
                         THREADED->my_num_threads);

    if (ctx->rank == 0) {
        printf("# Number of Sender threads: %d \n# Number of Receiver threads: "
               "%d\n\n",
               THREADED->num_threads_send, THREADED->num_threads_recv);
    }

    for (i = 0; i < THREADED->my_num_threads; i++) {
        THREADED->contexts[i].id = i;
        THREADED->contexts[i].omb_env = ctx;

        pthread_create(&THREADED->pthreads[i], NULL, threaded_entry,
                       &THREADED->contexts[i]);
    }
    for (i = 0; i < THREADED->my_num_threads; i++) {
        pthread_join(THREADED->pthreads[i], NULL);
    }

    return EXIT_SUCCESS;
}

void omb_sync_inter_threads(omb_env_t *ctx)
{
    if (!RUN->is_pthread) {
        MPI_CHECK(MPI_Barrier(ctx->comm));
        return;
    }

    int rv = pthread_barrier_wait(&THREADED->intra_barrier);

    /* One thread per process returns PTHREAD_BARRIER_SERIAL_THREAD */
    if (rv == PTHREAD_BARRIER_SERIAL_THREAD) {
        MPI_CHECK(MPI_Barrier(ctx->comm));
    }

    /* Release threads after inter_barrier */
    pthread_barrier_wait(&THREADED->intra_barrier);
}
