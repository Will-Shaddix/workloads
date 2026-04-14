#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include "annotate.h"
#include <string.h>

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

typedef enum { SEND, ISEND, SSEND, BCAST, SEND_ALT, ISEND_ALT, SSEND_ALT, ALLGATHER, GROUPSEND, ISEND_ALL, UNKNOWN_SEND } SendType;

int main(int argc, char** argv) {
    M5_RESET_STATS();

    MPI_Init(&argc, &argv);
    printf("MPI_Init success\n");

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    printf("MPI Rank and Size = %d, %d success\n", rank, size);

    // int send_type = 0;
    char* send_type = "send";
    
    int num_loops = 1;
    if (argc > 1) {
        num_loops = atoi(argv[1]);
    }
    if (argc > 2) {
        send_type = argv[2];
    }

    SendType type_enum = UNKNOWN_SEND;
    if (strcmp(send_type, "send") == 0) type_enum = SEND;
    else if (strcmp(send_type, "isend") == 0) type_enum = ISEND;
    else if (strcmp(send_type, "ssend") == 0) type_enum = SSEND;
    else if (strcmp(send_type, "bcast") == 0) type_enum = BCAST;
    else if (strcmp(send_type, "send_alt") == 0) type_enum = SEND_ALT;
    else if (strcmp(send_type, "isend_alt") == 0) type_enum = ISEND_ALT;
    else if (strcmp(send_type, "ssend_alt") == 0) type_enum = SSEND_ALT;
    else if (strcmp(send_type, "allgather") == 0) type_enum = ALLGATHER;
    else if (strcmp(send_type, "groupsend") == 0) type_enum = GROUPSEND;
    else if (strcmp(send_type, "isendall") == 0) type_enum = ISEND_ALL;
    else {
        if (rank == 0) fprintf(stderr, "Unknown send type: %s\n", send_type);
        MPI_Abort(MPI_COMM_WORLD, 1);
        ROI_END();
    }
    ROI_END_LOOP();

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        M5_CHECKPOINT();
    }

    int send_peer = (rank + 1) % size;
    int recv_peer = (rank + size - 1) % size; // Add size to prevent negative modulus
    // Use send_peer in all MPI_Send/MPI_Isend
    // Use recv_peer in all MPI_Recv/MPI_Irecv

    int send_tag = 100 + send_peer;
    int recv_tag = 100 + rank;
    int recv_bcast[size];
    int recv_val = -1;

    annotate_init_();

    ROI_BEGIN_LOOP();
    for (int i = 0; i < num_loops; i++) {
        ROI_BEGIN();
        int send_val = rank * 100 + i;
        MPI_Status st;
        MPI_Request reqs[2];
        

        switch (type_enum) {
            case SEND:
                if ((rank % 2) == 0) {
                    MPI_Send(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD);
                    MPI_Recv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &st);
                } else {
                    MPI_Recv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &st);
                    MPI_Send(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD);
                }
                break;
            case ISEND:
                MPI_Irecv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &reqs[0]);
                MPI_Isend(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD, &reqs[1]);
                MPI_Waitall(2, reqs, MPI_STATUSES_IGNORE);
                break;
            case SSEND:
                if ((rank % 2) == 0) {
                    MPI_Ssend(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD);
                    MPI_Recv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &st);
                } else {
                    MPI_Recv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &st);
                    MPI_Ssend(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD);
                }
                break;
            case BCAST:
                recv_bcast[rank] = send_val;
                MPI_Bcast(&(recv_bcast[rank]), 1, MPI_INT, 0, MPI_COMM_WORLD);
                break;
            case ALLGATHER:
                // Every rank contributes 'send_val'
                // The results are placed sequentially into the 'recv_bcast' array
                MPI_Allgather(&send_val, 1, MPI_INT, recv_bcast, 1, MPI_INT, MPI_COMM_WORLD);
                // After this runs, recv_bcast[0] has Rank 0's value, recv_bcast[1] has Rank 1's, etc.
                break;

            case SEND_ALT:
                if ((i + rank) % 2 == 0) {
                    MPI_Send(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD);
                } else {
                    MPI_Recv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &st);
                }
                break;
            case ISEND_ALT:
                if ((i + rank) % 2 == 0) {
                    MPI_Isend(&send_val, 1, MPI_INT, send_peer, send_tag, MPI_COMM_WORLD, &reqs[1]);
                    MPI_Wait(&reqs[1], MPI_STATUS_IGNORE);
                } else {
                    MPI_Irecv(&recv_val, 1, MPI_INT, recv_peer, recv_tag, MPI_COMM_WORLD, &reqs[0]);
                    MPI_Wait(&reqs[0], MPI_STATUS_IGNORE);
                }
                break;
            case SSEND_ALT:
                break;
            case GROUPSEND:
                // need to look over
                if (rank != 0) {
                    // Send to node 0
                    MPI_Send(&send_val, 1, MPI_INT, 0, 100 + rank, MPI_COMM_WORLD);
                } else {
                    // Node 0 receives from all other nodes
                    for (int p = 1; p < size; p++) {
                        MPI_Recv(&recv_bcast[p], 1, MPI_INT, p, 100 + p, MPI_COMM_WORLD, &st);
                    }
                    recv_bcast[0] = send_val;
                }
                break;
            case ISEND_ALL:
            {
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
                break;
            }
            default:
                break;
        }
        
        ROI_END();
    }
    ROI_END_LOOP();

    MPI_Barrier(MPI_COMM_WORLD);

    // Print the result on the last loop to verify correctness
    printf("Rank %d (%s): final received %d\n", rank, send_type, recv_val);
    fflush(stdout);

    if (rank == 0) {
        M5_EXIT();
    }

    MPI_Finalize();

    return 0;
}
