/* 
 * Name: Natasha Sarkar
 * Email: nat41575@gmail.com
 * ID: 904743795
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <ctype.h>
#include <mraa.h>

#define A0 1
#define GPIO_50 60

char scale = 'F';
int period = 1;
FILE *file = 0;
int report = 1;
struct tm *curr_time;
struct timeval my_clock;
time_t next_time = 0;

mraa_aio_context temp; 
mraa_gpio_context button; 

/* gets temperature */
float get_temp() {
	int temperature = mraa_aio_read(temp);
	int therm = 4275;
	float nom = 100000.0;
	float R = 1023.0/((float) temperature) - 1.0;
	R *= nom;
	float ret = 1.0/(log(R/nom)/therm + 1/298.15) - 273.15; 
	if (scale == 'F') { //convert temperature to F
		return (ret * 9)/5 + 32; 
	} else { //return value in C
		return ret; 
	}
}

/* prints to stdout and file */
void print(char *str, int to_stdout) {
	if (to_stdout == 1) {
		fprintf(stdout, "%s\n", str);
	} 

	if (file != 0) {
		fprintf(file, "%s\n", str);
		fflush(file);
	}
}

/* shuts down, prints shut down time */
void shutdown() {
	curr_time = localtime(&my_clock.tv_sec);
	char out[200];
	sprintf(out, "%02d:%02d:%02d SHUTDOWN", curr_time->tm_hour, 
    		curr_time->tm_min, curr_time->tm_sec);
    	print(out,1);
    	exit(0);
}

/* prints out the time stamp and temperature */
void time_stamp() {
	gettimeofday(&my_clock, 0);
	if (report && my_clock.tv_sec >= next_time) {
		float temp = get_temp();
		int t = temp * 10;
		curr_time = localtime(&my_clock.tv_sec);
		char out[200];
    		sprintf(out, "%02d:%02d:%02d %d.%1d", curr_time->tm_hour, 
    			curr_time->tm_min, curr_time->tm_sec, t/10, t%10);
    		print(out,1);
    		next_time = my_clock.tv_sec + period; 
	}
}

/* parse input from stdin */
void process_stdin(char *input) {
	int EOL = strlen(input); 
	input[EOL-1] = '\0';
	while(*input == ' ' || *input == '\t') {
		input++;
	}
	char *in_per = strstr(input, "PERIOD=");
	char *in_log = strstr(input, "LOG");

	if(strcmp(input, "SCALE=F") == 0) {
		print(input, 0);
		scale = 'F'; 
	} else if(strcmp(input, "SCALE=C") == 0) {
		print(input, 0);
		scale = 'C'; 
	} else if(strcmp(input, "STOP") == 0) {
		print(input, 0);
		report = 0;
	} else if(strcmp(input, "START") == 0) {
		print(input, 0);
		report = 1;
	} else if(strcmp(input, "OFF") == 0) {
		print(input, 0);
		shutdown();
	} else if(in_per == input) {
		char *n = input;
		n += 7; 
		if(*n != 0) {
			int p = atoi(n);
			while(*n != 0) {
				if (!isdigit(*n)) {
					return;
				}
				n++;
			}
			period = p;
		}
		print(input, 0);
	} else if (in_log == input) {
		print(input, 0); 
	}
}


int main(int argc, char* argv[]) {

	struct option options[] = {
		{"period", required_argument, NULL, 'p'},
   		{"scale", required_argument, NULL, 's'},
    		{"log", required_argument, NULL, 'l'},
    		{0, 0, 0, 0}
	};

	int opt;

	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 'p': 
				period = atoi(optarg);
				break;

			case 'l':
				file = fopen(optarg, "w+");
            	if(file == NULL) {
            		fprintf(stderr, "Logfile invalid\n");
					exit(1);
				}
				break;

			case 's':
				if (optarg[0] == 'F' || optarg[0] == 'C') {
					scale = optarg[0];
					break;
				}

			default:
				fprintf(stderr, "Error in arguments.\n");
				exit(1);
				break;
		}
	}

	temp = mraa_aio_init(A0);

	if (temp== NULL) {
        	fprintf(stderr, "Failed to initialize AIO\n");
        	mraa_deinit();
        	return EXIT_FAILURE;
    	}

    	button = mraa_gpio_init(GPIO_50);

    	if (button == NULL) {
        	fprintf(stderr, "Failed to initialize GPIO_50\n");
        	mraa_deinit();
        	return EXIT_FAILURE;
    	}

    	mraa_gpio_dir(button, MRAA_GPIO_IN);
    	mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &shutdown, NULL);

	struct pollfd pollInput; 
    		pollInput.fd = STDIN_FILENO; 
    		pollInput.events = POLLIN; 

    	char *input;
    	input = (char *)malloc(1024 * sizeof(char));
    	if(input == NULL) {
    		fprintf(stderr, "Could not allocate input buffer\n");
    		exit(1);
    	}

    	while(1) {
  		time_stamp();
    		int ret = poll(&pollInput, 1, 0);
    		if(ret) {
    			fgets(input, 1024, stdin);
    			process_stdin(input); 
    		}
	}

	mraa_aio_close(temp);
    	mraa_gpio_close(button);

    	return 0;
}
