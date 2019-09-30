

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

long sum = 0;
long times = 0;
int block_size = 0;
int thread_count = 0;
pthread_mutex_t  sum_mutex;  


void* computingGlobalSum_pthreads(void * threadID);

int main(int argc, char * argv[]) {

	thread_count = strtol(argv[1], NULL, 10);
	times = strtol(argv[2], NULL, 10);

	if ((thread_count <= 0) || (times <= 0)){
		printf("Usage: ./computingPi_pthreads  #(threads) N_times\n");
		printf("#(threads) > 0 and N_times > 0\n");
		exit(1);
	}

	block_size = times % thread_count == 0 ? times/thread_count : times/thread_count + 1;
	printf("Block size is %d\n", block_size);
	pthread_t * thread_handlers = malloc(thread_count * sizeof(pthread_t));

	struct timeval begin, end;
	gettimeofday(&begin, NULL);
    
    pthread_mutex_init(&sum_mutex, NULL);  

	for (int thread = 0; thread < thread_count; thread++){
		if (pthread_create(&thread_handlers[thread], NULL, computingGlobalSum_pthreads, (void *)thread)){
			printf("Error creating a thread (id: %ld)\n", thread);
			exit(1);
		}
	}

	printf("%d threads are working on their local calculation and synchronizing with master thread\n", thread_count);

	for (int thread = 0; thread < thread_count; thread++){
		if (pthread_join(thread_handlers[thread], NULL)){
			printf("Error joining a thread (id: %d)\n", thread);
			exit(1);
		}
	}
	// pthread_mutex_destroy (&sum_mutex );
	printf("Global sum from 0 to %ld is equal to %ld\n", times-1, sum);
	gettimeofday(&end, NULL);

	long seconds = end.tv_sec - begin.tv_sec;
	long micros = (seconds * 1000000) + end.tv_usec - begin.tv_usec;

	printf("Thread_count=%d;N=%ld;Sum=%ld;Seconds=%ld;Micros=%ld\n", thread_count, times, sum, seconds, micros);
	free(thread_handlers);
	return EXIT_SUCCESS;
}

void* computingGlobalSum_pthreads(void * threadID){
	int my_rank = (int) threadID;

	long  local_sum = 0;
	long my_first_id = block_size * my_rank;
		printf("my_first_id is %ld\n",my_first_id );

	long my_last_id = block_size * (my_rank + 1) > times ? times : block_size * (my_rank + 1);

	double factor;

	for (long i = my_first_id; i < my_last_id; i++){
		local_sum += i;
	   }
	   printf("Local sum of thread %d from %ld to %ld is equal to %ld\n", my_rank, my_first_id, my_last_id, local_sum);	

	
		pthread_mutex_lock(&sum_mutex);
		sum = sum + local_sum;
		pthread_mutex_unlock(&sum_mutex);
		printf("Rank is %d and Sum is %ld\n" ,my_rank, sum);
	


	return NULL;
}
