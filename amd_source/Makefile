if:
	gcc Attacker_if_amd.c Victim_if.c -DAMD=1 -O0 -o attack_if

ii:
	gcc Attacker_ii_amd.c Victim.c -DAMD=1 -O0 -o attack_ii

loop:
	gcc Attacker_amd_loop.c Victim.c -DAMD=1 -O0 -o attack_loop

rsa:
	gcc Attacker_rsa.c -DAMD=1 -O0 -o attack_rsa

ecdsa:
	gcc Attacker_ecdsa.c -DAMD=1 -O0 -o attack_ecdsa

asm:
	gcc Attacker.c Victim.c -O0 -S
