//NAME: Khoa Quach
//EMAIL: khoaquachschool@gmail.com
//ID: 105123806
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
// Project 2A
#include <pthread.h>
#include <time.h>
#include <sched.h>
#include "SortedList.h"

// Global Vars
long numthreads = 1; // default 1
long iterations = 1; /// default 1

char opt_sync = 0;

/*--locks-*/
//pthread_mutex_t mutexlock;
//static int spinlock;
/*------*/

/*-- lock flags---*/
int mutexflag = 0;
int spinlockflag=0;

/*----Yield and sync flags--*/
int opt_yield = 0; // set to appropiate YIELD version: INSERT, DELETE, or LOOKUP
int sync_flag = 0; // sync flag => if 1, set the m,s approapiately using opt_sync and set the lock type flags
/*----------*/

/*---Part 2, List variables ---*/
//unsigned long num_elements = 0; // for initialization
int num_elements = 0;
//SortedList_t* list;
//SortedListElement_t* elements;

/*----Project 2B Vars---------*/
pthread_mutex_t* mutexs;
static int* spinlocks;
int numlist = 1;
long long wait_time = 0;
SortedList_t* lists;
SortedListElement_t* elements;
int* hash_key;

/*----------------------Error functions exit(1) and exit(2)--------------*/
void print_error(const char* msg){
	fprintf(stderr, "%s , error number: %d, strerror: %s\n ",msg, errno, strerror(errno));
    exit(1);
}

void print_error2(const char* msg){
	fprintf(stderr, "%s , error number: %d, strerror: %s\n ",msg, errno, strerror(errno));
    exit(2);
}


/*----------------------Custom Signal Handler---------------------------*/
void signal_handler(int sig)
{
	if (sig == SIGSEGV){
		print_error2("Error: Segmentation fault");
	}
}


/*-----------------------Hash Function----------------------------------*/
int hasher(const char *key){
	return key[0]%numlist;
}

/*----------------------Helper functions for Thread function------------*/


