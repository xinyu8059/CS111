#!/bin/sh

#NAME: Natasha Sarkar
#EMAIL: nat41575@gmail.com
#ID: 904743795

rm -f lab2b_list.csv 

./lab2_list --threads=1  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m	>> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=16  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=24  --iterations=1000 --sync=m >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=16  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=24  --iterations=1000 --sync=s >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=10 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=20 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=40 --sync=m >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=80 --sync=m >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
./lab2_list --threads=1 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
./lab2_list --threads=12 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
./lab2_list --threads=4 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
./lab2_list --threads=8 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=10 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=20 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=40 --sync=s >> lab2b_list.csv
./lab2_list --threads=16 --yield=id --lists=4 --iterations=80 --sync=s >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m	--lists=4 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=m --lists=4 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s	--lists=4 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=s --lists=4 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=8 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=m --lists=8 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s	--lists=8 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=s --lists=8 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=m	--lists=16 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=m	--lists=16 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=m --lists=16 >> lab2b_list.csv
./lab2_list --threads=1  --iterations=1000 --sync=s	 --lists=16 >> lab2b_list.csv
./lab2_list --threads=2  --iterations=1000 --sync=s	 --lists=16 >> lab2b_list.csv
./lab2_list --threads=4  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv
./lab2_list --threads=8  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv
./lab2_list --threads=12  --iterations=1000 --sync=s  --lists=16 >> lab2b_list.csv