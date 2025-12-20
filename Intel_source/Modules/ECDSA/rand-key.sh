#!/bin/bash

r=$(./rand-key)
sed -Ei "s/0x[0-9a-f]+/0x$r/" mychardev.c
echo $r