/* Brandon Thenell, Gina Phipps, Nawwaf Almutairi */
/* Concurrency 3 */
/* CS 444 - OS II - Spring 2017 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "mt19937ar.c"

// Global variables
struct linkedList list;
int eax, ebx, ecx, edx;
int searchers, inserters, deleters, number;
int searchIndex=1, insertIndex=1, deleteIndex=1;

// Mutexes
pthread_mutex_t searchLock;
pthread_mutex_t insertLock;
pthread_mutex_t deleteLock;


// LL
struct linkedList {
    int value;
    struct linkedList *next;
} *head;

void setRegisters() {
    // EAX must be set or seg fault will occure
    eax = 0x01;
    __asm__ __volatile__("cpuid;" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(eax));
}

void printList(){
	printf("__________________________________________________________\n");
	if (printList == NULL)
		return;
	struct linkedList *printList = head;
	
  while (printList != NULL) {
				printf("%d ", printList->value);
				printList = printList->next;
			}

	
	printf("\n__________________________________________________________\n");
	
	}

// RNG
int randomNumberGenerator(int min, int max) {
    int num = 0;
    setRegisters();

   // Taken from concurrency 1, depends on type of machine running
    if (ecx & 1<<30) {
        rdrand(&num);
    } else {
        num = (int)genrand_int32();
    }
    num = abs(num);
    num %= (max - min);
    if (num < min) {
        num = min;
    }

    return num;
}
int rdrand(int *num) {
	// Taken from concurrency 1, depends on type of machine running
    unsigned char err;
    __asm__ __volatile__("rdrand %0; setc %1" : "=r" (*num), "=qm" (err));
    return (int) err;
}


// returns size for checking purposes
int listSize() {
    struct linkedList *sizeList;
    sizeList = head;
    int i = 0;
    while (sizeList != NULL) { // iter through
        sizeList = sizeList->next;
        i++;
    }

    return i;
}

// Searcher thread implementation
void *searcher() {
    struct linkedList *searchLinkedList;
	int myIndex=searchIndex++;
    // activate searchers to continuously go through list
    while (1) {
			int flag=1;
			int num = randomNumberGenerator(1,10);
			
            searchLinkedList = head;

            // Check for empty list
            if (searchLinkedList == NULL) {
                printf("Empty List!\n");
                continue;
            } else {
				printf("Searcher #%d is looking for the number:  %d\n", myIndex,num);
                // Search through the list and print
                while (searchLinkedList != NULL) {      
					if (searchLinkedList->value == num){
						printf("Searcher #%d found the number it was searching for:  %d\n",myIndex,num);
						flag=0;
						break;
					}
					else
						searchLinkedList = searchLinkedList->next;
                }
				if (flag)
					printf("Searcher #%d could not find the number:  %d\n",myIndex,num);
            }
           
        
        sleep(randomNumberGenerator(1,5));
    }
}

// Inserter thread implementation
void *inserter() {
    int num;
    struct linkedList *insertLinkedList, **tail;
	int myIndex=insertIndex++;
    // activate searchers to continuously go through list
    while (1) {
        if (listSize() < 25) {
            if (!pthread_mutex_trylock(&insertLock)) {

             
                num = randomNumberGenerator(1, 10);
                insertLinkedList = (struct linkedList *)malloc(sizeof(struct linkedList));
				insertLinkedList->value = num;
                insertLinkedList->next = NULL;
				printf("Inserter #%d is inserting the value: %d and the list looks like: \n",myIndex, num);
                printList();
				tail = &head;

               
                if (head == NULL) {
                    head == insertLinkedList;
                } else {
                 
                    while (*tail != NULL) {
                        tail = &((*tail)->next);
                    }

                    *tail = insertLinkedList;
                }

                pthread_mutex_unlock(&insertLock);
                sleep(2);
            }
			else{
				printf("Inserter #%d cannot insert right now due to other active inserters\n",myIndex);
				sleep(2);
			}
		}
    }
}

// Deleter thread implementation
void *deleter() {
    int num;
    struct linkedList *deleteLinkedList, *previous;
	int myIndex = deleteIndex++;
// activate deleters to continuously go through list
    while (1) {

        // check to ensure it's not empty
        if (listSize() > 1) {

            // checking constraints
            if (!pthread_mutex_trylock(&insertLock)) {
                if (!pthread_mutex_trylock(&searchLock)) {                 
                    deleteLinkedList = head;
                    num = randomNumberGenerator(1, 10);

                    // While there's still nodes in the linked list
                    while (deleteLinkedList != NULL) {

                        // If the current node has the same value as our target
                        if (deleteLinkedList->value == num) {
                            printf("Deleter #%d is removing value: %d and the list looks like:\n",myIndex, num);
							printList();
                            // move head if it's gonna be removed
                            if (deleteLinkedList == head) {
                                head = deleteLinkedList->next;
                                free(deleteLinkedList);
                                break;
                            } else {
                                previous->next = deleteLinkedList->next;
                                free(deleteLinkedList);
                                break;
                            }
                        } else {
                            previous = deleteLinkedList;
                            deleteLinkedList = deleteLinkedList->next;
                        }
                    }

                    pthread_mutex_unlock(&searchLock);
                }
				else{
					printf("Deleter #%d cannot delete right now due to active searchers\n",myIndex);
					sleep(1);
				}
                pthread_mutex_unlock(&insertLock);
				
            }
			else{
					printf("Deleter #%d cannot delete right now due to active inserts\n",myIndex);
					sleep(1);
				}
			
            
            sleep(2);
        }
    }
}


int main(int argc, char const *argv[]) {
	int i;
    // Initialize threads
   pthread_t searchThread[3], insertThread[3], deleteThread[3];
	
    // Initialize List
    struct linkedList *list;
    list = (struct linkedList *)malloc(sizeof(struct linkedList));
    list->value = randomNumberGenerator(1, 10);
    head = list;
    head->next = NULL;

    // create 3 of each: insert, search, and destroy
    
    for (i = 0; i < 3; i++) {
        pthread_create(&searchThread[i], NULL, searcher, NULL);
        pthread_create(&insertThread[i], NULL, inserter, NULL);
        pthread_create(&deleteThread[i], NULL, deleter, NULL);
    }

    // initialize
    for (i = 0; i < 3; i++) {
        pthread_join(searchThread[i], NULL);
        pthread_join(insertThread[i], NULL);
        pthread_join(deleteThread[i], NULL);
    }

    return 0;
}