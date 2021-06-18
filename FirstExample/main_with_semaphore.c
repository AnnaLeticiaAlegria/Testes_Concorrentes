#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#include "../StateManager/stateManager.h"

pthread_t * threadArray;
int ** idArray;

sem_t * semaphoreE;

int count = 0;

void initializeThreads (int nThreads);
void freeThreads (int nThreads);
void * threadFunction (void * id);
char * nameRandomize (char * name, int nLetters);
void initializeSemaphore (void);
void freeSemaphore (void);

int main (int argc, char** argv) {
  int nThreads;

  if (argc != 3) {
    printf("Program needs 2 parameters: nThreads and statesFileName \n");
    return 0;
  }
  nThreads = strtol(argv[1], NULL, 10);
  initializeManager (argv[2], nThreads);
  initializeSemaphore();
  
  initializeThreads (nThreads);

  printf("---Final value of count: %d\n",count);

  freeThreads (nThreads);
  freeSemaphore();
  finalizeManager();
  return 0;
}

void initializeThreads (int nThreads) {
  int i;
  int * id;

  threadArray = (pthread_t*) malloc (nThreads * sizeof(pthread_t));
  if (threadArray == NULL) {
    printf("Error during threadArray alloc\n");
  }

  idArray = (int**) malloc (nThreads * sizeof(int*));
  if (idArray == NULL) {
    printf("Error during idArray alloc\n");
  }

  for (i = 0; i < nThreads ; i++) {
    id = (int *) malloc (sizeof(int));
    if (id == NULL) {
      printf("Error during id alloc\n");
    }
    *id = i;
    idArray[i] = id;
    pthread_create(&threadArray[i], NULL, threadFunction, (void*) id);
  }

  for(i = 0; i < nThreads; i++) {
    pthread_join(threadArray[i],NULL);
  }
}

void freeThreads (int nThreads) {
  free(threadArray);
  for(int i=0;i<nThreads;i++) {
    free(idArray[i]);
  }
  free(idArray);
}

char * nameRandomize (char * name, int nLetters) {
  int i, nameSize;
  char * randomName;
  nameSize = strlen(name);
  randomName = (char *) malloc ((nameSize + nLetters + 1) * sizeof(char));
  if (randomName == NULL) {
    printf("Error during randomName alloc\n");
    exit(0);
  }

  strcpy(randomName, name);
  for(i=0;i<nLetters;i++){
    randomName[nameSize + i] = 'A' + rand()%26;
  }
  randomName[nameSize + i] = '\0';

  return randomName;
}

void initializeSemaphore (void) {
  char * semName;

  semName = nameRandomize ("/fSemE", 10);
  sem_unlink(semName);
  semaphoreE = sem_open(semName, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 1); 
  if (semaphoreE == SEM_FAILED){
    printf("Error opening semaphore semaphoreE\n");
    free(semName);
    exit(0);
  }
  free(semName);
}

void freeSemaphore (void) {
  sem_close(semaphoreE);
}

void * threadFunction (void * id) {
  int myId = *((int *) id);
  int aux;
  printf("---Thread %d started\n", myId);

  checkState("ThreadStarts");
  sem_wait(semaphoreE);
  checkState("ReadCount");
  aux = count;
  checkState("UpdateCount");
  count = aux + 1;
  sem_post(semaphoreE);

  printf("---Thread %d wrote %d\n", myId, count);

  pthread_exit(NULL);
}
