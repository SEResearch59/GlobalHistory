loop:
	gcc Attacker_ii.c Victim.c evict_set.c -DA76=1 -O0 -o attack_ii -lm -g 

if:
# 	gcc Attacker_if.c Victim.c evict_set.c -DA76=1 -O0 -o attack_if -lm
	gcc Attacker_if.c Victim.c -DA76=1 -O0 -o attack_if -lm

rsa:
	g++ Attacker_rsa.cpp evict_set.c -DA76=1 -O0 -fpermissive -fno-operator-names -o attack_rsa

ecdsa:
	gcc Attacker_ecdsa.c -DA76=1 -O0 -o attack_ecdsa -lm

