#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "annotate.h"
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

    int recv_bcast[size];

    annotate_init_();

    ROI_BEGIN_LOOP();
    for (int i = 0; i < num_loops; i++) {
        ROI_BEGIN();
        int send_val = rank * 100 + i;

        int num_peers = size - 1;
        MPI_Request all_reqs[2 * num_peers];
        int req_idx = 0;
        
        for (int p = 0; p < size; p++) {
            if (p != rank) {
                // Receive tag should be 100 + our rank. Send tag should be 100 + dest rank (p)
                MPI_Irecv(&recv_bcast[p], 1, MPI_INT, p, 100 + rank, MPI_COMM_WORLD, &all_reqs[req_idx++]);
                MPI_Isend(&send_val, 1, MPI_INT, p, 100 + p, MPI_COMM_WORLD, &all_reqs[req_idx++]);
            }
        }
        
        if (num_peers > 0) {
            MPI_Waitall(2 * num_peers, all_reqs, MPI_STATUSES_IGNORE);
        }
        recv_bcast[rank] = send_val;

        ROI_END();
    }
    ROI_END_LOOP();

    MPI_Barrier(MPI_COMM_WORLD);

    printf("Rank %d (IsendAll): final received at index 0 %d\n", rank, recv_bcast[0]);
    fflush(stdout);

    if (rank == 0) {
        M5_EXIT();
    }

    MPI_Finalize();

    return 0;
}
