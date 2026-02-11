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
LDFLAGS_MPI =
TARGET_MPI = mpi_test
SRCS_MPI   = mpi_test.c
OBJS_MPI   = $(SRCS_MPI:.c=.o)

CFLAGS_MPI += $(ROI_INC)

# --- mmap Hook Section ---
CC_MMAP      = gcc
CFLAGS_MMAP  = -O2 -Wall -Wextra -fPIC -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
LDFLAGS_MMAP = -shared -ldl -pthread
TARGET_MMAP  = libmmaphook.so
TEST_MMAP    = test_mmap

.PHONY: all gem5 run mmap run_mmap clean

# --- Default rule builds only the MPI program ---
all: $(TARGET_MPI)

# --- MPI Build Rules ---
$(TARGET_MPI): $(OBJS_MPI)
	$(CC_MPI) $(CFLAGS_MPI) -o $@ $^ $(LDFLAGS_MPI) $(ROI_LIB) $(ROI_OBJ)

%.o: %.c
	$(CC_MPI) $(CFLAGS_MPI) -c $< -o $@

# --- gem5 build target (re-evaluates ifeq by running a sub-make) ---
gem5:
	$(MAKE) -B PLATFORM=gem5 $(TARGET_MPI)

# --- mmap Hook and Test Rules ---
mmap_overload.o: mmap_overload.c
	$(CC_MMAP) $(CFLAGS_MMAP) -c $< -o $@

$(TARGET_MMAP): mmap_overload.o
	$(CC_MMAP) -o $@ $^ $(LDFLAGS_MMAP)

$(TEST_MMAP): test_mmap.c
	$(CC_MMAP) -O2 -Wall -Wextra -o $@ $<

mmap: $(TARGET_MMAP) $(TEST_MMAP)

# --- Run targets ---
run: $(TARGET_MPI)
	mpirun -np 2 ./$(TARGET_MPI)

run_mmap: mmap
	LD_PRELOAD=$(PWD)/libmmaphook.so ./$(TEST_MMAP)

# --- Cleanup ---
clean:
	rm -f $(TARGET_MPI) $(OBJS_MPI) \
	      $(TARGET_MMAP) mmap_overload.o \
	      $(TEST_MMAP)
