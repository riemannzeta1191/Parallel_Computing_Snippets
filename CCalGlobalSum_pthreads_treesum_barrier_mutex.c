#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#include <math.h>
#include <semaphore.h>


long sum = 0;
long times = 0, block_size = 0;
int thread_count = 0;

long * thread_sum = NULL;

int *counts; 
pthread_mutex_t  count_mutex; 
pthread_mutex_t  barrier_mutex; 

void* computingGlobalSum_pthreads(void * threadID);

int main(int argc, char * argv[]) {
	thread_count = strtol(argv[1], NULL, 10);
	times = strtol(argv[2], NULL, 10);

	if ((thread_count <= 0) || (times <= 0)){
		printf("Usage: ./computingPi_pthreads  #(threads) N_times\n");
		printf("#(threads) > 0 and N_times > 0\n");
		exit(1);
	}

	thread_sum = malloc(thread_count * sizeof(long));

	struct timeval begin, end;
	gettimeofday(&begin, NULL);

	block_size = times % thread_count == 0 ? times/thread_count : times/thread_count + 1;
	pthread_t * thread_handlers = malloc(thread_count * sizeof(pthread_t));

]	int levels = ceil(log(thread_count) / log(2)) + 1;

	counts = malloc(levels * sizeof(int));
	for (int i = 0; i < levels ; i++ )
		counts[i] = 0;

	
	for (int thread = 0; thread < thread_count; thread++){
		long threadID = thread;
		if (pthread_create(&thread_handlers[thread], NULL, computingGlobalSum_pthreads, (void *)threadID)){
			printf("Error creating a thread (id: %d)\n", thread);
			exit(1);
		}
	}


	for (int thread = 0; thread < thread_count; thread++){
		if (pthread_join(thread_handlers[thread], NULL)){
			printf("Error joining a thread (id: %d)\n", thread);
			exit(1);
		}
	}
	gettimeofday(&end, NULL);

	long seconds = end.tv_sec - begin.tv_sec;
	long micros = (seconds * 1000000) + end.tv_usec - begin.tv_usec;

	printf("Thread_count=%d;N=%ld;Sum=%ld;Seconds=%ld;Micros=%ld\n", thread_count, times, thread_sum[0], seconds, micros);

	free(thread_handlers);
	free(counts);

	
	pthread_mutex_destroy(&(barrier_mutex));

	pthread_mutex_destroy(&(count_mutex));

	return EXIT_SUCCESS;
}

void* computingGlobalSum_pthreads(void * threadID){
	int my_rank = (int)((long) threadID);

	thread_sum[my_rank] = 0;
	long my_first_id = block_size * my_rank;
	long my_last_id = block_size * (my_rank + 1) > times ? times : block_size * (my_rank + 1);

	for (long i = my_first_id; i < my_last_id; i++){
		thread_sum[my_rank] += i;
	}
	printf("Local sum of thread %d from %ld to %ld is equal to %ld\n", my_rank, my_first_id, my_last_id, thread_sum[my_rank]);

	int iLevel = 0;
	for (int stride = 1; stride < thread_count; stride *= 2){
		if (my_rank % (iLevel == 0 ? 1 : 2 * iLevel) == 0){
			int active_threads = iLevel == 0 ? thread_count : floor(thread_count / (2 * iLevel));

			pthread_mutex_lock(&count_mutex);
			if (counts[iLevel] == active_threads - 1) {
				counts[iLevel] = 0;
				pthread_mutex_unlock(&count_mutex);
				for (int j = 0; j < active_threads-1; j++)
				   pthread_mutex_unlock(&barrier_mutex);
			} else {
				counts[iLevel]++;
				pthread_mutex_unlock(&count_mutex);
				pthread_mutex_lock(&barrier_mutex);
				printf("In level %d, %d is released from waiting...\n", iLevel, my_rank);
				fflush(stdout);
			}


			if (my_rank == 0) {
				printf("%d threads completed barrier %d \n", active_threads, iLevel);
				fflush(stdout);
			}

			if (my_rank % (2 * stride) == 0){
				thread_sum[my_rank] += thread_sum[my_rank + stride];
				printf("thread sum is %ld\n",thread_sum[my_rank] );
			}


		}
		iLevel++;
	}

	return NULL;
}
