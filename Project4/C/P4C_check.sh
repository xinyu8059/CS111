#!/bin/bash
#
# sanity check script for Project 4B
#	extract tar file
#	required README fields (ID, EMAIL, NAME)
#	required Makefile targets (clean, dist, graphs, tests)
#	make default
#	make dist
#	make clean (returns directory to untared state)
#	make default, success, creates program
#	unrecognized parameters
#	recognizes standard parameters
#	retrieve TCP/TLS session logs
#	    confirm successful identification
#	    confirm successful completion
#	    confirm server validation of all reports

# Note: if you dummy up the sensor sampling
#	this test script can be run on any
#	Linux system.
#
LAB="lab4c"
README="README"
MAKEFILE="Makefile"

EXPECTED=""
EXPECTEDS=".c"
PGMS="lab4c_tcp lab4c_tls"

SUFFIXES=""

LIBRARY_URL="www.cs.ucla.edu/classes/cs111/Software"

EXIT_OK=0
EXIT_ARG=1
EXIT_FAIL=1

TIMEOUT=1

SERVER="lever.cs.ucla.edu"
BASE_URL="http://$SERVER"
BASE_FILE="/var/spool/CS111_P4C"
SERVERLOG="server.log"
CLIENT_PFX="client_"
CLIENT_SFX="log"
TCP_PORT=18000
TLS_PORT=19000

MIN_REPORTS=15

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

echo "... deleting programs and data to force rebuild"
rm -f $PGMS

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
		echo -n "    "
		cat STDERR
	fi
done

echo
# figure out if we are testing with a local or remote server
if [ -d "$BASE_FILE" ]; then
	SERVER="localhost"
fi

# check for successful session records
for p in TCP TLS
do
	# figure out what the correct command is
	if [ "$p" == "TCP" ]; then
		PGM="./lab4c_tcp"
		PORT=$TCP_PORT
	else
		PGM="./lab4c_tls"
		PORT=$TLS_PORT
	fi
	echo "... running $p --id=$student session to $SERVER:$PORT (~1 minute)"
	$PGM --id=$student --host=$SERVER --log=./LOG_$p $PORT

	testRC $? $EXIT_OK
	let errors+=$?

	echo ... checking for logging of all commands and actions
	for key in ID= START PERIOD= SCALE= STOP OFF SHUTDOWN
	do
		grep $key LOG_$p > /dev/null
		if [ $? -ne 0 ]; then
			echo "ERROR: LOG_$p does not record $key"
			let errors+=1
		else
			echo "    logged $key ... OK"
		fi
	done

	egrep '[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9]+\.[0-9]\>' LOG_$p > /dev/null
	if [ $? -eq 0 ]; then
		echo "    valid reports in log file ... OK"
	else
		echo "ERROR: no valid reports in log file"
		let errors+=1
	fi

	# retrieve the server log
	rm -f $SERVERLOG
	sfx="_SERVER"
	if [ -d "$BASE_FILE/$p$sfx" ]; then
		# local script testing
		url="$BASE_FILE/$p$sfx"
		cp $url/$SERVERLOG .
		ok=$?
	else
		# standard testing is from remote server
		url=$BASE_URL/$p$sfx
		wget $url/$SERVERLOG 2> /dev/null
		ok=$?
	fi

	if [ $ok -ne 0 ]; then
		echo "ERROR: Unable to retrieve $SERVERLOG from $url"
		let errors+=1
		continue
	else
		echo "... retrieve $SERVERLOG from $url ... OK"
	fi

	# confirm session identification
	grep "SESSION STARTED: ID=$student" $SERVERLOG > /dev/null
	if [ $? -ne 0 ]; then
		echo "ERROR: No successful $p session establishements for $student"
		let errors+=1
		continue
	else
		echo "    confirm successful $p identification ... OK"
	fi
		
	# confirm completion
	grep "SESSION COMPLETED: ID=$student" $SERVERLOG > HITS
	if [ $? -ne 0 ]; then
		echo "ERROR: No successful $p session completions for $student"
		let errors+=1
		continue
	else
		echo "    confirm successful $p completion ... OK"
	fi

	# confirm a reasonable number of reports
	rpts=`tail -n1 HITS | cut -f3 -d=`
	tot=`echo $rpts | cut -f2 -d/`
	if [ $tot -lt $MIN_REPORTS ]; then
		echo "ERROR: only $tot $p reports received"
		let errors+=1
		continue
	fi

	good=`echo $rpts | cut -f1 -d/`
	if [ $good -ne $tot ]; then
	echo
		echo "ERROR: only $good/$tot valid $p reports"
		let errors+=1
	else
		echo "    good $p reports ... $good/$tot"
	fi
done


#echo "... usage of expected library functions"
#for r in sched_yield pthread_mutex_lock pthread_mutex_unlock __sync_lock_test_and_set __sync_lock_release
#do
#	grep $r *.c > /dev/null
#	if [ $? -ne 0 ] 
#	then
#		echo "No calls to $r"
#		let errors+=1
#	else
#		echo "    ... $r ... OK"
#	fi
#done

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
