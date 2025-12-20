#!/bin/bash
PHR_LEN=60
make ii
for ((i=0; i < $PHR_LEN; i++)); do
    for ((j=0; j < 300; j++)); do
        taskset -c 2 ./attack_ii $i >> loop_ii_res/res_r5-5600_$i.txt
    done
done
