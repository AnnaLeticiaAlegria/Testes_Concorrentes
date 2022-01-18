#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "../EventManager/eventManager.h"
#include "../ConcurrencyModule/concurrency.h"


/* Global variable's declaration */

int nProducers;
int nConsumers;

sem_t * empty;
sem_t * full;
sem_t * exc;

int * buffer;
int bufferSize;

int nxtfree = 0;
int nxtdata = 0;

/* Encapsulated function's declarations */

void* producer (void * num);

void* consumer (void * num);

int main(int argc, char** argv) 
{ 
  pthread_t * producersArray;
  pthread_t * consumersArray;

  int ** producersIdArray = NULL;
  int ** consumersIdArray = NULL;

	if (argc < 5 && argc > 6) {
    printf("Program needs 4 or 5 parameters: nProducers, nConsumers, bufferSize, eventFileName and configFileName(optional) \n");
    return 0;
  }
  nProducers = strtol(argv[1], NULL, 10);
  nConsumers = strtol(argv[2], NULL, 10);
  bufferSize = strtol(argv[3], NULL, 10);
  initializeManager (argv[4], argv[5]);

  buffer = (int*) malloc (bufferSize * sizeof(int));
  if (buffer == NULL) {
    printf("Error in buffer malloc\n");
    exit(0);
  }

  producersArray = initializeThreads (nProducers, producersIdArray, producer);
  consumersArray = initializeThreads (nConsumers, consumersIdArray, consumer);
	
  empty = initializeSemaphore("/empty", bufferSize);
  full = initializeSemaphore("/full", 0);
  exc = initializeSemaphore("/exc", 1);

  joinThreads (producersArray, nProducers);
  joinThreads (consumersArray, nConsumers);

	finalizeManager();

  freeSemaphore(empty);
  freeSemaphore(full);
  freeSemaphore(exc);

  free(buffer);

  freeThreads (producersArray, nProducers, producersIdArray);
  freeThreads (consumersArray, nConsumers, consumersIdArray);

	return 0;
}

void* producer (void * num) {
  int id = *((int *) num);
  int a;

  while(1) {
    a = rand() % 100;

    checkCurrentEvent("ProducerWantsToStart");

    sem_wait(empty);

    checkCurrentEvent("ProducerPassedEmpty");

    sem_wait(exc);

    checkCurrentEvent("ProducerStarts");

    buffer[nxtfree] = a;

    nxtfree = (nxtfree + 1) % bufferSize;

    checkCurrentEvent("ProducerEnds");
    sem_post(exc);

    checkCurrentEvent("ProducerPostsFull");

    sem_post(full);

    printf("--------Producer %d produced item %d--------\n", id, a);
  }


	pthread_exit(NULL); 
}

void* consumer (void * num) {
  int id = *((int *) num);
  int b;

  while(1) {

    checkCurrentEvent("ConsumerWantsToStart");

    sem_wait(full);

    checkCurrentEvent("ConsumerPassedFull");

    sem_wait(exc);

    checkCurrentEvent("ConsumerStarts");
    
    b = buffer[nxtdata];

    nxtdata = (nxtdata + 1) % bufferSize;

    checkCurrentEvent("ConsumerEnds");

    sem_post(exc);

    checkCurrentEvent("ConsumerPostsEmpty");

    sem_post(empty);

    printf("--------Consumer %d consumed item %d--------\n", id, b);
  }

	pthread_exit(NULL); 
}



