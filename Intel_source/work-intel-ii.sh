#!/bin/bash
PHR_LEN=46
make ii
for ((i=0; i <= $PHR_LEN; i++)); do
    for ((j=0; j < 300; j++)); do
        taskset -c 2 ./attack_ii $i >> loop_ii_res/res_i710700_$i.txt
    done
done
