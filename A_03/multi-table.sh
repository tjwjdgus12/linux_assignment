#!/bin/bash

if [ $# -ne 2 ]; then
	echo "[Error] Arguments must be two."
	exit -1
fi

if [ $1 -le 0 ] || [ $2 -le 0 ]; then
	echo "[Error] Arguments must be positive."
	exit -1
fi

for i in $(seq 1 $1); do
	for j in $(seq 1 $2); do
		echo -n "$i*$j=`expr $i \* $j` "
	done
	echo
done
