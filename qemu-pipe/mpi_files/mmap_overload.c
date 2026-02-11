#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
/*
 * mmap_overload.c — keep shm_open normal; redirect certain mmaps to fsdax.
 *
 * Now also records FDs created by open()/openat() under /dev/shm so that
 * OpenMPI paths that don't use shm_open() are recognized.
 */

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

/* forward decls for local logging used by helpers defined later */
static void slog(const char *s);

/* -------- Optional: per-page dump via /proc/self/pagemap (enable with PMEM_DUMP_PAGETABLE=1) ------- */
static int env_dump_enabled(void) {
    const char *v = getenv("PMEM_DUMP_PAGETABLE");
    return (v && *v && (strcmp(v,"0")!=0));
}
static long get_pagesz(void) {
    long ps = sysconf(_SC_PAGESIZE);
    return ps > 0 ? ps : 4096;
}
/* Decode bits from pagemap entry (see Documentation/admin-guide/mm/pagemap.rst) */
static void dump_pagemap_range(void *addr, size_t length, const char *label) {
    if (!env_dump_enabled()) return;

    int fd = open("/proc/self/pagemap", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        slog("[mmap-hook/fsdax] pagemap: open failed (need root or relax kernel.pagemap_restricted?)\n");
        return;
    }

    long ps = get_pagesz();
    unsigned long pages = (length + ps - 1) / ps;
    unsigned long long base = (unsigned long long)(uintptr_t)addr;

    char hdr[256];
    int n = snprintf(hdr, sizeof(hdr),
                     "[mmap-hook/fsdax] PAGEMAP DUMP (%s): base=0x%llx len=%zu pages=%lu\n",
                     (label?label:"region"), base, length, pages);
    if (n > 0) (void)!write(STDERR_FILENO, hdr, (size_t)n);

    for (unsigned long i = 0; i < pages; ++i) {
        unsigned long long va = base + (unsigned long long)i * (unsigned long long)ps;
        off_t off = (off_t)((va / (unsigned long long)ps) * 8ULL);

        unsigned long long entry = 0;
        ssize_t r = pread(fd, &entry, sizeof(entry), off);
        if (r != (ssize_t)sizeof(entry)) {
            slog("[mmap-hook/fsdax] pagemap: short read (likely restricted)\n");
            break;
        }

        int present = (entry >> 63) & 1;
        int swapped = (entry >> 62) & 1;
        unsigned long long pfn = entry & ((1ULL << 55) - 1ULL);

        char line[256];
        int m = snprintf(line, sizeof(line),
                         "  VA 0x%llx : present=%d swapped=%d PFN=0x%llx\n",
                         va, present, swapped, pfn);
        if (m > 0) (void)!write(STDERR_FILENO, line, (size_t)m);

        /* Optional limit to avoid huge spam: PMEM_DUMP_MAXPAGES=N */
        const char *lim = getenv("PMEM_DUMP_MAXPAGES");
        if (lim && *lim) {
            long maxp = strtol(lim, NULL, 10);
            if (maxp > 0 && (long)i+1 >= maxp) {
                slog("[mmap-hook/fsdax] pagemap: truncated by PMEM_DUMP_MAXPAGES\n");
                break;
            }
        }
    }
    close(fd);
}

/* ---------------- real symbols ---------------- */
static int   (*real_shm_open_fn)(const char*, int, mode_t);
static int   (*real_shm_unlink_fn)(const char*);
static void* (*real_mmap_fn)(void*, size_t, int, int, int, off_t);
static int   (*real_open_fn)(const char*, int, ...);
static int   (*real_openat_fn)(int, const char*, int, ...);
static int   (*real_dup_fn)(int);
static int   (*real_dup2_fn)(int, int);
static int   (*real_close_fn)(int);
static int   (*real_memfd_create_fn)(const char*, unsigned int);
static int   (*real_dup3_fn)(int, int, int);
static int   (*real_fcntl_fn)(int, int, ...);

/* ---------------- tiny logging ---------------- */
static void slog(const char *s){ (void)!write(STDERR_FILENO, s, strlen(s)); }
static void slog2(const char *tag, const char *p){
    char b[512]; int n=snprintf(b,sizeof(b),"[mmap-hook/fsdax] %s: %s\n",tag,p?p:"(null)");
    if(n>0)(void)!write(STDERR_FILENO,b,(size_t)n);
}
static void slog3(const char *tag, unsigned long a, unsigned long len, int fd){
    char b[256]; int n=snprintf(b,sizeof(b),"[mmap-hook/fsdax] %s: addr=0x%lx len=%lu fd=%d\n",tag,a,len,fd);
    if(n>0)(void)!write(STDERR_FILENO,b,(size_t)n);
}

