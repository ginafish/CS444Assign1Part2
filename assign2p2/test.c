/****
 *Author: Taylor Fahlman
 *Assignment: The Dining Philosophers Problem
 *Date Due: 10/12/2015
****/

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

//void sig_catch(int sig);
static pthread_mutex_t forks[5] = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER
};
pthread_mutex_t fork1, fork2, fork3, fork4, fork5;
struct philosopher philosophers[5];
int fork_pairs[5];
int global_index=0;
void plato(void);
void locke(void);
void pythagoras(void);
void socrates(void);
void marx(void);
void eat(void);
void think(void);
int gen_number(int high, int low);

void sig_catch(int sig){/*
    printf("Catching signal %d\n", sig);
    printf("fork12: %d\n", fork12);
    printf("fork23: %d\n", fork23);
    printf("fork34: %d\n", fork34);
    printf("fork45: %d\n", fork45);
    printf("fork51: %d\n", fork51);
    pthread_mutex_destroy(&forks[0]);
    pthread_mutex_destroy(&forks[1]);
    pthread_mutex_destroy(&forks[2]);
    pthread_mutex_destroy(&forks[3]);
    pthread_mutex_destroy(&forks[4]);
    kill(0,sig);
    exit(0);*/
}

int gen_number(int high, int low) {

    int num = 0;

    num = (int)genrand_int32();
    num = abs(num);
    num %= (high - low);
    if (num < low)
    {
        num = low;
    }

    return num;
}

void eat(void){
    sleep(gen_number(9, 2));
}

void think(void){
    sleep(gen_number(20, 1));

}

void process(void){
	int left_pair,index;
	index=global_index++;
    while(1){
        //Think
        printf("   %s is thinking\n",philosophers[index].name);
        think();
        printf("   %s is done thinking\n",philosophers[index].name);
        //Check that the adjacent philos dont have forks
        //If they do, wait on signal 
        //Philosopher 1, gets forks 1,2
	
        while(fork_pairs[index] == 1)
        {
            printf("   %s is waiting for fork %d\n",philosophers[index].name,index);
            sleep(5);
        }
        while(fork_pairs[(index + 1) % 5] == 1)
        {
            printf("   %s is waiting for fork %d\n",philosophers[index].name,index+1);
            sleep(5);
        }
        pthread_mutex_lock(&forks[philosophers[index].left]);
        pthread_mutex_lock(&forks[philosophers[index].right]);
        fork_pairs[index] = 1;
		fork_pairs[(index+1)%5] = 1;


        //Do eat
        printf("   %s is eating\n",philosophers[index].name);
        eat();
		printf("   %s is done eating\n",philosophers[index].name);

        //Puts forks
        pthread_mutex_unlock(&forks[0]);
        pthread_mutex_unlock(&forks[1]);
        fork_pairs[index] = 0;
		fork_pairs[(index+1)%5] = 0;
        printf("   %s has put forks %d and %d down\n",philosophers[index].name,index,(index+1)%5);
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
	struct sigaction sig;
	 void* process_func = process;
	initialize_philosophers();
	pthread_t philosopher_threads[5]; // doesn't have the same ring to it as "stone"
	for (i = 0; i < 5; i++){
		//printf("%s\n",philosophers[i].name);
		//printf("%d\n",philosophers[i].left);
		//printf("%d\n",philosophers[i].right);
		
		
		pthread_create(&philosopher_threads[i], NULL, process_func, NULL);
		
	}
	
	
   
    pthread_t plato_thread;
    pthread_t locke_thread;
    pthread_t pythag_thread;
    pthread_t socrates_thread;
    pthread_t marx_thread;
	
    for(;;){ // lag out main to stop from finishing, allowing threads to run infinitely

    }
}