#! /bin/sh
# Title             :test-server.sh
# Description       :This script executes the server for Program 1.
# Author            :Reza Naeemi
# Date              :04/11/2016
###############################################################################


PORT=12721
REPETITION=20000


echo "Server Address: "
ifconfig $1 | grep "inet addr" | grep "Bcast" | awk -F: '{print $2}' | awk '{print $1}'


for NBUF in 15 30 60 100
do
	for TYPE in 1 2 3
	do
		echo "Waiting for Number Of Buffers -> $NBUF, type = $TYPE"
		./server $PORT $REPETITION
	done
done

