#ifndef ATTACKER_H
#define ATTACKER_H  

#define HIT_THRESHOLD 120
#define PHR_TRAIN_ROUNDS 4
#define ATK_ROUNDS 100
#define TIMES_THRESHOLD (int)(ATK_ROUNDS/3)

#ifdef I9
#define PHR_LEN 194
#else 
    #ifdef AMD
    #define PHR_LEN 120
    #else
    #define PHR_LEN 93
    #endif
#endif

#endif

int PHR_Flush(int, int);
