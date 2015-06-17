#!/bin/bash

main_path=$(cat chainsign.conf)
echo "START" > "$main_path/fifo"
echo "SIGN-NEXTKEY" > "$main_path/fifo"
sleep 1
echo "$1" > "$main_path/fifo"