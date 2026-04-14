#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "annotate.h"

#ifdef ROI_PER_ITERATION
#define ROI_BEGIN() roi_begin_()
#define ROI_END() roi_end_()
#define ROI_BEGIN_LOOP()
#define ROI_END_LOOP()
#else
#define ROI_BEGIN()
#define ROI_END()
#define ROI_BEGIN_LOOP() roi_begin_()
#define ROI_END_LOOP() roi_end_()
#endif

#ifdef GEM5
#include "gem5/m5ops.h"
#define M5_RESET_STATS() m5_reset_stats(0, 0)
#define M5_CHECKPOINT() m5_checkpoint(0, 0)
#define M5_EXIT() m5_exit(0)
#else
#define M5_RESET_STATS()
#define M5_CHECKPOINT()
#define M5_EXIT()
#endif

int main(int argc, char** argv) {
    M5_RESET_STATS();

    MPI_Init(&argc, &argv);
    printf("MPI_Init success\n");

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("MPI Rank and Size = %d, %d success\n", rank, size);

    int num_loops = 1;
    if (argc > 1) {
        num_loops = atoi(argv[1]);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        M5_CHECKPOINT();
    }

    int recv_val = -1;
    
    // We need an array of requests and values for the Isends (size-1 ranks, num_loops elements)
    MPI_Request* send_reqs = NULL;
    int* send_vals = NULL;
    
    if (rank != 0) {
        send_reqs = (MPI_Request*)malloc(num_loops * sizeof(MPI_Request));
        send_vals = (int*)malloc(num_loops * sizeof(int));
    }

    annotate_init_();

    ROI_BEGIN_LOOP();
    for (int i = 0; i < num_loops; i++) {
        ROI_BEGIN();
        
        if (rank != 0) {
            // Rank non-zero fires an Isend to Rank 0. 
            // We use tag "100 + rank" to identify the sender
            send_vals[i] = rank * 100 + i;
            MPI_Isend(&send_vals[i], 1, MPI_INT, 0, 100 + rank, MPI_COMM_WORLD, &send_reqs[i]);
        }
        // Rank 0 does nothing inside the loop! Memory buffers are getting filled.

        ROI_END();
    }
    ROI_END_LOOP();
    
    // After the loop has completed, Rank 0 clears the buffer by receiving all messages
    if (rank == 0) {
        for (int i = 0; i < num_loops; i++) {
            for (int p = 1; p < size; p++) {
                MPI_Status st;
                MPI_Recv(&recv_val, 1, MPI_INT, p, 100 + p, MPI_COMM_WORLD, &st);
            }
        }
    } else {
        // Non-zero ranks wait for their Isends to complete and free memory
        MPI_Waitall(num_loops, send_reqs, MPI_STATUSES_IGNORE);
        free(send_reqs);
        free(send_vals);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    printf("Rank %d (Isend_Fill): final received %d\n", rank, recv_val);
    fflush(stdout);

    if (rank == 0) {
        M5_EXIT();
    }

    MPI_Finalize();

    return 0;
}
