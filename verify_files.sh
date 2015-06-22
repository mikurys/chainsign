#!/bin/bash
# $1 = file type (png, mp3, txt, ...)
# $2 = files dir

cp chainsign $2
cd $2
for f in  $(find "$2" -name "*.$1")
do
	echo "start verify $f"
	echo "./chainsign --verify-file $f"
	./chainsign --verify-file "$f"
	if [ "$?" -ne "1" ] ; then
		echo "*********VERIFICATION ERROR*********"
		break
	fi
done
