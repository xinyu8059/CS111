/* 
 * Name: 	Natasha Sarkar
 * Email: 	nat41575@gmail.com
 * ID: 		904743795
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <zlib.h>

#define CR '\015' //carriage return
#define LF '\012' //line feed
char newline = '\n';
char crlf[2] = {CR, LF};
int log_fd;

z_stream client_to_server;
z_stream server_to_client;

struct termios original_mode;
int sock;

//the function that will be called upon normal process termination
void restore(void) {
	close(sock);
	close(log_fd);
	tcsetattr(STDIN_FILENO, TCSANOW, &original_mode);
}

/* Puts the standard input into non-canonical input mode with no echo */
void set_mode(void) {
	struct termios mode;

	if (!isatty(STDIN_FILENO)) {
		fprintf(stderr, "Standard input does not refer to a terminal.\n");
		exit(1);
	}

	tcgetattr(STDIN_FILENO, &original_mode);
	atexit(restore);

	tcgetattr(STDIN_FILENO, &mode);

	mode.c_iflag = ISTRIP;
	mode.c_oflag = 0;
	mode.c_lflag = 0;
	mode.c_cc[VMIN] = 1;
	mode.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSANOW, &mode);
}

int main(int argc, char* argv[]) {
	/* supports --log, --port, and --compress arguments */
	struct option options[] = {
		{"port", required_argument, NULL, 'p'},
   		{"log", required_argument, NULL, 'l'},
    		{"compress", no_argument, NULL, 'c'},
    		{0, 0, 0, 0}
	};

	int portno = 0;
	int log_flag = 0;
	log_fd = -1;
	int port_flag = 0;
	int compress_flag = 0;

	int opt;
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 'p': 
				portno = atoi(optarg);
				port_flag = 1;
				break;

			case 'l':
				log_flag = 1;
				if ((log_fd = creat(optarg, 0666)) == -1) {
					fprintf(stderr, "Failure to create/write to file.\n");
				}

				break;

			case 'c':
				compress_flag = 1;

				server_to_client.zalloc = Z_NULL;
				server_to_client.zfree = Z_NULL;
				server_to_client.opaque = Z_NULL;

				if (inflateInit(&server_to_client) != Z_OK) {
					fprintf(stderr, "Failure to inflateInit on client side.\n");
					exit(1);
				}

				client_to_server.zalloc = Z_NULL;
				client_to_server.zfree = Z_NULL;
				client_to_server.opaque = Z_NULL;

				if (deflateInit(&client_to_server, Z_DEFAULT_COMPRESSION) != Z_OK) {
					fprintf(stderr, "Failure to deflateInit on client side.\n");
					exit(1);
				}

				break;

			default:
				fprintf(stderr, "Error in arguments.\n");
				exit(1);
				break;
		}
	}

	if (!port_flag) {
		fprintf(stderr, "--port= option is mandatory.\n");
		exit(1);
	}

	set_mode();

	/* Create a socket */

	struct sockaddr_in server_address;
	struct hostent* server;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Failure to create socket in client program.\n");
		exit(1);
	}

	server = gethostbyname("localhost");

	memset( (void * ) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	memcpy( (char *) &server_address.sin_addr.s_addr, 
		    (char *) server->h_addr, 
		    server->h_length);

	server_address.sin_port = htons(portno);

	if (connect(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
      		fprintf(stderr, "Failure to connect on client side.\n");
      		exit(1);
  	}

	/*
	 * Use poll(2) to 
	 * 1) Read input from the keyboard, echo to stdout, and forward
	 *    it to the socket.
	 * 2) Read input from the socket and echo it to stdout.
	 * Data should be passed in both directions as soon as it is
	 * received. 
	 */ 

	struct pollfd descriptors[] = {
		{STDIN_FILENO, POLLIN, 0}, //keyboard (stdin)
		{sock, POLLIN, 0} //socket
	};

	int i;
	char sending_prefix[20] = "SENT ";
	char sending_end[20] = " bytes: ";
	char receiving_prefix[20] = "RECEIVED ";
	char receiving_end[20] = " bytes: ";

	while (1) {
		if (poll(descriptors, 2, 0) > 0) {

			short revents_stdin = descriptors[0].revents;
			short revents_socket = descriptors[1].revents;

			/* check that stdin has pending input */
			if (revents_stdin == POLLIN) {
				char input[256];
				int num = read(STDIN_FILENO, &input, 256);
				for (i = 0; i < num; i++) {
					if (input[i] == CR || input[i] == LF) { 
						write(STDOUT_FILENO, crlf, 2);			
					} else { 
						write(STDOUT_FILENO, (input + i), 1);
					}
				}

				if (compress_flag) {
					//compress data
					//fprintf(stderr, "client compressing data\n");
					char compression_buf[256];
					client_to_server.avail_in = num;
					client_to_server.next_in = (unsigned char *) input;
					client_to_server.avail_out = 256;
					client_to_server.next_out = (unsigned char *) compression_buf;

					do {
						deflate(&client_to_server, Z_SYNC_FLUSH);
					} while (client_to_server.avail_in > 0);

					write(sock, compression_buf, 256 - client_to_server.avail_out);

					if (log_flag) {
						char num_bytes[20];
						sprintf(num_bytes, "%d", 256 - client_to_server.avail_out);
						write(log_fd, sending_prefix, strlen(sending_prefix));
						write(log_fd, num_bytes, strlen(num_bytes));
						write(log_fd, sending_end, strlen(sending_end));
						write(log_fd, compression_buf, 256 - client_to_server.avail_out);
						write(log_fd, &newline, 1);
					}

				} else {
					//no compress option
					write(sock, input, num);

					if (log_flag) {
						char num_bytes[20];
						sprintf(num_bytes, "%d", num);
						write(log_fd, sending_prefix, strlen(sending_prefix));
						write(log_fd, num_bytes, strlen(num_bytes));
						write(log_fd, sending_end, strlen(sending_end));
						write(log_fd, input, num);
						write(log_fd, &newline, 1);
					}
				}

			} else if (revents_stdin == POLLERR) {
				fprintf(stderr, "Error with poll with STDIN.\n");
				exit(1);
			}

			/* check that the socket has pending input */
			if (revents_socket == POLLIN) {
				char input[256];
				int num = read(sock, &input, 256); 
				if (num == 0) {
					break;
				}

				if (compress_flag) {
					//decompress data
					//fprintf(stderr, "client decompressing data\n");
					char compression_buf[1024];
					server_to_client.avail_in = num;
					server_to_client.next_in = (unsigned char *) input;
					server_to_client.avail_out = 1024;
					server_to_client.next_out = (unsigned char *) compression_buf;

					do {
						inflate(&server_to_client, Z_SYNC_FLUSH);
					} while (server_to_client.avail_in > 0);

					write(STDOUT_FILENO, compression_buf, 1024 - server_to_client.avail_out);

				} else {
					// no compress option
					write(STDOUT_FILENO, input, num);
				}

				if (log_flag) {
					char num_bytes[20];
					sprintf(num_bytes, "%d", num);
					write(log_fd, receiving_prefix, strlen(receiving_prefix));
					write(log_fd, num_bytes, strlen(num_bytes));
					write(log_fd, receiving_end, strlen(receiving_end));
					write(log_fd, input, num);
					write(log_fd, &newline, 1);
				}

			} else if (revents_socket & POLLERR || revents_socket & POLLHUP) { //polling error
				exit(0);
			} 
		}
	}

	if (compress_flag) {
		inflateEnd(&server_to_client);
		deflateEnd(&client_to_server);
	}
	
	exit(0);
}
