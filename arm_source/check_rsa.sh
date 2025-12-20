for ((i = 0; i < 200; i++)); do
	taskset -c 5 ./attack_rsa 19 >> rsa_res_$1.txt
	sleep 0.1
done

