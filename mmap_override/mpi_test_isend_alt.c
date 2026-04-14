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

    int send_peer = (rank + 1) % size;
    int recv_peer = (rank + size - 1) % size;

    int send_tag = 100 + send_peer;
    int recv_tag = 100 + rank;
    int recv_val = -1;

    annotate_init_();

    ROI_BEGIN_LOOP();
    for (int i = 0; i < num_loops; i++) {
        ROI_BEGIN();
        int send_val = rank * 100 + i;
        MPI_Request reqs[2];

        if ((i + rank) % 2 == 0) {
            MPI_Isend(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD, &reqs[1]);
            MPI_Wait(&reqs[1], MPI_STATUS_IGNORE);
        } else {
            MPI_Irecv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &reqs[0]);
            MPI_Wait(&reqs[0], MPI_STATUS_IGNORE);
        }

        ROI_END();
    }
    ROI_END_LOOP();

    MPI_Barrier(MPI_COMM_WORLD);

    printf("Rank %d (Isend_Alt): final received %d\n", rank, recv_val);
    fflush(stdout);

    if (rank == 0) {
        M5_EXIT();
    }

    MPI_Finalize();

    return 0;
}
