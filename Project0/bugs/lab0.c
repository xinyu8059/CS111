/*
NAME: Brandon Oestereicher
EMAIL: brandon.oestereicher@gmail.com
ID: 604825250
*/

#include <unistd.h>
#include <getopt.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void sighandler() {
	fprintf(stderr, "Caught segmentation fault, exiting program\n");
	exit(4);
}

void seg_fault(char* fault) {
	*fault = 'x';
	return;
}

int main(int argc, char* argv[]) {
	//fstat exists
	int op = 0;
	static struct option long_ops[] = {
	{"input", required_argument, 0, 'i'},
	{"output", required_argument, 0, 'o'},
	{"segfault", no_argument, 0, 's'},
	{"catch", no_argument, 0, 'c'},
	{0, 0, 0, 0}
	};
	int ifd;
	int ofd;
	char* fault = NULL;
	int cause_fault = 0;
	while ((op = getopt_long(argc, argv, "i:o:sc", long_ops, NULL)) != -1) { //colon after options that need args
		switch (op) {
			case 'i':
				ifd = open(optarg, O_RDONLY);
				if (ifd >= 0) {
					close(0);
					dup2(ifd, 0);
					close(ifd);
				}
				else {
					//fprintf(stderr, "Cannot access input file %s\n", optarg);
					//perror("Error");
					fprintf(stderr, "Cannot access input file %s\n", optarg);
					fprintf(stderr, "%s", strerror(errno));
					exit(2);
				}
				break;
			case 'o':
				ofd = creat(optarg, 0666);
				if (ofd >= 0) {
					close(1);
					dup2(ofd, 1);
					close(ofd);
				}
				else if (ofd < 0){
					fprintf(stderr, "Cannot access output file %s\n", optarg);
					//perror("Error");
					fprintf(stderr, "%s", strerror(errno));
					exit(3);
				}
				break;
			case 's':
				cause_fault = 1;
				break;
			case 'c':
				signal(SIGSEGV, sighandler);
				break;
			default:
				fprintf(stderr, "Invalid option\n");
				fprintf(stderr, "Valid options are: --input=filename --output=filename --segfault and --catch\n");
				exit(1);
		}
	}
	if (cause_fault) {
		seg_fault(fault);
	}
	

	//better reading and writing :(
	char in;
	while (read(0, &in, 1) > 0) {
		if (write(1, &in, 1) < 0) {
				//error
			fprintf(stderr, "Cannot write to output file\n");
			//perror("Error");
			fprintf(stderr, "%s", strerror(errno));
			exit(3);
		}
	}
	close(0);
	close(1);
	exit(0);
}