/*
Module: main.c
Author: Anna Leticia Alegria
Last Modified at: 17/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the main file of the First Example. 
In this example, there is a variable called 'count'. All the threads manipulate the same variable (They increment by 1
it's value). It does not uses semaphore to protect the variable count. This example simulates the operation of read a 
value and update this value. This example simulates that assigning 'count' value to 'aux' and then changing 'count' 
value. Inserting events before each one of this operating, it is possible to notice that a thread can be interrupted 
and another thread can read the same value of 'count'. Both of them would write the same value of 'count', when they
should write sequential values.
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "../StateManager/stateManager.h"

/* Global variable's declaration */

pthread_t * threadArray;
int ** idArray;

int count = 0;

void initializeThreads (int nThreads);
void freeThreads (int nThreads);
void * threadFunction (void * id);

int main (int argc, char** argv) {
  int nThreads;

  if (argc != 3) {
    printf("Program needs 2 parameters: nThreads and statesFileName \n");
    return 0;
  }
  nThreads = strtol(argv[1], NULL, 10);
  initializeManager (argv[2], nThreads);
  
  initializeThreads (nThreads);

  printf("---Final value of count: %d\n",count);

  freeThreads (nThreads);
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

void * threadFunction (void * id) {
  int myId = *((int *) id);
  int aux;
  printf("---Thread %d started\n", myId);
  checkState("ThreadStarts");
  checkState("ReadCount");
  aux = count;
  checkState("UpdateCount");
  count = aux + 1;
  printf("---Thread %d wrote %d\n", myId, count);

  pthread_exit(NULL);
}
