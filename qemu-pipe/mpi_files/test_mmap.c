#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    /* 1) Anonymous mapping (8 KiB) */
    size_t anon_len = 8192;
    void *p = mmap(NULL, anon_len, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("anon mmap"); return 1; }
    ((char*)p)[0] = 'A'; /* touch it */
    printf("anon mmap ok: %p (%zu bytes)\n", p, anon_len);

    /* 2) File-backed mapping (create temp file, 16 KiB) */
    char path[] = "/tmp/mmap_test.XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) { perror("mkstemp"); return 1; }
    unlink(path); /* remove name; file persists via fd */

    size_t file_len = 16384;
    if (ftruncate(fd, (off_t)file_len) != 0) { perror("ftruncate"); close(fd); return 1; }

    void *q = mmap(NULL, file_len, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (q == MAP_FAILED) { perror("file mmap"); close(fd); return 1; }
    ((char*)q)[0] = 'B';
    printf("file mmap ok: %p (%zu bytes)\n", q, file_len);

    /* Clean up */
    munmap(p, anon_len);
    munmap(q, file_len);
    close(fd);

    puts("done");
    return 0;
}
