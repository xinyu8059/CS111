#!/bin/bash

{ echo "START"; sleep 2; echo "STOP"; sleep 2; echo "OFF"; } | ./lab4b --log=log.txt

if [ $? -ne 0 ]
then
	echo "Error: program should have exited with 0"
else
	echo "good return value!"
fi

for c in START STOP OFF SHUTDOWN
	do
		grep "$c" log.txt > /dev/null
		if [ $? -ne 0 ]
		then
			echo "failed to log $c command"
		else
			echo "$c was logged successfully!"
		fi
	done

rm -f log.txt