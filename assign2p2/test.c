/* 	CS 444
*	Group 13-05
*	Brandon Thenell, Gina Phipps, Nawwaf Almutairi
*	Concurrency 2
*	5/5/2017
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "mt19937ar.c"
struct philosopher{
	char* name;
	int left;
	int right;
};
static pthread_mutex_t forks[5] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};
void eat(void);
void think(void);
int gen_number(int high, int low);
pthread_mutex_t fork1, fork2, fork3, fork4, fork5;
int fork_pairs[5];
int global_index=0;
struct philosopher philosophers[5];

int gen_number(int high, int low) {
    int num = 0;
    num = abs((int)genrand_int32());
    num %= (high - low);
    if (num < low)
        num = low;
    return num;
}

void eat(void){sleep(gen_number(9, 2));}

void think(void){sleep(gen_number(20, 1));}

void process(void){
	int left_pair,index,i;
	index=global_index++;
	char spacer[5] = "\0\0\0\0\0";
	for (i=0; i < index; i++)
		spacer[i]='\t';
    while(1){
        printf("%s%s is thinking\n",spacer,philosophers[index].name);
        think();
        printf("%s%s is done thinking\n",spacer,philosophers[index].name);
        while(fork_pairs[index] == 1){
            printf("%s%s is waiting for fork %d\n",spacer,philosophers[index].name,index);
            sleep(5);
        }
        while(fork_pairs[(index + 1) % 5] == 1){
            printf("%s%s is waiting for fork %d\n",spacer,philosophers[index].name,index+1);
            sleep(5);
        }
		// locks the forks
        pthread_mutex_lock(&forks[philosophers[index].left]);
        pthread_mutex_lock(&forks[philosophers[index].right]);
        fork_pairs[index] = 1;
		fork_pairs[(index+1)%5] = 1;
        // start and finish eating
        printf("%s%s is eating\n",spacer,philosophers[index].name);
        eat();
		printf("%s%s is done eating\n",spacer,philosophers[index].name);
        // puts forks down
        pthread_mutex_unlock(&forks[philosophers[index].left]);
        pthread_mutex_unlock(&forks[philosophers[index].right]);
        fork_pairs[index] = 0;
        fork_pairs[(index+1)%5] = 0;
		
        printf("%s%s has put forks %d and %d down\n",spacer,philosophers[index].name,index,(index+1)%5);
    }
}
void initialize_philosophers(){
	char *names[5] = {"Plato","Aristotle","Socrates","Isaac Newton","Galileo Galilei"};
	int i=0,j=0;
	for (i = 0; i < 5; i++){
		fork_pairs[i]=0;
		philosophers[i].name = names[i];
		philosophers[i].left= j++;
		philosophers[i].right= j % 5;
	}
}

int main(int argc, char **argv) {
	int i;
	void* process_func = process;
	initialize_philosophers();
	pthread_t philosopher_threads[5]; // doesn't have the same ring to it as "stone"
	for (i = 0; i < 5; i++)		
		pthread_create(&philosopher_threads[i], NULL, process_func, NULL);
    for(;;){ // lag out main to stop from finishing, allowing threads to run infinitely
    }
}