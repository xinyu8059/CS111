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
#include "SortedList.h"

SortedList_t head;
SortedListElement_t* elements;

unsigned long threads;
long iterations;
int lock;
char opt_sync;
pthread_mutex_t m_lock;

void handle_segfault() {
	fprintf(stderr, "Segmentation fault. Exiting.");
	exit(2);
}

void* thread_function(void *stuff) {
	SortedListElement_t* array = stuff;
    if (opt_sync == 'm') {
        pthread_mutex_lock(&m_lock);
    } else if (opt_sync == 's') {
        while (__sync_lock_test_and_set(&lock, 1));
    }

	long i;
	for (i = 0; i < iterations; i++) {
		SortedList_insert(&head, (SortedListElement_t *) (array+i));
	}
	long list_length = SortedList_length(&head);

	if (list_length < iterations) {
		fprintf(stderr, "Not all items inserted in list.\n");
		exit(2);
	}

	char *curr_key = malloc(sizeof(char)*256);

	SortedListElement_t *ptr;

	for (i = 0; i < iterations; i++) {
		strcpy(curr_key, (array+i)->key);
        ptr = SortedList_lookup(&head, curr_key);
        if (ptr == NULL) {
            fprintf(stderr, "Failure to look up element");
            exit(2);
        }

		int n = SortedList_delete(ptr);
        if (n != 0) {
            fprintf(stderr, "Failure to delete element");
            exit(2);
        }
	}

    // release mutex or lock based on sync options
    if (opt_sync == 'm') {
        // unlock the mutex
        pthread_mutex_unlock(&m_lock);
    }
    else if (opt_sync == 's') {
        // release lock
        __sync_lock_release(&lock);
    }
    return NULL;
}

int main(int argc, char* argv[]) {

	signal(SIGSEGV, handle_segfault);
	threads = 1;
	iterations = 1;
	opt_yield = 0;
	opt_sync = 0;

	struct option options[] = {
		{"threads", required_argument, NULL, 't'},
		{"iterations", required_argument, NULL, 'i'},
		{"yield", required_argument, NULL, 'y'},
		{"sync", required_argument, NULL, 's'},
		{0, 0, 0, 0}
	};

	int opt;
	unsigned long i;
	unsigned long j;
	while ((opt = getopt_long(argc, argv, "", options, NULL)) != -1) {
		switch (opt) {
			case 't':
				threads = atoi(optarg);
				break;
			case 'i':
				iterations = atoi(optarg);
				break;
			case 'y':
				for (i = 0; i < strlen(optarg); i++) {
					if (optarg[i] == 'i') {
						opt_yield |= INSERT_YIELD;
					} else if (optarg[i] == 'd') {
						opt_yield |= DELETE_YIELD;
					} else if (optarg[i] == 'l') {
						opt_yield |= LOOKUP_YIELD;
					}
				}
				break;
			case 's':
				opt_sync = optarg[0];
				break;
			default:
				fprintf(stderr, "Bad arguments\n");
				exit(1);
		}
	}

	unsigned long num_elements = threads * iterations;
	elements = malloc(sizeof(SortedListElement_t) * num_elements);

	if (elements == NULL) {
		fprintf(stderr, "Could not allocate memory for elements.\n");
		exit(1);
	}

	char** keys = malloc(iterations * threads * sizeof(char*));
	if (keys == NULL) {
		fprintf(stderr, "Could not allocate memory for keys\n");
		exit(1);
	}
	for (i = 0; i < num_elements; i++) {
		keys[i] = malloc(sizeof(char) * 256);
		if (keys[i] == NULL) {
			fprintf(stderr, "Could not allocate memory for keys.\n");
			exit(1);
		}
		for (j = 0; j < 255; j++) {
			keys[i][j] = rand() % 94 + 33;
		}
		keys[i][255] = '\0';
		(elements + i)->key = keys[i];
	}

	if (opt_sync == 'm') {
    	if (pthread_mutex_init(&m_lock, NULL) != 0) {
      		fprintf(stderr, "Could not create mutex\n");
      		exit(1);
    	}
  	}

    pthread_t *thread_ids = malloc(sizeof(pthread_t) * threads);
    if (thread_ids == NULL) {
    	fprintf(stderr, "Could not allocate memory for threads\n");
    	exit(1);
    }

  	struct timespec begin, end;
  	clock_gettime(CLOCK_MONOTONIC, &begin);

  	for (i = 0; i < threads; i++) {
  		if (pthread_create(&thread_ids[i], NULL, &thread_function, (void *) (elements + iterations * i)) != 0) {
  			fprintf(stderr, "Could not create threads\n");
  			exit(1);
  		}
  	}

  	for (i = 0; i < threads; i++) {
  		pthread_join(thread_ids[i], NULL);
  	}

  	clock_gettime(CLOCK_MONOTONIC, &end);

  	if (SortedList_length(&head) != 0) {
  		fprintf(stderr, "Length of list not 0 at end.\n");
  		exit(2);
  	}

  	fprintf(stdout, "list");

	switch(opt_yield) {
	    case 0:
	        fprintf(stdout, "-none");
	        break;
	    case 1:
	        fprintf(stdout, "-i");
	        break;
	    case 2:
	        fprintf(stdout, "-d");
	        break;
	    case 3:
	        fprintf(stdout, "-id");
	        break;
	    case 4:
	        fprintf(stdout, "-l");
	        break;
	    case 5:
	        fprintf(stdout, "-il");
	        break;
	    case 6:
	        fprintf(stdout, "-dl");
	        break;
	    case 7:
	        fprintf(stdout, "-idl");
	        break;
	    default:
	        break;
	}

	  
	switch(opt_sync) {
	    case 0:
	        fprintf(stdout, "-none");
	        break;
	    case 's':
	        fprintf(stdout, "-s");
	        break;
	    case 'm':
	        fprintf(stdout, "-m");
	        break;
	    default:
	        break;
	  }
	  
	long operations = threads * iterations * 3;
	long run_time = 1000000000L * (end.tv_sec - begin.tv_sec) + end.tv_nsec - begin.tv_nsec;
	long time_per_op = run_time / operations;
	long num_lists = 1;

	  // print rest of data
	fprintf(stdout, ",%ld,%ld,%ld,%ld,%ld,%ld\n", threads, iterations, num_lists, operations, run_time, time_per_op);

	free(elements);
	free(thread_ids);

	if (opt_sync == 'm') {
		pthread_mutex_destroy(&m_lock);
	}

	exit(0);

}