/* ---------------- config helpers ---------------- */
static const char* get_env_def(const char* k, const char* def){ const char *v=getenv(k); return (v&&*v)?v:def; }
static size_t get_env_size(const char* k, size_t def){ const char *v=getenv(k); if(!v||!*v) return def; char*e=NULL; unsigned long long x=strtoull(v,&e,10); return (e&&*e=='\0')?(size_t)x:def; }
static const char* base_dir(void){ return get_env_def("PMEM_SHM_DIR","/mnt/pmem/ompi-shm"); }
static size_t redirect_min(void){ return get_env_size("REDIRECT_MIN_BYTES",(size_t)(3.5*1024*1024)); }
static size_t redirect_max(void){ return get_env_size("REDIRECT_MAX_BYTES",(size_t)(4.5*1024*1024)); }

static bool name_whitelisted(const char* name){
    if(!name||!*name) return false;
    const char* def="/ompi.,/pmix,/oshmem,/mpich_,/i_mpi_,/vader_segment";
    const char* list=get_env_def("PMEM_SHM_PREFIXES",def);
    const char* p=list;
    while(*p){
        while(*p==','||*p==' ') p++;
        const char* s=p; while(*p&&*p!=',') p++;
        size_t n=(size_t)(p-s);
        if(n && strncmp(name,s,n)==0) return true;
    }
    return false;
}

/* ---------------- utils ---------------- */
static bool looks_like_devshm(const char* path){ return path && strncmp(path,"/dev/shm/",9)==0; }
static void sanitize_name(const char* in, char* out, size_t n){
    if(!in||n==0){ if(n) out[0]='\0'; return; }
    size_t i=0; for(const char* p=in; *p && i+1<n; ++p) out[i++]= (*p=='/') ? '_' : *p;
    out[i]='\0';
}
static int ensure_dir(const char* dir){
    struct stat st;
    if(stat(dir,&st)==0){ if(S_ISDIR(st.st_mode)) return 0; errno=ENOTDIR; return -1; }
    if(errno!=ENOENT) return -1;
    if(mkdir(dir,0777)==0) return 0;
    if(errno==EEXIST) return 0;
    return -1;
}
static int build_data_path_from_name(const char* name, char* dst, size_t n){
    char sname[NAME_MAX]; sanitize_name(name,sname,sizeof(sname));
    const char* dir=base_dir(); if(ensure_dir(dir)!=0) return -1;
    int r=snprintf(dst,n,"%s/%s.data",dir,sname);
    return (r>0 && (size_t)r<n) ? 0 : -1;
}

/* ---------------- bookkeeping: fd -> shm name ---------------- */
typedef struct { int fd; char name[NAME_MAX]; } fdrec_t;

#define MAXREC 2048
static fdrec_t g_rec[MAXREC];
static pthread_mutex_t g_mu = PTHREAD_MUTEX_INITIALIZER;

static fdrec_t* rec_by_fd_nl(int fd){ for(int i=0;i<MAXREC;i++) if(g_rec[i].fd==fd) return &g_rec[i]; return NULL; }
static fdrec_t* rec_alloc_nl(void){ for(int i=0;i<MAXREC;i++) if(g_rec[i].fd==0) return &g_rec[i]; return NULL; }
static void rec_tag_fd(int fd, const char* name){
    if(fd<0||!name||!*name) return;
    if(!name_whitelisted(name)) return;
    pthread_mutex_lock(&g_mu);
    fdrec_t* r=rec_alloc_nl();
    if(r){ memset(r,0,sizeof(*r)); r->fd=fd; strncpy(r->name,name,sizeof(r->name)-1); }
    pthread_mutex_unlock(&g_mu);
}
static void rec_copy(int newfd,int oldfd){
    pthread_mutex_lock(&g_mu);
    fdrec_t* o=rec_by_fd_nl(oldfd);
    if(o){
        fdrec_t* n=rec_by_fd_nl(newfd);
        if(!n) n=rec_alloc_nl();
        if(n){ *n=*o; n->fd=newfd; }
    }
    pthread_mutex_unlock(&g_mu);
}
static void rec_clear(int fd){
    pthread_mutex_lock(&g_mu);
    fdrec_t* r=rec_by_fd_nl(fd);
    if(r) memset(r,0,sizeof(*r));
    pthread_mutex_unlock(&g_mu);
}

