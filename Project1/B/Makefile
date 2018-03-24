# NAME: Natasha Sarkar
# EMAIL: nat41575@gmail.com
# ID: 904743795

default:
	gcc -Wall -Wextra -lz -o lab1b-client lab1b-client.c
	gcc -Wall -Wextra -lz -o lab1b-server lab1b-server.c

client:
	gcc -Wall -Wextra -lz -o lab1b-client lab1b-client.c

server: 
	gcc -Wall -Wextra -lz -o lab1b-server lab1b-server.c

dist: 
	tar -cvzf lab1b-904743795.tar.gz lab1b-server.c lab1b-client.c \
		README Makefile

clean: 
	rm -f lab1b-904743795.tar.gz lab1b-client lab1b-server