#ifndef ATTACKER_RSA_H
#define ATTACKER_RSA_H  

#define HIT_THRESHOLD 90
#define PHR_TRAIN_ROUNDS 8
#define ATK_ROUNDS 120
#define TIMES_THRESHOLD (int)(ATK_ROUNDS/3)

#ifdef I9
    #define PHR_LEN 194
#else 
    #ifdef AMD
    	#define PHR_LEN 120
    #else
	#ifdef A55
    	    #define PHR_LEN 13
    	#else
            #ifdef A76
		#define PHR_LEN 48
	    #else
		#ifdef KP920
			#define PHR_LEN 64
		#else
			#define PHR_LEN 93
		#endif
	    #endif
	#endif
    #endif
#endif

#endif

int PHR_Flush(int, int);
