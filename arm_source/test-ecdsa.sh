#!/bin/bash

# rm attack_ecdsa.txt

# cd ~/Documents/taozikang_from_hit_temp_dir/myModules/toy_ECDSA

cd ./Modules/ECDSA

# rm attack_ecdsa.txt
make
for ((i=0;i<1;i++)); do
    bash rand-key.sh >> attack_ecdsa$1.txt
    sudo rmmod mychardev
    sleep 3
    make > /dev/null
    sleep 4
    sudo insmod mychardev.ko

    cd ../..
    sleep 1
    time taskset -c 5 ./attack_ecdsa 1 21 >> attack_ecdsa$1.txt

    cd ./Modules/ECDSA
    echo $i
done
