/****************************
* Brandon Thenell | Gina Phipps | Nawwaf Almutairi
* CS 444 - Concurrency 1 
* Group 13-05
****************************/

/****References**************
system_check() -> 0x40000000 - http://stackoverflow.com/questions/9994275/anything-specific-about-the-addresses-0x40000000-0x80000000-and-0xbf000000
mutatrix -> genrand_int32() - https://github.com/ekg/mutatrix/blob/master/mt19937ar.h
qemu flags - http://download.qemu.org/qemu-doc.html#index-_002dnet
****************************/

//delete after fulfilled
/************REQUIREMENTS**********************************************************************************************
* The item in the buffer should be a struct with two numbers in it.
	** 	The first value is just a number. The consumer will print out this value as part of its consumption.
	
	** 	The second value is a random waiting period between 2 and 9 seconds, which the consumer 
		will sleep for prior to printing the other value. This is the "work" done to consume the item.
		
	** 	Both of these values should be created using the rdrand x86 ASM instruction on systems that support 
		it, and using the Mersenne Twister on systems that don't support rdrand. It is your responsibility 
		to learn how to do this, how to include it in your code, and how to condition the value such that 
		it is within the range you want. Being able to work with x86 ASM is actually a necessary skill in this class. 
		Hint: os-class does not support rdrand. Your laptop likely does.
		
		
* Your producer should also wait a random amount of time (in the range of 3-7 seconds) before "producing" a new item.
* Your buffer in this case can hold 32 items. It must be implicitly shared between the threads.
* Use whatever synchronization construct you feel is appropriate.
**************************************************************************************************************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include "mt19937ar.h"

int system_type = 0, counter;


struct DATA{
	int number_value; //gets random value from RNG
	int wait_length; //gets random value from RNG
	
};


/* The mutex lock */
pthread_mutex_t mutex;

/* the buffer */
struct DATA buffer[32];
int num_data;

pthread_attr_t attr; //Set of thread attributes
/* pseudo semaphores? */
pthread_cond_t empty;
pthread_cond_t full;

void *producer(void *param); /* the producer thread */
void *consumer(void *param); /* the consumer thread */

/**************************************************************************
Name: 		void system_check()
Purpose: 	Checks the system to see if rdrand functionality is allowed.
Notes: 		Uses/sets global variable "system_type"
**************************************************************************/
void system_check(){
	unsigned int eax = 0x01;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
    __asm__ __volatile__(
	                     "cpuid;"
	                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
	                     : "a"(eax)
	                     );
						 
	// 32 bit check
	if (ecx & 0x40000000) // found this value on stackoverflow,check references above
		system_type = 1;
	else // 64 bit
		system_type = 0;
	
}



void initialize_data() {

   /* Create the mutex lock */
   pthread_mutex_init(&mutex, NULL);

   /* Get the default attributes */
   pthread_attr_init(&attr);
   /* Set empty pthread_cond_t */
   pthread_cond_init(&empty,NULL);
   /* Set full pthread_cond_t */
   pthread_cond_init(&full,NULL);

   /* init buffer */
   num_data = 0;
}

/* Producer Thread */
void *producer(void *param) {



   while(1) {
     
    struct DATA product;
	product.number_value = RNG(0,1000);
	product.wait_length = RNG(3,7);
	

    /* Lock */
    pthread_mutex_lock(&mutex);

    /* loop to make threads wait for space, only hits if full */
	while(num_data == 32){
		pthread_cond_wait(&full,&mutex);
	}
	/* if previous loop hits, num_data won't add to buffer until space */
	buffer[num_data++] = product;
    /* Let's consumers know there is a product available. */
	if (num_data == 1)
		pthread_cond_broadcast(&empty);
	/* release the mutex lock */
	pthread_mutex_unlock(&mutex);
	/* sleep for designated rand sleep time before producting again */
	sleep(RNG(2,9));
	
   }
}


void *consumer(void *param) {
	/**** Basically the same function as producer ******/
	struct DATA product;
	while(1){
		pthread_mutex_lock(&mutex);
		while(num_data==0) //while there are no current jobs
			pthread_cond_wait(&empty,&mutex); //wait for something to come in
		
		//take product from producer and decrement
		product=buffer[num_data--];
		//tells producer there is space available
		if (num_data == 31)
			pthread_cond_broadcast(&full);
		pthread_mutex_unlock(&mutex);
		sleep(product.wait_length);
		printf("Consumer received product after %d seconds and it's value is %d\n", product.wait_length,product.number_value);
			
	}
}

/**************************************************************************
Name: 		int RNG(int lower, int upper)
Purpose: 	Generates and returns random number
Parameters:	
			Lower: lowest possible number that can be returned
			Upper: Highest possible number that can be returned
Notes: 		Uses global variable "system_type"
**************************************************************************/
int RNG(int lower, int upper){
	
	int number;
	if (system_type)	//iff 32 bit, use x86 ASM
		__asm__ __volatile__("rdrand %0":"=r"(number));
	else
		number = (int)genrand_int32(); //iff 64 bit, use Mersenne Twister
	
	// check for out of bounds
	number = number % upper;
	
	if (number < 0 )
		number = number * -1;
	if (number < lower)
		number = number + lower;
	return number;
}


int main(int argc, char* argv[]){
	init_genrand(time(NULL));
	int i,num_consumers, num_producers;
	if (argc != 3 || argv[1]<0 || argv[2]<0){
		printf("Invalid arguments. Format:  ./concurrency1 <positive number of producers> <positive number of consumers>\n");
		exit(1);
	}
	system_check(); // sets global variable depending on type
	num_producers = atoi(argv[1]);
	num_consumers = atoi(argv[2]);
	pthread_t consumers_array[num_consumers];
	pthread_t producers_array[num_producers];
	initialize_data();
	/* Fill up pthread arrays */
	for(i = 0; i < num_consumers; i++){
		pthread_create(&producers_array[i], NULL, producer, NULL);
	}
	for(i = 0; i < num_consumers; i++){
		pthread_create(&consumers_array[i], NULL, consumer, NULL);
	}
	while(1)
		pause();
	
	
	
	
	return 0;
}