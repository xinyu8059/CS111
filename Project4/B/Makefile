# NAME: Natasha Sarkar
# EMAIL: nat41575@gmail.com
# ID: 904743795

default:
	gcc -o lab4b -g -lm -lmraa -Wall -Wextra lab4b.c

check:
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

clean: 
	rm -f lab4b lab4b-904743795.tar.gz

dist: 
	tar -czvf lab4b-904743795.tar.gz lab4b.c Makefile README


