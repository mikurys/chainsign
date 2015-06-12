#!/bin/bash
#if [x]
echo "START" > ~/chainsign/fifo
echo "SIGN-NEXTKEY" > ~/chainsign/fifo
echo "$1" > ~/chainsign/fifo