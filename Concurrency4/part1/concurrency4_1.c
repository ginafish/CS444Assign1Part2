#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>

void sig_catch(int sig);
void process(void *);
void print_format(int);
struct buffer {
    int number;
    sem_t mutex;    
};

struct buffer buf;
int counter;
int global_index=1;


void process (void *buffer){
    int zero = 0;
	int local_index = global_index++;
    if(counter > 0)
        zero = 1;
    while(!zero){
        printf("Process %d Waiting\n",local_index);
        sleep(5);
        if(counter == 3){   
            printf("Process %d Done waiting\n",local_index);
            zero = 1;
        }
    }
    sem_wait(&buf.mutex);
    counter--;
    printf("\tProcess %d Working\n", local_index);
    sleep(10);
    printf("\tProcess %d Done working\n", local_index);
    sem_post(&buf.mutex);
    counter++;
}


int main(int argc, char **argv) {

    void *proc_func = process;
    struct sigaction sig;
	int i=0;
	if (argc != 2){
		printf("Usage: concurrency4_1 <NUM_THREADS>\n");
		exit(1);
	}
	int num_threads = atoi(argv[1]);
    pthread_t* thread_array = malloc(sizeof(pthread_t)*num_threads);
    sem_init(&buf.mutex, 0, 3);
    counter = 3;   
	for (i=0; i<num_threads; i++)
		 pthread_create(&thread_array[i], NULL, proc_func, NULL);
    while(1);
}