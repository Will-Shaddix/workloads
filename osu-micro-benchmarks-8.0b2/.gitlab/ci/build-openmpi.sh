#!/bin/bash

set -o pipefail

echo "Gitlab Runner OMB Open MPI Builder"
MAKE_NP=8
OPENMPI_MODULE=${OPENMPI_MODULE:-openmpi/4.0.1}
OPENMPI_DIR=${OPENMPI_DIR:-$BUILD_DIR/OPENMPI}

if [[ -d $OPENMPI_DIR ]]; then
    echo "Open MPI build already exists for:"
    echo $OPENMPI_DIR
    exit 0
fi

echo "Compiling Open MPI build..."
if ! ml load $OPENMPI_MODULE; then
    echo "Unable to load $OPENMPI_MODULE"
    exit 1
fi
OPENMPI_MPICC=$(command -v mpicc || true)
if [[ -z "$OPENMPI_MPICC" ]]; then
    echo "Unable to find Open MPI compiler after loading $OPENMPI_MODULE"
    ml unload $OPENMPI_MODULE
    exit 1
fi
OPENMPI_ROOT=$(dirname "$(dirname "$OPENMPI_MPICC")")

mkdir -p $OPENMPI_DIR
date > $OPENMPI_DIR/time_start
./autogen.sh |& tee autogen-openmpi.log

./configure --prefix=$OPENMPI_DIR \
            CC=$OPENMPI_ROOT/bin/mpicc \
            CXX=$OPENMPI_ROOT/bin/mpicxx \
            LDFLAGS=-L$OPENMPI_ROOT/lib \
            CFLAGS=-I$OPENMPI_ROOT/include \
            |& tee configure-openmpi.log

make -j $MAKE_NP install |& tee install-openmpi.log

if [[ $? -eq 0 ]]; then
    echo "Success. Installed Open MPI build here: $OPENMPI_DIR"
    date > $OPENMPI_DIR/time_end
else
    echo "Error building Open MPI..."
    rm -rf $OPENMPI_DIR
    ml unload $OPENMPI_MODULE
    exit 1
fi
ln -s $OPENMPI_ROOT/bin $OPENMPI_DIR/bin
ln -s $OPENMPI_ROOT/include $OPENMPI_DIR/include
ln -s $OPENMPI_ROOT/lib $OPENMPI_DIR/lib
if [[ -f Makefile ]]; then
    make distclean >> distclean-openmpi.log 2>&1
fi
ml unload $OPENMPI_MODULE
echo "Open MPI Build: $OPENMPI_DIR"
