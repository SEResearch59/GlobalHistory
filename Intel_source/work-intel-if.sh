#!/bin/bash
touch res_intel_if_$1.txt
for ((j=0; j < 300; j++)); do
    taskset -c 2 ./attack_if $1 >> new_res_intel_if_$1.txt
done

