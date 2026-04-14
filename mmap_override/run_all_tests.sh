#!/bin/bash

# Default number of processes is 2, unless overridden by an argument or environment variable
NP=${NP:-${1:-2}}
LOOPS=${LOOPS:-${2:-1}}
LOGFILE="mpi_test_results.log"

TESTS=(
    "mpi_test_send"
    "mpi_test_isend"
    "mpi_test_ssend"
    "mpi_test_bcast"
    "mpi_test_send_alt"
    "mpi_test_isend_alt"
    "mpi_test_ssend_alt"
    "mpi_test_allgather"
    "mpi_test_groupsend"
    "mpi_test_isendall"
    "mpi_test_isend_fill"
)

echo "Starting MPI test suite with ${NP} processes..." | tee $LOGFILE
echo "=============================================" | tee -a $LOGFILE

for TEST in "${TESTS[@]}"; do
    if [ ! -f "./$TEST" ]; then
        echo "Warning: Binary ./$TEST not found! Have you run 'make all'?" | tee -a $LOGFILE
        continue
    fi

    echo "" | tee -a $LOGFILE
    echo "=============================================" | tee -a $LOGFILE
    echo "▶ RUNNING TEST: $TEST (NP=${NP}, LOOPS=${LOOPS})" | tee -a $LOGFILE
    echo "=============================================" | tee -a $LOGFILE
    
    # Run the test, appending stdout and stderr to the logfile
    mpirun -np $NP ./$TEST $LOOPS >> $LOGFILE 2>&1
    
    # Check the exit status of the MPI command to see if it crashed
    if [ $? -eq 0 ]; then
        echo "✔ $TEST completed successfully." | tee -a $LOGFILE
    else
        echo "✖ $TEST FAILED." | tee -a $LOGFILE
    fi
done

echo "" | tee -a $LOGFILE
echo "All tests finished. Check $LOGFILE for details." | tee -a $LOGFILE
