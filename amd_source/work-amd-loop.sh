#!/bin/bash
# input $1: attack times
# input $2: cpu core number
PHR_LEN=120
make loop
fi
for ((j=0; j < $1; j++)); do
    for ((i=0; i < $PHR_LEN; i++)); do
        taskset -c $2 ./attack_loop $i >> loop_res/res_r5-5600_$2_$i.txt
    done
done
