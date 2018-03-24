#!/bin/bash
#
# sanity check script for Project 2A
#	extract tar file
#	required README fields (ID, EMAIL, NAME)
#	required Makefile targets (clean, dist, graphs, tests)
#	make default
#	make dist
#	make clean (returns directory to untared state)
#	make default, success, creates client/server
#	add recognizes standard parameters, produces plausible output
#	list recognizes standard parameters, produces plausible output
#	use of expected functions
#
LAB="lab2a"
README="README"
MAKEFILE="Makefile"

SOURCES="lab2_add.c lab2_list.c SortedList.c SortedList.h"
DATA="lab2_add.csv lab2_list.csv"
GRAPHS="lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png"

EXPECTED="$SOURCES $DATA $GRAPHS"

ADD_PGM=./lab2_add
LIST_PGM=./lab2_list
PGMS="$ADD_PGM $LIST_PGM"

SUFFIXES=""

LIBRARY_URL="www.cs.ucla.edu/classes/cs111/Software"

EXIT_OK=0
EXIT_ARG=1
EXIT_FAIL=2

TIMEOUT=1

let errors=0

if [ -z "$1" ]
then
	echo usage: $0 your-student-id
	exit 1
else
	student=$1
fi

# make sure the tarball has the right name
tarball="$LAB-$student.tar.gz"
if [ ! -s $tarball ]
then
	echo "ERROR: Unable to find submission tarball:" $tarball
	exit 1
fi

# get copy of our grading/checking functions
if [ -s functions.sh ]; then
	source functions.sh
else
	wget $LIBRARY_URL/functions.sh 2> /dev/null
	if [ $? -eq 0 ]; then
		>&2 echo "Downloading functions.sh from $LIBRARY_URL"
		source functions.sh
	else
		>&2 echo "FATAL: unable to pull test functions from $LIBRARY_URL"
		exit -1
	fi
fi

# read the tarball into a test directory
TEMP="/tmp/CS111_test.$LOGNAME"
if [ -d $TEMP ]
then
	echo Deleting old $TEMP
	rm -rf $TEMP
fi
mkdir $TEMP
unTar $LAB $student $TEMP
cd $TEMP

# note the initial contents
dirSnap $TEMP $$

echo "... checking for README file"
checkFiles $README
let errors+=$?

echo "... checking for submitter ID in $README"
ID=`getIDs $README $student`
let errors+=$?

echo "... checking for submitter email in $README"
EMAIL=`getEmail $README`
let errors+=$?

echo "... checking for submitter name in $README"
NAME=`getName $README`
let errors+=$?

echo "... checking for other expected files"
checkFiles $MAKEFILE $EXPECTED
let errors+=$?

# make sure we find files with all the expected suffixes
if [ -n "$SUFFIXES" ]; then
	echo "... checking for other files of expected types"
	checkSuffixes $SUFFIXES
	let errors+=$?
fi

echo "... checking for required Make targets"
checkTarget clean
let errors+=$?
checkTarget tests
let errors+=$?
checkTarget graphs
let errors+=$?
checkTarget dist
let errors+=$?

echo "... checking for required compillation options"
checkMakefile Wall
let errors+=$?
checkMakefile Wextra
let errors+=$?

# make sure we can build the expected program
echo "... building default target(s)"
make 2> STDERR
testRC $? 0
let errors+=$?
noOutput STDERR
let errors+=$?

echo "... deleting all data and graphs to force rebuild"
rm -f $PGMS $DATA $GRAPHS

echo "... checking make dist"
make dist 2> STDERR
testRC $? 0
let errors+=$?

checkFiles $TARBALL
if [ $? -ne 0 ]; then
	echo "ERROR: make dist did not produce $tarball"
	let errors+=1
fi

echo " ... checking make clean"
rm -f STDERR
make clean
testRC $? 0
let errors+=$?
dirCheck $TEMP $$
let errors+=$?

#
# now redo the default make and start testing functionality
#
echo "... redo default make"
make 2> STDERR
testRC $? 0
let errors+=$?
noOutput STDERR
let errors+=$?

echo "... checking for expected products"
checkPrograms $PGMS
let errors+=$?

