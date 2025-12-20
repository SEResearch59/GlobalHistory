#ifndef __EVICT_SET_H
#define __EVICT_SET_H

#include <sys/utsname.h>  
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>  
#include <errno.h>  
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>  
#include <sys/time.h>
#include <semaphore.h>  
#include <pthread.h>

#include <stdint.h>

typedef struct elem
{
    struct elem *next;
    struct elem *prev;
    int set;
    uint64_t delta;
    char pad[32]; // up to 64B
} Elem;
typedef void (*traverser_t)(Elem*);
typedef struct cache_ctx* cache_ctx_t;

__attribute__((always_inline))
static inline void access_one_block(void* p) {
  asm volatile (
    "DSB SY \n\t"
    "LDR x3, [%0] \n\t"
    "DSB SY \n\t"
    :: "r" (p)
    : "x10"
  );
}

static inline uint64_t timer_read() {
    uint64_t begin = 0;
    asm volatile (
        "isb \n"
        "dsb sy\n"
        "mrs %[times], PMCCNTR_EL0\n"
        :[times]"=&r"(begin)
        ::
    );
    return begin;
}

cache_ctx_t evsets_prepare(char* addr);
void evsets_flush(cache_ctx_t ctx);

#endif
