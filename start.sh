#!/bin/bash
cd ~/chainsign
mkfifo fifo
echo "START" > fifo
./chainsign --daemon