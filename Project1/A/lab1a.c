/* 
 * Name: 	Natasha Sarkar
 * Email:	nat41575@gmail.com
 * ID:		904743795
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

#define CR '\015' //carriage return
#define LF '\012' //line feed
#define EOT '\004' //^D (End of transmission)
#define ETX '\003' //^C (End of text)

/* the termios struct that saves the original terminal mode, so that
   it can be restored upon exit */
struct termios original_mode;

int pipe_ptoc[2];
int pipe_ctop[2];
int pid;
char crlf[2] = {CR, LF};

//the function that will be called upon normal process termination
void restore(void) {
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

	//what the professor suggested to do to accomplish the goal
	mode.c_iflag = ISTRIP;
	mode.c_oflag = 0;
	mode.c_lflag = 0;
	mode.c_cc[VMIN] = 1;
	mode.c_cc[VTIME] = 0;

	tcsetattr(STDIN_FILENO, TCSANOW, &mode);
}

void shell_exit_status() {
	int status;
	waitpid(pid, &status, 0);
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WIFSIGNALED(status), WEXITSTATUS(status));
}

void handle_sigpipe() {
	close(pipe_ptoc[1]);
	close(pipe_ctop[0]);
	kill(pid, SIGINT);
	shell_exit_status();
	exit(0);
}

int main (int argc, char* argv[]) {
	/* support a --shell argument */
	struct option options[] = {
		{"shell", no_argument, NULL, 's'},
		{0, 0, 0, 0}
	};

	int shell_opt = 0; //flag for shell option
	int opt;
	while ( (opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 's': 
				shell_opt = 1;
				break;
			default:
				fprintf(stderr, "Only the option --shell is permitted.\n");
				exit(1);
		}
	}

	/*
	 * SHELL OPTION
	 */
	if (shell_opt) {

		//create pipes
		if (pipe(pipe_ctop) != 0) {
			fprintf(stderr, "Failed to create pipe from terminal to shell.\n");
			exit(1);
		}
		if (pipe(pipe_ptoc) != 0) {
			fprintf(stderr, "Failed to create pipe from shell to terminal.\n");
			exit(1);
		}
		
		set_mode();
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
			
			/*
			 * Use poll(2) to 
			 * 1) Read input from the keyboard, echo to stdout, and forward
			 *    it to the shell.
			 * 2) Read input from the shell pipe and write it to stdout.
			 * Data should be passed in both directions as soon as it is
			 * received. 
			 */ 

			struct pollfd descriptors[] = {
				{STDIN_FILENO, POLLIN, 0}, //keyboard (stdin)
				{pipe_ctop[0], POLLIN, 0} //output from shell
			};

			int EOT_flag = 0;
			int i;

			while (!EOT_flag) {
				if (poll(descriptors, 2, 0) > 0) {

					short revents_stdin = descriptors[0].revents;
					short revents_shell = descriptors[1].revents;

					/* check that stdin has pending input */
					if (revents_stdin == POLLIN) {
						char input[256];
						int num = read(STDIN_FILENO, &input, 256);
						for (i = 0; i < num; i++) {
							if (input[i] == ETX) {
								kill(pid, SIGINT); 

						    } else if (input[i] == EOT) {
						    	EOT_flag = 1;

						    } else if (input[i] == CR || input[i] == LF) { 
						    	char lf = LF;
								write(STDOUT_FILENO, crlf, 2);
								write(pipe_ptoc[1], &lf, 1);
							
							} else { 
								write(STDOUT_FILENO, (input + i), 1);
								write(pipe_ptoc[1], (input + i), 1);
							}
						}

					} else if (revents_stdin == POLLERR) {
						fprintf(stderr, "Error with poll with STDIN.\n");
						exit(1);
					}

					/* check that the shell has pending input */
					if (revents_shell == POLLIN) {
						/* "Upon receiving EOF or polling error from
						 * the shell, we know that there will be no
						 * more output coming from the shell."
						 */
						char input[256];
						int num = read(pipe_ctop[0], &input, 256); 
						int count = 0;
						int j;
						for (i = 0, j = 0; i < num; i++) {
							if (input[i] == EOT) { //EOF from shell
								EOT_flag = 1;
							} else if (input[i] == LF) {
								write(STDOUT_FILENO, (input + j), count);
								write(STDOUT_FILENO, crlf, 2);
								j += count + 1;
								count = 0;
								continue;
							}
							count++;
						}
						write(STDOUT_FILENO, (input+j), count);		

					} else if (revents_shell & POLLERR || revents_shell & POLLHUP) { //polling error
						EOT_flag = 1;
					} 
				} 
			}

			close(pipe_ctop[0]);
			close(pipe_ptoc[1]);
		    	shell_exit_status();
			exit(0);

		} else { //fork failed
			fprintf(stderr, "Could not fork process.\n");
			exit(1);
		}
	}

	/* 
	 * NO SHELL OPTION
	 */
	set_mode();

	//read input from the keyboard into a buffer
	char input;

	while (read(STDIN_FILENO, &input, 10) > 0 && input != EOT) {
		//carriage return or line feed should become <cr><lf>
		if (input == CR || input == LF) { 
			input = CR;
			write(STDOUT_FILENO, &input, 1);
			input = LF;
			write(STDOUT_FILENO, &input, 1);
		} else {
			write(STDOUT_FILENO, &input, 1);
		}
	}

	exit(0);
}
