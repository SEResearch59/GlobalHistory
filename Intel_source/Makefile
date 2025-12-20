if:
	gcc Attacker_if_intel.c Victim_if.c -O0 -o attack_if

ii:
	gcc Attacker_ii_intel.c Victim.c -O0 -o attack_ii

loop:
	gcc Attacker_intel_loop.c Victim.c -O0 -o attack_loop

full_offset:
	gcc Attacker_intel_full_all_off.c Victim.c -O0 -o attack_all_off

asm:
	gcc Attacker.c Victim.c -O0 -S

rsa:
	gcc Attacker_rsa.c -O0 -o attack_rsa

ecdsa:
	gcc Attacker_ecdsa.c -O0 -o attack_ecdsa
