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

int nThreads;

sem_t ** mutexArray;

/* Encapsulated function's declarations */

void initializeMutexArray (void);

void* philosopherFunction (void * num);

void eat (int philosopher);
void think (int philosopher);

void getFork (int philosopher, int fork);
void putFork (int philosopher, int fork);

int main(int argc, char** argv) 
{ 
  pthread_t * threadsArray;

  int ** threadsIdArray = NULL;

	if (argc < 4 && argc > 5) {
    printf("Program needs 3 or 4 parameters: nThreads, eventFileName and configFileName(optional) \n");
    return 0;
  }
  nThreads = strtol(argv[1], NULL, 10);
  initializeManager (argv[2], argv[3]);

  initializeMutexArray ();

  threadsArray = initializeThreads (nThreads, threadsIdArray, philosopherFunction);
	
  joinThreads (threadsArray, nThreads);

	finalizeManager();

  for (int i=0;i<nThreads;i++) {
    freeSemaphore(mutexArray[i]);
  }

  free(mutexArray);

  // freeThreads (threadsArray, nThreads, threadsIdArray);

	return 0;
}

void initializeMutexArray (void) {
  char stringName [11];

  mutexArray = (sem_t**) malloc (nThreads * sizeof(sem_t*));
  if (mutexArray == NULL) {
    printf("Error in mutexArray malloc\n");
    exit(0);
  }

  strcpy(stringName, "/semMutex");

  for (int i=0; i<nThreads; i++) {
    stringName[9] = 'a' + i;
    stringName[10] = '\0';
    mutexArray[i] = initializeSemaphore(stringName, 1);
  }
}

void* philosopherFunction (void * num) {
  int id = *((int *) num);

  while(1) {
    checkCurrentEventWithId ("Starting", id + 1);
    think(id);

    if(id == 0) {
      checkCurrentEvent("LeftieWantsFork1");
      getFork(id, nThreads - 1);
      checkCurrentEvent("LeftieGotFork1");

      checkCurrentEvent("LeftieWantsFork2");
      getFork(id, 0);
      checkCurrentEvent("LeftieGotFork2");
    }
    else {
      checkCurrentEvent("RightieWantsFork1");
      getFork(id, id);
      checkCurrentEvent("RightieGotFork1");

      checkCurrentEvent("RightieWantsFork2");
      getFork(id, id - 1);
      checkCurrentEvent("RightieGotFork2");
    }

    checkCurrentEvent("Eating");
    eat(id);

    if(id == 0) {
      checkCurrentEvent("LeftieGivesFork1");
      putFork(id, nThreads - 1);
      checkCurrentEvent("LeftiePutFork1");

      checkCurrentEvent("LeftieGivesFork2");
      putFork(id, 0);
      checkCurrentEvent("LeftiePutFork2");
    }
    else {
      checkCurrentEvent("RightieGivesFork1");
      putFork(id, id);
      checkCurrentEvent("RightiePutFork1");

      checkCurrentEvent("RightieGivesFork2");
      putFork(id, id - 1);
      checkCurrentEvent("RightiePutFork2");
    }
  }
	pthread_exit(NULL); 
}


void eat (int philosopher) {
  printf("--------------Philosopher %d eating--------------\n", philosopher + 1);
  sleep(1);
}

void think (int philosopher) {
  printf("--------------Philosopher %d thinking--------------\n", philosopher + 1);
  sleep(1);
}

void getFork (int philosopher, int fork) {

  sem_wait(mutexArray[fork]);

  printf("--------------Philosopher %d got fork %d--------------\n", philosopher + 1, fork);
}

void putFork (int philosopher, int fork) {

  sem_post(mutexArray[fork]);

  printf("--------------Philosopher %d put fork %d--------------\n", philosopher + 1, fork);
}