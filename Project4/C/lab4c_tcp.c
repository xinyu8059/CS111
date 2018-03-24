/* 
 * Name: Natasha Sarkar
 * Email: nat41575@gmail.com
 * ID: 904743795
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>

#define A0 1
#define TO_SERVER 1

/* GLOBALS */
struct tm *curr_time;
struct timeval my_clock;
time_t next_time = 0;
mraa_aio_context temp; 

//for options
char scale = 'F';
int period = 1;
FILE *file = 0;
int report = 1;
int port = -1;
char* id = "";

//host and socket structures
struct hostent *server;
char* host = "";
struct sockaddr_in server_address;
int sock;


/* gets temperature */
double get_temp() {
	int temperature = mraa_aio_read(temp);
	int therm = 4275;
	float nom = 100000.0;
	float R = 1023.0/((float) temperature) - 1.0;
	R *= nom;
	float ret = 1.0/(log(R/nom)/therm + 1/298.15) - 273.15; 
	if (scale == 'F') { //convert temperature to F
		ret = (ret * 9)/5 + 32; 
	}
	return ret; 
}

/* prints to stdout and file */
void print(char *str, int to_server) {
	if (to_server) {
		dprintf(sock, "%s\n", out);
	}
	fprintf(stderr, "%s\n", out);
	fprintf(file, "%s\n", out);
	fflush(file);
}

/* shuts down, prints shut down time */
void shutdown_and_exit(){
	curr_time = localtime(&my_clock.tv_sec);
	char out[200];
	sprintf(out, "%02d:%02d:%02d SHUTDOWN", curr_time->tm_hour, 
    		curr_time->tm_min, curr_time->tm_sec);
    	print(out, TO_SERVER);
   	exit(0);
}

/* prints out the time stamp and temperature */
void time_stamp() {
	gettimeofday(&my_clock, 0);
	if (report && my_clock.tv_sec >= next_time) {
		double temp = get_temp();
		int t = temp * 10;
		curr_time = localtime(&my_clock.tv_sec);
		char out[200];
		sprintf(out, "%02d:%02d:%02d %d.%1d", curr_time->tm_hour, 
				curr_time->tm_min, curr_time->tm_sec, t/10, t%10);
		print(out, TO_SERVER);
		next_time = my_clock.tv_sec + period; 
	}
}

/* parse input from stdin */
void process_input(char *input) {
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
		shutdown_and_exit();
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

/* reads input from the server */
void server_input(char* input) {
	int ret = read(sock, input, 256);
	//fprintf(stderr, "Ret is %d\n", ret);
	if (ret > 0) {
		input[ret] = 0;
		//fprintf(stderr, "Received %d bytes %s\n", ret, input);
	}
	char *s = input;
	while (s < &input[ret]) {
		char* e = s;
		while (e < &input[ret] && *e != '\n') {
			e++;
		}
		*e = 0;
		//fprintf(stderr, "processing %s\n", s);
		process_input(s);
		s = &e[1];
	}
}


int main(int argc, char* argv[]) {

	struct option options[] = {
		{"period", required_argument, NULL, 'p'},
		{"log", required_argument, NULL, 'l'},
		{"id", required_argument, NULL, 'i'},
   		{"scale", required_argument, NULL, 's'},
		{"host", required_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	int opt; //parse options
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 'p': 
				period = atoi(optarg);
				break;
			case 'l':
				file = fopen(optarg, "w+");
            	if (file == NULL) {
	           		fprintf(stderr, "Logfile invalid\n");
					exit(1);
				}
				break;
			case 's':
				if (optarg[0] == 'F' || optarg[0] == 'C') {
					scale = optarg[0];
					break;
				}
			case 'i':
				id = optarg;
				break;
			case 'h':
				host = optarg;
				break;
			default:
				fprintf(stderr, "Error in arguments.\n");
				exit(1);
		}
	}
	if (optind < argc) {
		port = atoi(argv[optind]);
		if (port <= 0) {
			fprintf(stderr, "Invalid port\n");
			exit(1);
		}
	}

	if (strlen(host) == 0) {
		fprintf(stderr, "Host argument is mandatory\n");
		exit(1);
	}
	if (file == 0) {
		fprintf(stderr, "Log argument is mandatory\n");
		exit(1);
	}
	if (strlen(id) != 9) {
		fprintf(stderr, "ID must be a 9 digit number\n");
		exit(1);
	} 

	/* Open a TCP connection to the server at the specified address and port */

	//create a socket and find host
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Failure to create socket in client program.\n");
		exit(1);
	}
	if ((server = gethostbyname(host)) == NULL) {
		fprintf(stderr, "Could not find host\n");
	}
	memset( (void *) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	memcpy( (char *) &server_address.sin_addr.s_addr, 
		    (char *) server->h_addr, 
		    server->h_length);
	server_address.sin_port = htons(port);
	if (connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
      		fprintf(stderr, "Failure to connect on client side.\n");
      		exit(1);
  	}

	/* Immediately send (and log) an ID terminated with a newline. */
	char out[50];
	sprintf(out, "ID=%s", id);
	print(out, TO_SERVER);

	/* 
	 * Send (and log) temperature reports,
	 * process (and log) commands received over connection.
	 */

	temp = mraa_aio_init(A0);

	if (temp== NULL) {
        fprintf(stderr, "Failed to initialize AIO\n");
        mraa_deinit();
        return EXIT_FAILURE;
    }

	struct pollfd pollInput; 
	pollInput.fd = sock; 
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
			server_input(input);
		}
	}

	mraa_aio_close(temp);
	exit(0);
}