# see if they detect and report invalid arguments
for p in $PGMS
do
	echo "... $p detects/reports bogus arguments"
	./$p --bogus < /dev/tty > /dev/null 2>STDERR
	testRC $? $EXIT_ARG
	if [ ! -s STDERR ]
	then
		echo "No Usage message to stderr for --bogus"
		let errors+=1
	else
		echo -n "        "
		cat STDERR
	fi
done


# sanity test on basic output
#	--threads=
#	--iterations=
#	--sync=
#	--yield
#
iterations=2
threads=1
opsper=2
for s in m c s
do
	echo "... testing $ADD_PGM --iterations=$iterations --threads=$threads --yield --sync=$s"
	$ADD_PGM --iterations=$iterations --threads=$threads --yield --sync=$s > STDOUT

	# check return code
	testRC $? 0
	let errors+=$?

	record=`cat STDOUT`

	# check number of fields
	numFields "$record" 7
	let errors+=$?

	fieldValue "$record" "output tag" 1 "add-yield-$s"
	let errors+=$?

	fieldValue "$record" "threads" 2 "$threads"
	let errors+=$?

	fieldValue "$record" "iterations" 3 "$iterations"
	let errors+=$?

	fieldValue "$record" "operations" 4 "$((threads*iterations*opsper))"
	let errors+=$?

	# check field 5 = total time per run
	fieldRange "$record" "time/run" 5 2 10000000
	let errors+=$?

	# check field 6 = time per operation
	t=`echo $record | cut -d, -f5`
	let avg=$((t/(threads*iterations*opsper)))
	fieldRange "$record" "time/op" 6 $((avg-1)) $((avg+1))
	let errors+=$?

	# check field 7 = sum
	fieldValue "$record" "sum" 7 0
	let errors+=$?
done

# sanity test on basic output
#	--threads=
#	--iterations=
#	--yield=
#	--sync=
#
# (a) are the parameters recognized, and do they seem to have effect
# (b) is the output propertly formatted and consistent with the parameters

#
threads=1
iterations=2
lists=1
yield="idl"
opsper=3
for s in m s
do
	echo "... testing $LIST_PGM --iterations=$iterations --threads=$threads --yield=$yield --sync=$s"
	$LIST_PGM --iterations=$iterations --threads=$threads --yield=$yield --sync=$s > STDOUT
	testRC $? 0
	let errors+=$?
	record=`cat STDOUT`

	# check number of fields (2A: 7, 2B: 8)
	numFields "$record" 7 8
	let errors+=$?

	fieldValue "$record" "output tag" 1 "list-$yield-$s"
	let errors+=$?

	fieldValue "$record" "threads" 2 "$threads"
	let errors+=$?

	fieldValue "$record" "iterations" 3 "$iterations"
	let errors+=$?

	fieldValue "$record" "lists" 4 "$lists"
	let errors+=$?

	fieldValue "$record" "operations" 5 "$((threads*iterations*opsper))"
	let errors+=$?

	# check field 6 = total time per run
	fieldRange "$record" "time/run" 6 2 10000000
	let errors+=$?

	# check field 7 = time per operation
	t=`echo $record | cut -d, -f6`
	let avg=$((t/(threads*iterations*opsper)))
	fieldRange "$record" "time/op" 7 $((avg-1)) $((avg+1))
	let errors+=$?
done

echo "... usage of expected library functions"
for r in sched_yield pthread_mutex_lock pthread_mutex_unlock __sync_val_compare_and_swap __sync_lock_test_and_set __sync_lock_release
do
	grep $r *.c > /dev/null
	if [ $? -ne 0 ] 
	then
		echo "No calls to $r"
		let errors+=1
	else
		echo "    ... $r ... OK"
	fi
done

echo
echo "THE ONLY STUDENTS WHO WILL RECEIVE CREDIT FOR THIS SUBMISSION ARE:"
commas=`echo $ID | tr -c -d "," | wc -c`
let submitters=commas+1
let f=1
while [ $f -le $submitters ]
do
	id=`echo $ID | cut -d, -f$f`
	mail=`echo $EMAIL | cut -d, -f$f`
	echo "    $id    $mail"
	let f+=1
done
echo

# delete temp files, report errors, and exit
cleanup $$ $errors
