# ===== Makefile for MPI example and mmap hook =====

# --- MPI Example Section ---
CC_MPI     = mpicc
CFLAGS_MPI = -O2 -Wall -Wextra
TARGET_MPI = mpi_test
SRCS_MPI   = mpi_test.c
OBJS_MPI   = $(SRCS_MPI:.c=.o)

# --- mmap Hook Section ---
CC_MMAP      = gcc
CFLAGS_MMAP  = -O2 -Wall -Wextra -fPIC -D_GNU_SOURCE -D_LARGEFILE64_SOURCE
LDFLAGS_MMAP = -shared -ldl -pthread
TARGET_MMAP  = libmmaphook.so
TEST_MMAP    = test_mmap

# --- Default rule builds only the MPI program ---
all: $(TARGET_MPI)

# --- MPI Build Rules ---
$(TARGET_MPI): $(OBJS_MPI)
	$(CC_MPI) $(CFLAGS_MPI) -o $@ $^

%.o: %.c
	$(CC_MPI) $(CFLAGS_MPI) -c $< -o $@

# --- mmap Hook and Test Rules ---
mmap_overload.o: mmap_overload.c
	$(CC_MMAP) $(CFLAGS_MMAP) -c $< -o $@

$(TARGET_MMAP): mmap_overload.o
	$(CC_MMAP) -o $@ $^ $(LDFLAGS_MMAP)

$(TEST_MMAP): test_mmap.c
	$(CC_MMAP) -O2 -Wall -Wextra -o $@ $<

# Combined target to build both the library and test program
mmap: $(TARGET_MMAP) $(TEST_MMAP)

# --- Run targets ---
run: $(TARGET_MPI)
	mpirun -np 2 ./$(TARGET_MPI)

# Example: Run mmap test with LD_PRELOAD
run_mmap: mmap
	LD_PRELOAD=$(PWD)/libmmaphook.so ./$(TEST_MMAP)

# --- Cleanup ---
clean:
	rm -f $(TARGET_MPI) $(OBJS_MPI) \
	      $(TARGET_MMAP) mmap_overload.o \
	      $(TEST_MMAP)
