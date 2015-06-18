#!/bin/bash

targetfile="$1" # will sign this file

main_path=$(cat chainsign.conf)
fifo_filename="$main_path/fifo"
echo "Will write to $fifo_filename"
echo "fifo write 1"
echo "START" > "$fifo_filename"
echo "START" > "$fifo_filename"
echo "fifo write 2 (targetfile=$targetfile)"
echo "SIGN-NEXTKEY $targetfile" > "$fifo_filename"
echo "fifo write 3"
sleep 1 # TODO why this is needed?
echo "START" > "$fifo_filename"
echo "START" > "$fifo_filename"
echo "fifo write 4"
echo "fifo write 5"
echo "fifo write - ALL Done for saving file $targetfile"

