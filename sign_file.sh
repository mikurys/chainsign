#!/bin/bash
#if [x]
echo "START" > ~/chainsign/fifo
echo "SIGN-NEXTKEY" > ~/chainsign/fifo
sleep 1
echo "$1" > ~/chainsign/fifo