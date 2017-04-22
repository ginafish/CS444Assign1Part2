/****************************
* Brandon Thenell | Gina Phipps | Nawwaf Almutairi
* CS 444 - Concurrency 1 
* Group 13-05
****************************/

/****References**************
system_check() -> 0x40000000 - http://stackoverflow.com/questions/9994275/anything-specific-about-the-addresses-0x40000000-0x80000000-and-0xbf000000
mutatrix -> genrand_int32() - https://github.com/ekg/mutatrix/blob/master/mt19937ar.h

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

/* the semaphores */
sem_t full, empty;

/* the buffer */
struct DATA buffer[32];

pthread_attr_t attr; //Set of thread attributes

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

/* Add an item to the buffer */
int insert_item(int item) {
   /* When the buffer is not full add the item
      and increment the counter*/
   if(counter < 32) {
      buffer[counter].number_value = item;
      counter++;
      return 0;
   }
   else { /* Error the buffer is full */
      return -1;
   }
}

/* Remove an item from the buffer */
int remove_item(int *item) {
   /* When the buffer is not empty remove the item
      and decrement the counter */
   if(counter > 0) {
      *item = buffer[(counter-1)].number_value;
      counter--;
      return 0;
   }
   else { /* Error buffer empty */
      return -1;
   }
}

void initializeData() {

   /* Create the mutex lock */
   pthread_mutex_init(&mutex, NULL);

   /* Create the full semaphore and initialize to 0 */
   //sem_init(&full, 0, 0);

   /* Create the empty semaphore and initialize to BUFFER_SIZE */
   //sem_init(&empty, 0, 32);

   /* Get the default attributes */
   pthread_attr_init(&attr);

   /* init buffer */
   counter = 0;
}

/* Producer Thread */
void *producer(void *param) {
   int item;

   while(1) {
      /* sleep for a random period of time */
      sleep(10);

      /* generate a random number */
      //item = rand();

      /* acquire the empty lock */
      //sem_wait(&empty);
      /* acquire the mutex lock */
      pthread_mutex_lock(&mutex);

      if(insert_item(item)) {
         fprintf(stderr, " Producer report error condition\n");
      }
      else {
         printf("producer produced %d\n", item);
      }
      /* release the mutex lock */
      pthread_mutex_unlock(&mutex);
      /* signal full */
      //sem_post(&full);
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

main(){
	pthread_t consumer1, consumer2;
	pthread_t producer1, producer2;


	/* buffer counter */
	int counter;
	int i;
	system_check(); // sets global variable depending on type
	
	
	
	
}