/*----------------------Thread Function---------------------------------*/
/*
each thread:
starts with a set of pre-allocated and initialized elements (--iterations=#)
inserts them all into a (single shared-by-all-threads) list
gets the list length
looks up and deletes each of the keys it had previously inserted
exits to re-join the parent thread

TA example code:
startIndex = threadNum * iteration;
for (i = startIndex; i < startIndex + iteration; i++) SortedList_insert(listhead, pool[i]); //insert element

Looks ugly so you can alter the same logic to:
int startIndex = *((int*)thread_id);
for (i = startIndex; i < num_elments; i = i + numthreads);
since num_elements = iterations*numthreads;
*/
void* list_function(void* thread_id){
 	struct timespec start;
	struct timespec end;
	int startIndex = *((int*)thread_id);
	//inserts them all into a (single shared-by-all-threads)
	for (int i = startIndex; i < num_elements; i = i + numthreads){
		if(opt_sync == 'm' && mutexflag){
			if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
				print_error("Error: clock gettime error for start");
			}

			pthread_mutex_lock(&mutexs[hash_key[i]]);

			if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
				print_error("Error: clock gettime error for end");
			}

			wait_time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			SortedList_insert(&lists[hash_key[i]],elements + i);
			pthread_mutex_unlock(&mutexs[hash_key[i]]);
		} 
		else if (opt_sync == 's' && spinlockflag){
			if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
				print_error("Error: clock gettime error for start");
			}
			while(__sync_lock_test_and_set(&spinlocks[hash_key[i]],1));


			if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
				print_error("Error: clock gettime error for end");
			}
			wait_time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			SortedList_insert(&lists[hash_key[i]],elements + i);
			__sync_lock_release(&spinlocks[hash_key[i]]);
		}
		else{
			SortedList_insert(&lists[hash_key[i]],elements + i);
		}
	}
	//gets the length
	int size;
	if (opt_sync == 'm' && mutexflag){
		for (int j = 0; j < numlist ; j++){
			if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
				print_error("Error: clock gettime error for start");
			}

			pthread_mutex_lock(&mutexs[j]);

			if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
				print_error("Error: clock gettime error for end");
			}

			wait_time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			
			int length = SortedList_length(&lists[j]);
			if (length == -1)
			{
				print_error2("Error: Corrupted &lists[hash_key[i]]");
			} else {
				size += length;
			}
			pthread_mutex_unlock(&mutexs[j]);
		}
	} 
	else if (opt_sync == 's' && spinlockflag){
		for (int k = 0; k < numlist ; k++){
			if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
				print_error("Error: clock gettime error for start");
			}

			while(__sync_lock_test_and_set(&spinlocks[k],1));

			if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
				print_error("Error: clock gettime error for end");
			}

			wait_time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			int length = SortedList_length(&lists[k]);
			if (length == -1)
			{
				print_error2("Error: Corrupted &lists[hash_key[i]]");
			} else {
				size += length;
			}
			__sync_lock_release(&spinlocks[k]);
		}

	}
	else {
		for (int z = 0; z < numlist ; z++){
			int length = SortedList_length(&lists[z]);
			if (length == -1)
			{
				print_error2("Error: Corrupted &lists[hash_key[i]]");
			} else {
				size += length;
			}
		}
	}
	// looks up and deletes each of the keys it had previously inserted
	SortedListElement_t *find;
	for (int i = startIndex; i < num_elements; i = i + numthreads)
	{
		if(opt_sync == 'm' && mutexflag){
			if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
				print_error("Error: clock gettime error for start");
			}
			
			pthread_mutex_lock(&mutexs[hash_key[i]]);
			
			if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
				print_error("Error: clock gettime error for end");
			}
			wait_time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			
			find = SortedList_lookup(&lists[hash_key[i]],elements[i].key);
			if (find == NULL){
				print_error2("Error: Lookup but Corrupted list");
			}
			if (SortedList_delete(find)){
				print_error2("Error: Delete but corrupted list");
			}
			pthread_mutex_unlock(&mutexs[hash_key[i]]);
		} 
		else if (opt_sync == 's' && spinlockflag){
			if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
				print_error("Error: clock gettime error for start");
			}
			while(__sync_lock_test_and_set(&spinlocks[hash_key[i]],1));

			if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
				print_error("Error: clock gettime error for end");
			}

			wait_time += 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
			
			find = SortedList_lookup(&lists[hash_key[i]],elements[i].key);
			if (find == NULL){
				print_error2("Error: Lookup but Corrupted &lists[hash_key[i]]");
			}
			if (SortedList_delete(find)){
				print_error2("Error: Delete but corrupted &lists[hash_key[i]]");
			}
			__sync_lock_release(&spinlocks[hash_key[i]]);
		}
		else{
			find = SortedList_lookup(&lists[hash_key[i]],elements[i].key);
			if (find == NULL){
				print_error2("Error: Lookup but Corrupted &lists[hash_key[i]]");
			}
			if (SortedList_delete(find)){
				print_error2("Error: Delete but corrupted &lists[hash_key[i]]");
			}
		}
	}
	return NULL;
}

