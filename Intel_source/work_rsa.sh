make rsa
for ((i=1;i<50;i++)); do
	taskset -c 2 ./attack_rsa $i >> result_rsa.txt
done
