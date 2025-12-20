#!/bin/bash
PHR_LEN=180

# make ii
for ((i=0; i < 300; i++)); do
    for ((j=0; j <= $PHR_LEN; j++)); do
        taskset -c 5 ./attack_ii ${j} >> res_ii/cortex-a76_ii_${j}.txt
    done
done
