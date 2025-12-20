make rsa
for ((i=1;i<100;i++)); do
	taskset -c 2 ./attack_rsa 6 >> result_rsa.txt
done
