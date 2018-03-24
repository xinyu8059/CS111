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
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <sched.h>

long long counter;
long threads;
long iterations;

int opt_yield;
char opt_sync;
int lock = 0;
pthread_mutex_t m_lock; 

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield) {
    		sched_yield();
	}
   	*pointer = sum;
}

void add_m(long long *pointer, long long value) {
 	pthread_mutex_lock(&m_lock);
  	long long sum = *pointer + value;
  	if (opt_yield) {
   		sched_yield();
	}
 	*pointer = sum;
  	pthread_mutex_unlock(&m_lock);
}

void add_s(long long *pointer, long long value) {
	while (__sync_lock_test_and_set(&lock, 1));
	long long sum = *pointer + value;
	if (opt_yield) {
    		sched_yield();
    	}
  	*pointer = sum;
 	 __sync_lock_release(&lock);
}

void add_c(long long *pointer, long long value) {
	long long old;
	do {
		old = counter;
	    	if (opt_yield) {
	     		sched_yield();
	    	}
  	} while (__sync_val_compare_and_swap(pointer, old, old+value) != old);
}

void* thread_add(void* arg) {
	long i;	
	for (i = 0; i < iterations; i++) {
		if (opt_sync == 'm') {
			add_m(&counter, 1);
			add_m(&counter, -1);
		} else if (opt_sync == 's') {
			add_s(&counter, 1);
			add_s(&counter, -1);
		} else if (opt_sync == 'c') {
			add_c(&counter, 1);
			add_c(&counter, -1);
		} else {
			add(&counter, 1);
			add(&counter, -1);
		}
	}
	return arg;
}

int main(int argc, char* argv[]) {
	/* 
	 * takes a parameter for the number of parallel threads (default 1)
	 * takes a parameter for the number of iterations (default 1)
	 */
	threads = 1;
	iterations = 1;
	opt_yield = 0;
	opt_sync = 0;

	struct option options[] = {
		{"threads", required_argument, NULL, 't'},
		{"iterations", required_argument, NULL, 'i'},
		{"yield", no_argument, NULL, 'y'},
		{"sync", required_argument, NULL, 's'},
		{0, 0, 0, 0}
	};

	int opt;
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 't':
				threads = atoi(optarg);
				break;
			case 'i':
				iterations = atoi(optarg);
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				opt_sync = optarg[0];
				break;
			default:
				fprintf(stderr, "Bad arguments\n");
				exit(1);
		}
	}

	/* initializes a (long long) counter to zero */
	counter = 0;

	/* notes the (high resolution) starting time for the run */
	struct timespec begin, end;
	clock_gettime(CLOCK_MONOTONIC, &begin);

	pthread_t *thread_ids = malloc(threads * sizeof(pthread_t));
	if (thread_ids == NULL) {
		fprintf(stderr, "Could not allocate memory fot thread ids\n");
		exit(1);
	}

	if (opt_sync == 'm') {
		if (pthread_mutex_init(&m_lock, NULL) != 0){
			fprintf(stderr, "Could not create mutex.\n");
			exit(1);
		}	
	}

	/* create threads to add 1 to counter and add -1 to counter */
	int i;
	for (i = 0; i < threads; i++) {
		if (pthread_create(&thread_ids[i], NULL, &thread_add, NULL) != 0) {
			fprintf(stderr, "Could not create threads\n");
			exit(1);
		}
	}

	for (i = 0; i < threads; i++) {
		pthread_join(thread_ids[i], NULL);
	}

	/* stop the timer */
	clock_gettime(CLOCK_MONOTONIC, &end);

	/* data to csv strings */
	long operations = threads * iterations * 2;
	long run_time = 1000000000L * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;
	long time_per_op = run_time / operations;

	/* print the data */
	if (opt_yield == 1 && opt_sync == 'm')
	    fprintf(stdout, "add-yield-m,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else if (opt_yield == 1 && opt_sync == 's')
	    fprintf(stdout, "add-yield-s,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else if (opt_yield == 1 && opt_sync == 'c')
	    fprintf(stdout, "add-yield-c,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else if (opt_yield == 1 && opt_sync == 0)
	    fprintf(stdout, "add-yield-none,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else if (opt_yield == 0 && opt_sync == 'm')
	    fprintf(stdout, "add-m,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else if (opt_yield == 0 && opt_sync == 's')
	    fprintf(stdout, "add-s,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else if (opt_yield == 0 && opt_sync == 'c')
	    fprintf(stdout, "add-c,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);
	  else
	    fprintf(stdout, "add-none,%ld,%ld,%ld,%ld,%ld,%lld\n", threads, iterations, operations, run_time, time_per_op, counter);

	if (opt_sync == 'm')
	    pthread_mutex_destroy(&m_lock);

	free(thread_ids);
	exit(0);
}
