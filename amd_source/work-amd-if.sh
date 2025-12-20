#!/bin/bash
for ((j=0; j < 300; j++)); do
    taskset -c 2 ./attack_if $1 >> res_amd_if_$1.txt
done

