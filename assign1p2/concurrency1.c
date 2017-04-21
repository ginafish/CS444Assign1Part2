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
#include "mt19937ar.h"

int system_type = 0;


struct DATA{
	int number_value; //gets random value from RNG
	int wait_length; //gets random value from RNG
	
};


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
	if (system_type)
		__asm__ __volatile__("rdrand %0":"=r"(number));
	else
		number = (int)genrand_int32(); //Mersenne Twister
	abs(number);		// convert to positive number
	// check for out of bounds
	if (number < lower)
		number = lower; 
	if (number > upper)
		number = upper;
		
	return number;
}

main(){
	pthread_t consumer1, consumer2;
	pthread_t producer1, producer2;
	
	system_check(); // sets global variable depending on type
	
	
}