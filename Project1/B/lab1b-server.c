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
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <zlib.h>

#define CR '\015' //carriage return
#define LF '\012' //line feed
#define EOT '\004' //^D (End of transmission)
#define ETX '\003' //^C (End of text)

int pipe_ptoc[2];
int pipe_ctop[2];
int pid;
char crlf[2] = {CR, LF};
int sock;
int sock2;
z_stream client_to_server;
z_stream server_to_client;

void shell_exit_status() {
	int status;
	waitpid(pid, &status, 0);
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WIFSIGNALED(status), WEXITSTATUS(status));
}

void handle_sigpipe() {
	close(pipe_ptoc[1]);
	close(pipe_ctop[0]);
	kill(pid, SIGKILL);
	shell_exit_status();
	exit(0);
}

int main(int argc, char* argv[]) {
	/* supports --port and --compress options */

	struct option options[] = {
		{"port", required_argument, NULL, 'p'},
    	{"compress", no_argument, NULL, 'c'},
    	{0, 0, 0, 0}
	};

	int portno = 0;
	int compress_flag = 0;
	int port_flag = 0;

	int opt;
	while ( (opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 'p': 
				portno = atoi(optarg);
				port_flag = 1;
				break;

			case 'c':
				compress_flag = 1;

				server_to_client.zalloc = Z_NULL;
				server_to_client.zfree = Z_NULL;
				server_to_client.opaque = Z_NULL;

				if (deflateInit(&server_to_client, Z_DEFAULT_COMPRESSION) != Z_OK) {
					fprintf(stderr, "Failure to deflateInit on client side.\n");
					exit(1);
				}

				client_to_server.zalloc = Z_NULL;
				client_to_server.zfree = Z_NULL;
				client_to_server.opaque = Z_NULL;

				if (inflateInit(&client_to_server) != Z_OK) {
					fprintf(stderr, "Failure to inflateInit on client side.\n");
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

	/* Create a socket */

	unsigned int client_size;
	struct sockaddr_in server_address, client_address;
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "Failure to create socket in server program.\n");
		exit(1);
	}

	memset( (void *) &server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(portno);
	if (bind(sock, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
		fprintf(stderr, "Error with binding.\n");
		exit(1);
	}
	listen(sock, 8);
	client_size = sizeof(client_address);
	if ((sock2 = accept(sock, (struct sockaddr *) &client_address, &client_size)) < 0) {
		fprintf(stderr, "Error with accepting.\n");
		exit(1);
	}

	//create pipes
	if (pipe(pipe_ctop) != 0) {
		fprintf(stderr, "Failed to create pipe from terminal to shell.\n");
		exit(1);
	}
	if (pipe(pipe_ptoc) != 0) {
		fprintf(stderr, "Failed to create pipe from shell to terminal.\n");
		exit(1);
	}

	signal(SIGPIPE, handle_sigpipe);

	pid = fork();

	if (pid == 0) { //child (shell) process
		close(pipe_ptoc[1]); //write end from parent to child
		close(pipe_ctop[0]); //read end from child to parent

		/* replace STDIN, STDOUT, and STDERR with pipes */
		dup2(pipe_ptoc[0], STDIN_FILENO); //read from parent to child
		dup2(pipe_ctop[1], STDOUT_FILENO); //write from child to parent
		dup2(pipe_ctop[1], STDERR_FILENO);

		close(pipe_ptoc[0]);
		close(pipe_ctop[1]);

		/* exec a shell /bin/bash with no arguments other than its name */
		char *arguments[2];
		char filename[] = "/bin/bash";
		arguments[0] = filename;
		arguments[1] = NULL;

		if (execvp(filename, arguments) == -1) {
			fprintf(stderr, "Failed to exec a shell.\n");
			exit(1);
		}

		/* The shell will automatically read from the pipe and execute. */

	} else if (pid > 0) { //parent (terminal) process

		close(pipe_ptoc[0]); //read end from parent to child
		close(pipe_ctop[1]); //write end from child to parent

		struct pollfd descriptors[] = {
			{sock2, POLLIN, 0}, //socket
			{pipe_ctop[0], POLLIN, 0} //output from shell
		};
			
		int EOT_flag = 0;
		int i;

		int status;
		while (!EOT_flag) {

			if (waitpid (pid, &status, WNOHANG) != 0) {
				close(sock);
				close(sock2);
				close(pipe_ctop[0]);
				close(pipe_ptoc[1]);
				fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WIFSIGNALED(status), WEXITSTATUS(status));
				exit(0);
			}

			if (poll(descriptors, 2, 0) > 0) {

				short revents_socket = descriptors[0].revents;
				short revents_shell = descriptors[1].revents;

				/* check that socket has pending input */
				if (revents_socket == POLLIN) {
					char input[256];
					int num = read(sock2, &input, 256);

					if (compress_flag) {
						//decompress
						//fprintf(stderr, "server decompressing data\n");
						char compression_buf[1024];
						client_to_server.avail_in = num;
						client_to_server.next_in = (unsigned char *) input;
						client_to_server.avail_out = 1024;
						client_to_server.next_out = (unsigned char *) compression_buf;

						do {
							inflate(&client_to_server, Z_SYNC_FLUSH);
						} while (client_to_server.avail_in > 0);

						for (i = 0; (unsigned int) i < 1024 - client_to_server.avail_out; i++) {
							if (compression_buf[i] == ETX) {
								kill(pid, SIGINT); 
							} else if (compression_buf[i] == EOT) {
							    EOT_flag = 1;
							} else if (compression_buf[i] == CR || compression_buf[i] == LF) { 
								char lf = LF;
								write(pipe_ptoc[1], &lf, 1);
							} else { 
								write(pipe_ptoc[1], (compression_buf + i), 1);
							}
						}

					} else {
						//no compress option
						for (i = 0; i < num; i++) {
							if (input[i] == ETX) {
								kill(pid, SIGINT); 
							} else if (input[i] == EOT) {
							    EOT_flag = 1;
							} else if (input[i] == CR || input[i] == LF) { 
								char lf = LF;
								write(pipe_ptoc[1], &lf, 1);
							} else { 
								write(pipe_ptoc[1], (input + i), 1);
							}
						}
					}

				} else if (revents_socket == POLLERR) {
					fprintf(stderr, "Error with poll from socket.\n");
					exit(1);
				}

				/* check that the shell has pending input */
				if (revents_shell == POLLIN) {
					char input[256];
					int num = read(pipe_ctop[0], &input, 256); 

					int count = 0;
					int j;
					for (i = 0, j = 0; i < num; i++) {
						if (input[i] == EOT) { //EOF from shell
							EOT_flag = 1;

						} else if (input[i] == LF) {

							if (compress_flag) {
								//fprintf(stderr, "server compressing data\n");
								//compress
								char compression_buf[256];
								server_to_client.avail_in = count;
								server_to_client.next_in = (unsigned char *) (input + j);
								server_to_client.avail_out = 256;
								server_to_client.next_out = (unsigned char *) compression_buf;

								do {
									deflate(&server_to_client, Z_SYNC_FLUSH);
								} while (server_to_client.avail_in > 0);

								write(sock2, compression_buf, 256 - server_to_client.avail_out);

								//compress crlf
								char compression_buf2[256];
								char crlf_copy[2] = {CR, LF};
								server_to_client.avail_in = 2;
								server_to_client.next_in = (unsigned char *) (crlf_copy);
								server_to_client.avail_out = 256;
								server_to_client.next_out = (unsigned char *) compression_buf2;

								do {
									deflate(&server_to_client, Z_SYNC_FLUSH);
								} while (server_to_client.avail_in > 0);

								write(sock2, compression_buf2, 256 - server_to_client.avail_out);

							} else {
								//no compress option
								write(sock2, (input + j), count);
								write(sock2, crlf, 2);
							}

							j += count + 1;
							count = 0;
							continue;
						}
						count++;
					}

					//compress 
					write(sock2, (input+j), count);		

				} else if (revents_shell & POLLERR || revents_shell & POLLHUP) { //polling error
					EOT_flag = 1;
				} 
			} 
		}

		close(sock);
		close(sock2);
		close(pipe_ctop[0]);
		close(pipe_ptoc[1]);
		shell_exit_status();
		exit(0);

	} else { //fork failed
		fprintf(stderr, "Could not fork process.\n");
		exit(1);
	}

	if (compress_flag) {
		inflateEnd(&client_to_server);
		deflateEnd(&server_to_client);
	}
}