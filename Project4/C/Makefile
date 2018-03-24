# NAME: Natasha Sarkar
# EMAIL: nat41575@gmail.com
# ID: 904743795

default: 
	gcc -o lab4c_tcp -Wall -Wextra -lm -lmraa -g lab4c_tcp.c
	gcc -o lab4c_tls -Wall -Wextra -lm -lmraa -g -lssl -lcrypto lab4c_tls.c

clean:
	rm -f lab4c-904743795.tar.gz lab4c_tcp lab4c_tls

dist:
	tar -czvf lab4c-904743795.tar.gz README Makefile lab4c_tls.c lab4c_tcp.c