int main(int argc, char **argv){
	int choice;

	/*-----------------------Argument Setup----------------------------*/
	static struct option long_options[] = 
	{
		{"threads", 1, 0, 't'}, 
		{"iterations",1,0,'i'},
		{"yield",1,0,'y'}, // required argument for yield
		{"sync",1,0,'s'},
		{"lists",1,0,'l'},
		{0,0,0,0}
	};
	while((choice = getopt_long(argc, argv, "", long_options,NULL))!= -1)
	{
		switch(choice){
			case 't':
				numthreads = atoi(optarg);
				break;
			case 'i':
				iterations = atoi(optarg);
				break;
			case 'y':
				for (unsigned int i = 0; i < strlen(optarg); i++){
					if(optarg[i] =='i'){
						opt_yield = opt_yield|INSERT_YIELD;
					}
					else if (optarg[i] =='d'){
						opt_yield = opt_yield|DELETE_YIELD;
					}
					else if (optarg[i] =='l'){
						opt_yield = opt_yield|LOOKUP_YIELD;
					}
					else{
						print_error("Error: invalid argument for --yield option. Please use only idl");
					}

				}
				break;
			case 's':
				opt_sync = *optarg;
				sync_flag = 1; // to do , m , s,
				if (sync_flag){
					if (opt_sync == 'm'){
					//	if(pthread_mutex_init(&mutexs, NULL) < 0){
					//		print_error("Error: mutex initialization");
					//	}
						mutexflag=1;
						break;
					}
					else if (opt_sync == 's') {
						spinlockflag = 1;
						break;
					}
					else {
						fprintf(stderr, "unrecognized argument! Correct usage: ./lab2_list --iterations=# --threads=# --yield string --sync=option\n");
						exit(1);
						break;
					}
				}
				break;
			case 'l':
				numlist = atoi(optarg);
				break;
			case '?':
				fprintf(stderr, "unrecognized argument! Correct usage: ./lab2_list --iterations=# --threads=# --yield string --sync=option\n");
				exit(1);
			default:
				fprintf(stderr, "unrecognized argument! Correct usage: ./lab2_list--iterations=# --threads=# --yield string --sync=option\n");
				exit(1);
		}// switch
	}// while

	// Segmentation fault handler
	signal(SIGSEGV,signal_handler);

	if (numthreads < 1){
        print_error("Error: number of threads has to be >= 1");
    }

    if (iterations < 1){
        print_error("Error: number of iterations has to be >=1");
    }

    if (numlist < 1){
    	print_error("Error: number of lists has to be >=1");
    }

 	/*
	initializes an empty list.
	creates and initializes (with random keys) the required number (threads x iterations) of list elements.
	Note that we do this before creating the threads so that this time is not included in our start-to-finish measurement.
	Similarly, if you free elements at the end of the test, do this after collecting the test execution times.
 	*/

 	/*-----------------------Initilization empty list----------------------------*/
	// Initialization of the new list !!!!! :) , Don't forget this next time :o
	lists = (SortedList_t*)malloc(sizeof(SortedList_t) * numlist);
	for (int i = 0; i < numlist; i++){
		lists[i].prev = &lists[i];
		lists[i].next = &lists[i];
		lists[i].key = NULL;
	}

	/*-----------------------------Lock setup------------------------------------*/
	if (mutexflag) {
		mutexs = malloc(sizeof(pthread_mutex_t)*numlist);
		for (int i = 0; i < numlist; i++){
			if(pthread_mutex_init(&mutexs[i], NULL) < 0){
				print_error("Error: mutex initialization");
			}
		}
	}
		
	if (spinlockflag){
		spinlocks = malloc(sizeof(int)*numlist);
		for (int i = 0; i < numlist; i++){
			spinlocks[i] = 0;
		}
	}


	/*------------------------------Nodes/Elements setup-------------------------*/
	num_elements = iterations*numthreads;
	elements = malloc(sizeof(SortedListElement_t)*num_elements);  // allocate memory for number of nodes or elements
	if (elements == NULL){
		print_error("Error: element memory allocation error");
	}

	/*--------------------------Filling with random keys-------------------------*/
	// TA TENGYU LIU:
	// A simple way is to generate a sequence of random integers between 0 and 25, and then convert the integers to English letter characters.
    for(int i=0; i<num_elements; i++){
		char* keystring = (char*)malloc(3*sizeof(char));
		for(int i = 0; i < 1; i++){
			keystring[i] = 'a'+rand()%26;
		}
		keystring[2] = '\0'; // suggested by one of the TAs
		elements[i].key = keystring;
    }

    /*----------------------------Hashing-----------------------------------------*/
    hash_key = malloc(sizeof(int) * numthreads * iterations);
    for (int i =0; i < num_elements; i++){
    	hash_key[i] = hasher(elements[i].key);
    }

	/*-------------------------Timer Setup and Thread Setup----------------------*/
 	struct timespec start;
	struct timespec end;

	// TIMER START
	if(clock_gettime(CLOCK_MONOTONIC, &start) == -1){
		print_error("Error: clock gettime error for start");
	}
	/*
	   The pthread_create() function starts a new thread in the calling
       process.  The new thread starts execution by invoking
       start_routine(); arg is passed as the sole argument of
       start_routine().
	*/
	/*
	https://stackoverflow.com/questions/10575544/difference-between-array-type-and-array-allocated-with-malloc
	*/
	//pthread_t threads[numthreads];
	//int thread_id[numthreads];
	pthread_t *threads = malloc(numthreads *sizeof(pthread_t));
	int *thread_id = malloc(numthreads* sizeof(int));
	// Create n_job threads to run in parallel and waits for every thread to finishL
	for(int i =0; i < numthreads; i++){
		thread_id[i] = i;
		if(pthread_create(&threads[i],NULL,list_function,&thread_id[i]) != 0){
			print_error("Error: creating threads");
		}
	}
	// Join step:
	for(int i = 0; i < numthreads;i++){
		if (pthread_join(threads[i],NULL) != 0){
			print_error("Error: joining threads");
		}
	}
	// TIME END
	if(clock_gettime(CLOCK_MONOTONIC, &end) == -1){
		print_error("Error: clock gettime error for end");
	}
	/*-------------------------Timer Setup and Thread Setup----------------------*/

	/*-------------------------------Check Size----------------------------------*/
	if(SortedList_length(lists) !=0){
		print_error2("Length of list does not equal 0 at the end");
	}
	/*---------------------------------------------------------------------------*/

	/*------------------------------Time Variables-------------------------------*/
	long num_operations = numthreads * iterations * 3;
	long long time_elapsed = 1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	long time_per_operation = time_elapsed / num_operations;
	//int num_list = 1;
	/*---------------------------------------------------------------------------*/


	/*------------------------------- Printing Logic ----------------------------*/
	fprintf(stdout, "list");
	char *y_string = "";
	/// yield type -idl,-id,il,-i,-d,-l, or -none
	if (opt_yield&INSERT_YIELD && opt_yield&DELETE_YIELD && opt_yield&LOOKUP_YIELD){
		y_string = "-idl";
	}
	else if (opt_yield&INSERT_YIELD && opt_yield&DELETE_YIELD){
		y_string = "-id";
	}
	else if(opt_yield&DELETE_YIELD && opt_yield&LOOKUP_YIELD){
		y_string = "-dl";
	}
	else if (opt_yield&INSERT_YIELD && opt_yield&LOOKUP_YIELD){
		y_string = "-il";
	}
	else if (opt_yield&INSERT_YIELD){
		y_string = "-i";
	}
	else if (opt_yield&DELETE_YIELD){
		y_string = "-d";
	}
	else if (opt_yield&LOOKUP_YIELD){
		y_string = "-l";
	}
	else{
		y_string ="-none";
	}
	char *s_string = "";
	// synce type: none, m, or s
	if (mutexflag) {
		s_string = "-m";
	} 
	else if (spinlockflag){
		s_string = "-s";
	}
	else {
		s_string = "-none";
	}
	fprintf(stdout,"%s%s,%ld,%ld,%d,%ld,%lld,%ld,%lld\n",y_string, s_string,numthreads,iterations,numlist,num_operations, time_elapsed,time_per_operation, wait_time/num_operations);
	/*
	prints to stdout a comma-separated-value (CSV) record including:
	the name of the test, which is of the form: list-yieldopts-syncopts: where
	yieldopts = {none, i,d,l,id,il,dl,idl}
	syncopts = {none,s,m}
	the number of threads (from --threads=)
	the number of iterations (from --iterations=)
	the number of lists (always 1 in this project)
	the total number of operations performed: threads x iterations x 3 (insert + lookup + delete)
	the total run time (in nanoseconds) for all threads
	the average time per operation (in nanoseconds).
	*/
	//SortedList_t *free_list;
  	free(lists);
  	for (int i =0; i < num_elements; i++){
  		free((void*)elements[i].key);
  	}
  	free(threads);
  	free(thread_id);
  	free(hash_key);
  	if(mutexflag){
  		free(mutexs);
  	}
  	if (spinlockflag){
  		free(spinlocks);
  	}
  	if(mutexflag){
  		pthread_mutex_destroy(mutexs);
  	}
	exit(0);
}