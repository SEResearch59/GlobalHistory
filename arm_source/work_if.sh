#!/bin/bash

coreid=$1
for ((i = 0; i < 300; i++)); do
	taskset -c ${coreid} ./attack_if $2 >> $3 
done
