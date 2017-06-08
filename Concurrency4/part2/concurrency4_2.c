#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#define CHAIRS 10


pthread_t threads[CHAIRS + 1]; //+1 for barber
int waitTime = 0;

//globals for semaphores
sem_t customerSem;
sem_t barberSem;
//mutex for mutual exclusion
pthread_mutex_t barberShopLock = PTHREAD_MUTEX_INITIALIZER;

void cut_hair(int ID){//helper function barber thread calls when cutting hair
    printf("\t\tThe Barber is cutting hair!\n");
    int rand_num = rand() % 3;
    sleep(2);
    printf("\t\tThe Barber is done!\n");
}
void get_hair_cut(int ID){//helper function customer thread calls when getting hair cut
    printf("Customer #%d: is getting hair cut!\n", ID);
    int rand_num = rand() % 2;
    sleep(2);
    printf("\033[1mCustomer #%d: is done, now leaving!\033\n", ID);
}
void* customer(void* ID){//customer thread
    int customerID = *((int*)ID);
    while(1){
        pthread_mutex_lock(&barberShopLock);
        if(waitTime < CHAIRS){
            waitTime++;
            sem_post(&customerSem);
            pthread_mutex_unlock(&barberShopLock);
            if(sem_trywait(&barberSem) == 0){
                printf("Customer #%d: in barber's chair!\n", customerID);
            }else{
                
                sem_wait(&barberSem);
            }
            get_hair_cut(customerID);
        }else {
            pthread_mutex_unlock(&barberShopLock);
            printf("Customer #%d: is leaving because there is no space\n",  customerID);
        }
    }
}
void* barber(void* ID){
    int barberID = *((int*)ID);
    while(1){
        if(sem_trywait(&customerSem) == 0)
            printf("\t\tThe Barber is waking up.\n");
        else{
            printf("\t\tThe Barber has no customers and fell  assleep!");
            sem_wait(&customerSem);
			printf("\t\tThe Barber is waking up.\n");	//repeat because barber might be already awake
        }
        pthread_mutex_lock(&barberShopLock);
        if(waitTime > 0)
            waitTime--;
        else
            waitTime = 0;
        sem_post(&barberSem);
        pthread_mutex_unlock(&barberShopLock);
        cut_hair(barberID);
    }
}

int main(){
    sem_init(&customerSem, 0 , 0);
    sem_init(&barberSem, 0, 0);
    pthread_mutex_init(&barberShopLock,NULL);
    int i = 0;
    for(i = 0; i < CHAIRS; i++){
        pthread_create(&threads[i],NULL, customer, &i);
    }
    pthread_create(&threads[i], NULL, barber, &i);
    for(i = 0; i < CHAIRS+1; i++)
		pthread_join(threads[i], NULL);
    while(1);//infinite loop
}
