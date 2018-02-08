/*
 * Name:	Natasha Sarkar
 * Email: 	nat41575@gmail.com
 * ID:    	904743795
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define INPUT 'i'
#define OUTPUT 'o'
#define SEGFAULT 's'
#define CATCH 'c'

void create_segfault() {
	char* ptr = NULL;
	*ptr = 'n';
	return;
}

void handle_seg() {
	fprintf(stderr, "Segmentation fault caught, exiting.\n");
	exit(4);
}

int main(int argc, char* argv[]) {

	int val;
	int seg_flag = 0;
	int input_file;
	int output_file;

	struct option options[] = { 
		{"input", required_argument, NULL, INPUT},
		{"output", required_argument, NULL, OUTPUT},
		{"segfault", no_argument, NULL, SEGFAULT},
		{"catch", no_argument, NULL, CATCH},
		{0, 0, 0, 0}
	};

	while ((val = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (val) {
			case INPUT:
				//open a file and make it the new stdin
				input_file = open(optarg, O_RDONLY);
				if (input_file >= 0) {
					dup2(input_file, 0);
				} else {
					fprintf(stderr, "Cannot open input file %s: ", optarg);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(2);
				} break;

			case OUTPUT:
				//create a file and make it the new stdout
				output_file = creat(optarg, 0666);
				if (output_file >= 0) {
					dup2(output_file, 1);
				} else {
					fprintf(stderr, "Cannot create output file %s: ", optarg);
					fprintf(stderr, "%s\n", strerror(errno));
					exit(3);
				} break;

			case SEGFAULT:
				//set the seg_flag rather than calling the function
				//directly here so that if they have --segfault 
				//followed by a --catch, the segfault will be caught
				seg_flag = 1;
				break;

			case CATCH:
				//set up handler for segfault
				signal(SIGSEGV, handle_seg);
				break;

			default:
				fprintf(stderr, "Illegal option: options --input:filename --output:filename --segfault and --catch are allowed \n");
				exit(1);
		}
	}

	if (seg_flag) {
		create_segfault();
	}

	char input;
	while (read(0, &input, sizeof(char)) > 0) {
		if (write(1, &input, sizeof(char)) < 0) {
			fprintf(stderr, "Cannot write to output file: %s\n", strerror(errno));
			exit(3); //bc output file could not be created
		}
	}

	close(0);
	close(1);
	exit(0);
}