/* ---------------- ctor ---------------- */
__attribute__((constructor))
static void init_syms(void){
    real_shm_open_fn   = dlsym(RTLD_NEXT,"shm_open");
    real_shm_unlink_fn = dlsym(RTLD_NEXT,"shm_unlink");
    real_mmap_fn       = dlsym(RTLD_NEXT,"mmap");
    real_open_fn       = dlsym(RTLD_NEXT,"open");
    real_openat_fn     = dlsym(RTLD_NEXT,"openat");
    real_dup_fn        = dlsym(RTLD_NEXT,"dup");
    real_dup2_fn       = dlsym(RTLD_NEXT,"dup2");
    real_close_fn      = dlsym(RTLD_NEXT,"close");
    real_memfd_create_fn = dlsym(RTLD_NEXT,"memfd_create");
    real_dup3_fn         = dlsym(RTLD_NEXT,"dup3");
    real_fcntl_fn        = dlsym(RTLD_NEXT,"fcntl");
}

/* ---------------- interposed funcs ---------------- */

/* Keep shm_open behavior; just record name->fd so mmap can recognize it. */
int shm_open(const char* name, int oflag, mode_t mode){
    if(!real_shm_open_fn) real_shm_open_fn=dlsym(RTLD_NEXT,"shm_open");
    int fd = real_shm_open_fn ? real_shm_open_fn(name,oflag,mode) : -1;
    if(fd>=0 && name){ rec_tag_fd(fd,name); slog2("shm_open (recorded)",name); }
    return fd;
}

/* Record /dev/shm paths created via open/openat (common on glibc) */
int open(const char* path, int flags, ...){
    if(!real_open_fn) real_open_fn=dlsym(RTLD_NEXT,"open");
    mode_t mode=0; if(flags & O_CREAT){ va_list ap; va_start(ap,flags); mode=(mode_t)va_arg(ap,int); va_end(ap); }
    int fd = real_open_fn(path,flags,mode);
    if(fd>=0 && looks_like_devshm(path)){ /* derive a "name" from path for whitelist check */
        const char* name = path + 8; /* keep leading '/' from basename: "/ompi..." */
        if(name && *name) { rec_tag_fd(fd, name); slog2("open(/dev/shm) (recorded)", name); }
    }
    return fd;
}
int openat(int dirfd, const char* path, int flags, ...){
    if(!real_openat_fn) real_openat_fn=dlsym(RTLD_NEXT,"openat");
    mode_t mode=0; if(flags & O_CREAT){ va_list ap; va_start(ap,flags); mode=(mode_t)va_arg(ap,int); va_end(ap); }
    int fd = real_openat_fn(dirfd,path,flags,mode);
    if(fd>=0 && looks_like_devshm(path)){
        const char* name = path + 8;
        if(name && *name) { rec_tag_fd(fd, name); slog2("openat(/dev/shm) (recorded)", name); }
    }
    return fd;
}
int memfd_create(const char* name, unsigned int flags) {
    if (!real_memfd_create_fn) real_memfd_create_fn = dlsym(RTLD_NEXT,"memfd_create");
    int fd = real_memfd_create_fn ? real_memfd_create_fn(name, flags) : -1;
    if (fd >= 0) {
        char pseudo[NAME_MAX];
        if (name && *name) {
            // Preserve the leading slash expectation for our whitelist matcher
            snprintf(pseudo, sizeof(pseudo), "/%s", name);
        } else {
            snprintf(pseudo, sizeof(pseudo), "/memfd");
        }
        rec_tag_fd(fd, pseudo);
        slog2("memfd_create (recorded)", pseudo);
    }
    return fd;
}
int dup(int oldfd){ if(!real_dup_fn) real_dup_fn=dlsym(RTLD_NEXT,"dup"); int n=real_dup_fn(oldfd); if(n>=0) rec_copy(n,oldfd); return n; }
int dup2(int oldfd,int newfd){ if(!real_dup2_fn) real_dup2_fn=dlsym(RTLD_NEXT,"dup2"); int n=real_dup2_fn(oldfd,newfd); if(n>=0) rec_copy(newfd,oldfd); return n; }
int dup3(int oldfd, int newfd, int flags) {
    if (!real_dup3_fn) real_dup3_fn = dlsym(RTLD_NEXT,"dup3");
    int n = real_dup3_fn(oldfd, newfd, flags);
    if (n >= 0) rec_copy(newfd, oldfd);
    return n;
}
int fcntl(int fd, int cmd, ...) {
    if (!real_fcntl_fn) real_fcntl_fn = dlsym(RTLD_NEXT,"fcntl");
    va_list ap;
    va_start(ap, cmd);
    int ret;

    switch (cmd) {
        case F_DUPFD:
        case F_DUPFD_CLOEXEC: {
            int arg = va_arg(ap, int);
            ret = real_fcntl_fn(fd, cmd, arg);
            va_end(ap);
            if (ret >= 0) rec_copy(ret, fd);
            return ret;
        }
        default: {
            // Forward any other fcntl with the same variadic argument
            // Note: We can't generically know the arg type; pass the next int if present.
            // Many commands take an int; others take a pointer, but we avoid touching them.
            // We call into real_fcntl_fn twice-variadic carefully: pull an int and pass it.
            // If the command doesn't use it, the libc ignores it.
            int maybe = va_arg(ap, int);
            ret = real_fcntl_fn(fd, cmd, maybe);
            va_end(ap);
            return ret;
        }
    }
}
int close(int fd){ if(!real_close_fn) real_close_fn=dlsym(RTLD_NEXT,"close"); rec_clear(fd); return real_close_fn(fd); }

