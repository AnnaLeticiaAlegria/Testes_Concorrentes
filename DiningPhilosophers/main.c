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
int * forksArray;

sem_t * mutex;

/* Encapsulated function's declarations */

void* threadFunction (void * num);

void createForks ();
void deleteForks ();

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

  createForks();

  mutex = initializeSemaphore("/semMutex", 1);

  threadsArray = initializeThreads (nThreads, threadsIdArray, threadFunction);
	
  joinThreads (threadsArray, nThreads);

	finalizeManager();

  deleteForks();

  freeSemaphore(mutex);

  // freeThreads (threadsArray, nThreads, threadsIdArray);

	return 0;
}

void* threadFunction (void * num) {
  int id = *((int *) num);
  
  checkCurrentEvent("Thinking");
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

	pthread_exit(NULL); 
}

void createForks () {
  int i;
  forksArray = (int*) malloc (nThreads * sizeof(int));
  if (forksArray == NULL) {
    printf("Error allocating fork vector\n");
    exit(0);
  }

  for (i=0;i<nThreads;i++) {
    forksArray[i] = 1;
  }
}

void deleteForks () {
  free(forksArray);
}

void eat (int philosopher) {
  printf("--------------Philosopher %d eating--------------\n", philosopher);
  sleep(1);
}

void think (int philosopher) {
  printf("--------------Philosopher %d thinking--------------\n", philosopher);
  sleep(1);
}

void getFork (int philosopher, int fork) {
  int gotFork = 0;

  while(!gotFork) {
    sem_wait(mutex);
    if (forksArray[fork]) {
      gotFork = 1;
      forksArray[fork] = 0;
    }
    sem_post(mutex);
  }

  printf("--------------Philosopher %d got fork %d--------------\n", philosopher, fork);
}

void putFork (int philosopher, int fork) {
  sem_wait(mutex);
  forksArray[fork] = 1;
  sem_post(mutex);

  printf("--------------Philosopher %d put fork %d--------------\n", philosopher, fork);
}