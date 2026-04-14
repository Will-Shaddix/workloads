// pingpong.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// 

#include "gem5/m5ops.h"


int main(int argc, char** argv) {

    // printf("Resetting stats\n");
    m5_reset_stats(0, 0);

    MPI_Init(&argc, &argv);
    printf("MPI_Init success\n");

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("MPI Rank and Size = %d, %d success\n", rank, size);

    if (size != 2) {
        if (rank == 0) {
            fprintf(stderr, "Run with exactly 2 processes (mpirun -np 2 ...)\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        m5_checkpoint(0, 0);
    }

    int peer = 1 - rank;

    // ---------- Phase 1: synchronous send/recv ----------
    int send_sync = rank * 100 + 1;   // distinct payload per rank
    int recv_sync = -1;
    MPI_Status st;

    // Use opposite ordering to avoid deadlock with Ssend.
    if (rank == 0) {
        MPI_Ssend(&send_sync, 1, MPI_INT, peer, 100, MPI_COMM_WORLD);
        MPI_Recv (&recv_sync, 1, MPI_INT, peer, 101, MPI_COMM_WORLD, &st);
    } else {
        MPI_Recv (&recv_sync, 1, MPI_INT, peer, 100, MPI_COMM_WORLD, &st);
        MPI_Ssend(&send_sync, 1, MPI_INT, peer, 101, MPI_COMM_WORLD);
    }

    printf("Rank %d (sync): received %d\n", rank, recv_sync);
    fflush(stdout);
    MPI_Barrier(MPI_COMM_WORLD);

    // ---------- Phase 2: asynchronous send/recv ----------
    int send_async = rank * 1000 + 2;
    int recv_async = -1;
    MPI_Request reqs[2];

    // Post receive first (common pattern), then send.
    MPI_Irecv(&recv_async, 1, MPI_INT, peer, 200, MPI_COMM_WORLD, &reqs[0]);
    MPI_Isend(&send_async, 1, MPI_INT, peer, 200, MPI_COMM_WORLD, &reqs[1]);

    MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
    // m5_workend();
    

    printf("Rank %d (async): received %d\n", rank, recv_async);
    fflush(stdout);
    if (rank == 0) {
        m5_exit(0);
    }

    MPI_Finalize();
    

    return 0;
}