int shm_unlink(const char* name){
    if(!real_shm_unlink_fn) real_shm_unlink_fn=dlsym(RTLD_NEXT,"shm_unlink");
    int rc = real_shm_unlink_fn ? real_shm_unlink_fn(name) : -1;
    /* Optional: remove sidecar .data (derived from name) */
    if(name && name_whitelisted(name)){
        char path[PATH_MAX];
        if(build_data_path_from_name(name,path,sizeof(path))==0){
            if(unlink(path)==0) slog2("shm_unlink removed data file",path);
        }
    }
    return rc;
}

/* Core: redirect target-sized mmaps for recorded shm fds to fsdax file */
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset){
    if(!real_mmap_fn) real_mmap_fn=dlsym(RTLD_NEXT,"mmap");

    const size_t minB=redirect_min(), maxB=redirect_max();
    bool in_range=(length>=minB)&&(length<maxB);
    bool shared=(flags & MAP_SHARED)!=0;

    char dbg[160]; snprintf(dbg,sizeof(dbg),"mmap called len=%zu fd=%d in_range=%d shared=%d\n",length,fd,in_range,shared);
    slog(dbg);

    if(!in_range || !shared || fd<0 || offset!=0){
        void* p=real_mmap_fn(addr,length,prot,flags,fd,offset);
        if(p!=MAP_FAILED){
            snprintf(dbg,sizeof(dbg),"mmap normal ok addr=%p len=%zu\n",p,length); slog(dbg);
            /* Optional dump for non-redirected mappings if enabled */
            dump_pagemap_range(p, length, "normal");
        }
        else slog("mmap normal failed\n");
        return p;
    }

    /* Is this fd one we tagged as a shm segment? */
    pthread_mutex_lock(&g_mu);
    fdrec_t* r=rec_by_fd_nl(fd);
    char name_copy[NAME_MAX]; name_copy[0]='\0';
    if(r && r->name[0]) strncpy(name_copy,r->name,sizeof(name_copy)-1);
    pthread_mutex_unlock(&g_mu);

    if(name_copy[0]=='\0'){
        slog("mmap skipping redirect (unknown fd)\n");
        return real_mmap_fn(addr,length,prot,flags,fd,offset);
    }

    /* Build deterministic fsdax path from shm name */
    char data_path[PATH_MAX];
    if(build_data_path_from_name(name_copy,data_path,sizeof(data_path))!=0){
        slog("[mmap-hook/fsdax] WARN: PMEM_SHM_DIR not ready; using original shm map\n");
        return real_mmap_fn(addr,length,prot,flags,fd,offset);
    }

    int tfd=open(data_path,O_RDWR|O_CREAT,0666);
    if(tfd<0){
        slog2("WARN: open target failed; using original shm map",data_path);
        return real_mmap_fn(addr,length,prot,flags,fd,offset);
    }
    if(ftruncate(tfd,(off_t)length)!=0){
        slog2("WARN: ftruncate target failed; using original shm map",data_path);
        close(tfd);
        return real_mmap_fn(addr,length,prot,flags,fd,offset);
    }

    slog2("Attempting mmap redirect to fsdax",data_path);
    void* p=real_mmap_fn(addr,length,prot,(flags|MAP_SHARED)&~MAP_ANONYMOUS,tfd,0);
    if(p==MAP_FAILED){
        slog2("WARN: mmap target failed; using original shm map",data_path);
        close(tfd);
        return real_mmap_fn(addr,length,prot,flags,fd,offset);
    }

    snprintf(dbg,sizeof(dbg),"SUCCESS: mmap redirected to fsdax file %s addr=%p len=%zu\n",data_path,p,length);
    slog(dbg);
    /* Optionally dump per-page PFNs for the redirected region */
    dump_pagemap_range(p, length, data_path);
    close(tfd);
    return p;
}