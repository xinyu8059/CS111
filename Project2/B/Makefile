#NAME: Natasha Sarkar
#EMAIL: nat41575@gmail.com
#ID: 904743795

default:
	gcc -Wall -Wextra -g -o lab2_list -lpthread -lprofiler lab2_list.c SortedList.c

tests: default
	./lab2b_tests.sh
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=1 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=12 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=4 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=8 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=1 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=2 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=4 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=8 >> lab2b_list.csv
	-./lab2_list --threads=16 --yield=id --lists=4 --iterations=16 >> lab2b_list.csv

profile: default
	rm -f ./raw.gperf profile.out
	LD_PRELOAD=/u/cs/ugrad/sriharsh/lib/libprofiler.so.0
	CPUPROFILE=./raw.gperf ./lab2_list --iterations=1000 --threads=12 --sync=s
	pprof --text ./lab2_list ./raw.gperf > profile.out
	pprof --list=threadFunc ./lab2_list ./raw.gperf >> profile.out
	rm -f ./raw.gperf profile.gperf lab2_list

graphs: tests
	./lab2b_list.gp

dist: graphs profile
	tar -czvf lab2b-904743795.tar.gz SortedList.h SortedList.c lab2_list.c Makefile \
		lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png \
		lab2b_5.png README lab2b_tests.sh lab2b_list.gp

clean: 
	rm -f lab2b-904743795.tar.gz lab2_list 