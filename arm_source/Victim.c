#include <stdio.h>
#include "Victim.h"

__attribute__((always_inline))
static inline void flush_instruction_cache_line(void *addr) {
    // 1. DC CVAU: Clean Data Cache by VA to Point of Unification
    asm volatile ("dc cvau, %0" : : "r" (addr) : "memory");

    // 2. DSB ISH: Data Synchronization Barrier
    asm volatile ("dsb ish" : : : "memory");

    // 3. IC IVAU: Invalidate Instruction Cache by VA to Point of Unification
    asm volatile ("ic ivau, %0" : : "r" (addr) : "memory");

    // 4. DSB ISH: 
    asm volatile ("dsb ish" : : : "memory");

    // 5. ISB: Instruction Synchronization Barrier
    asm volatile ("isb" : : : "memory");
}

int func01(int zero1)
{
    int i, sum = 225103161;
	if (zero1 & 1) {
		// asm volatile (
		// "isb \n"
		// "dsb sy\n":::"memory");
		flush_instruction_cache_line(func01);
		sum &= 0x114514;
	}
    	
    
    return sum;
}

int func(int length)
{
	int i = -1, sum = 0;
	// for (i = 0; i < length; i++)
	// 	sum += i;
	do {
		sum += i;
		i++;
	} while(i < length);
	return sum;
}
