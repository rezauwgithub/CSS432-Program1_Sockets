#! /bin/sh
# Title             :test-client.sh
# Description       :This script executes the client for Program 1.
# Author            :Reza Naeemi
# Date              :04/11/2016
###############################################################################

BUFSIZE=1500
PORT=12721
REPETITION=20000
SERVER=$1

for NBUFS in 15 30 60 100
do
	BUFFER_SIZE=$(($BUFSIZE / $NBUFS))

	for TYPE in 1 2 3
	do
		./client $PORT $REPETITION $NBUFS $BUFFER_SIZE $SERVER $TYPE
	done
done

