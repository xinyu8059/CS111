#NAME: Brandon Oestereicher
#EMAIL: brandon.oestereicher@gmail.com
#ID: 604825250

default:
	gcc -o lab0 -g -Wall -Wextra lab0.c
	
check:
	./smoketest.sh
	
clean:
	rm -f lab0 lab0-604825250.tar.gz
	
dist:
	tar -czvf lab0-604825250.tar.gz lab0.c Makefile backtrace.png breakpoint.png README
