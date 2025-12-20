#!/bin/bash
make ecdsa
# rm attack_ecdsa.txt
cd ./Modules/ECDSA
# rm attack_ecdsa.txt
# make
for ((i=0;i<10000;i++)); do
    bash rand-key.sh >> amd_attack_ecdsa$1.txt
    sudo rmmod mychardev
    sleep 3
    make > /dev/null
    sleep 4
    sudo insmod mychardev.ko
    cd ..
    sleep 1
    time taskset -c 2 ./attack_ecdsa 1 6 >> new_amd_attack_ecdsa$1.txt
    cd ./Modules/ECDSA
    echo $i
done
