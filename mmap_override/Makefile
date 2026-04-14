# ===== Makefile for MPI example and mmap hook =====

ROI_DIR=$(PWD)/../annotate
ROI_BIN=
ROI_OBJ=
ROI_INC=-I$(ROI_DIR)/include
ROI_LIB=

ifeq ($(PLATFORM), gem5)
	ROI_BIN+=$(ROI_DIR)/lib/libannotate.gem5fs.so
	ROI_INC+=-I$(ROI_DIR)/gem5/include
	ROI_LIB+=-L$(ROI_DIR)/gem5/lib/arm
	ROI_OBJ+=-lm5
else
	ROI_BIN+=$(ROI_DIR)/lib/libannotate.native.so
endif

# --- MPI Example Section ---
CC_MPI     = mpicc
CFLAGS_MPI = -O2 -Wall -Wextra
ifeq ($(PLATFORM), gem5)
CFLAGS_MPI += -DGEM5
endif
ifdef ROI_PER_ITERATION
CFLAGS_MPI += -DROI_PER_ITERATION
endif
LDFLAGS_MPI =
TARGETS_MPI = mpi_test_all mpi_test_send mpi_test_isend mpi_test_ssend mpi_test_bcast mpi_test_send_alt mpi_test_isend_alt mpi_test_ssend_alt mpi_test_allgather mpi_test_groupsend mpi_test_isendall mpi_test_isend_fill
# mpi_test
CFLAGS_MPI += $(ROI_INC)

# --- mmap Hook Section ---
CC_MMAP      = gcc
CFLAGS_MMAP  = -O2 -Wall -Wextra -fPIC -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
LDFLAGS_MMAP = -shared -ldl -pthread
TARGET_MMAP  = libmmaphook.so
TEST_MMAP    = test_mmap

.PHONY: all gem5 run mmap run_mmap clean

# --- Default rule builds both MPI and mmap programs ---
all: $(TARGETS_MPI) mmap

# --- MPI Build Rules ---
$(TARGETS_MPI): %: %.o
	$(CC_MPI) $(CFLAGS_MPI) -o $@ $^ $(LDFLAGS_MPI) $(ROI_LIB) $(ROI_OBJ) $(ROI_BIN)

%.o: %.c
	$(CC_MPI) $(CFLAGS_MPI) -c $< -o $@

# --- gem5 build target (re-evaluates ifeq by running a sub-make) ---
gem5:
	$(MAKE) -B PLATFORM=gem5 all

# --- mmap Hook and Test Rules ---
mmap_overload.o: mmap_overload.c
	$(CC_MMAP) $(CFLAGS_MMAP) -c $< -o $@

$(TARGET_MMAP): mmap_overload.o
	$(CC_MMAP) -o $@ $^ $(LDFLAGS_MMAP)

$(TEST_MMAP): test_mmap.c
	$(CC_MMAP) -O2 -Wall -Wextra -o $@ $<

mmap: $(TARGET_MMAP) $(TEST_MMAP)

# --- Run targets ---
run: $(TARGETS_MPI)
	mpirun -np 2 ./mpi_test

run_mmap: mmap
	LD_PRELOAD=$(PWD)/libmmaphook.so ./$(TEST_MMAP)

# --- Cleanup ---
clean:
	rm -f $(TARGETS_MPI) *.o \
	      $(TARGET_MMAP) \
	      $(TEST_MMAP)
