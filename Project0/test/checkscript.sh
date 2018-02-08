#!/bin/bash

# Name:		Natasha Sarkar
# Email: 	nat41575@gmail.com
# ID:    	904743795

#run the program
`./lab0 --input="file1" --output="file2"`

#check that the program exited successfully
if [ $? -eq 0 ]
then
	echo "Success: Program exited with exit code 0"
else
	echo "Error: Program exited with an error"
fi

#compare input and output files
`cmp file1 file2` 
if [ $? -eq 0 ]
then 
	echo "Success: Copied input to output"
else 
	echo "Error: Input file does not match output file"
fi

#create an output file, remove write permission, and attempt
#to write to it. the program should exit with code 3
`touch file3`
`chmod u-w file3`
`./lab0 --input="file1" --output="file3" 2> /dev/null`
if [ $? -eq 3 ]
then
	echo "Success: Couldn't write to file, exit code 3"
else
	echo "Error: Wrong exit code for being unable to write to file"
fi

#check segfault and catching creation
`./lab0 --segfault --catch 2> /dev/null`
if [ $? -eq 4 ]
then 
	echo "Success: Segmentation fault created and caught"
else 
	echo "Error: Did not create/catch segfault"
fi

#check that when an input file doesn't exist, exits with code 2
`./lab0 --input="doesntexist" 2> /dev/null`
if [ $? -eq 2 ]
then 
	echo "Success: Input file couldn't be read, exit code 2"
else 
	echo "Error: Wrong exit code for being unable to open a file"
fi

`rm -f file2 file3`