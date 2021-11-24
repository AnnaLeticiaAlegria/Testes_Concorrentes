/*
Module: main.c
Author: Anna Leticia Alegria
Last Modified at: 19/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the main file of the First Example. 
In this example, there is a variable called 'count'. All the threads manipulate the same variable (They increment by 1
it's value). It does not uses semaphore to protect the variable count. This example simulates the operation of read a 
value and update this value. This example simulates that by assigning 'count' value to 'aux' and then changing 'count' 
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

int count = 0; // Variable that all threads increment

/* Encapsulated function's declarations */

void initializeThreads (int nThreads);
void freeThreads (int nThreads);
void * threadFunction (void * id);

/*
----------------------------------------------------------------------------------------------------------------------
Function: main
Parameters: 
  -> argc: number of parameters passed during execution
  -> argv: list of parameters passed
Returns:
  -> 0: In case of success
  -> 1: In case of error

Description: This main function receives the arguments passed by the command line. It receives the number of threads
the user wishes to create and the path of the states file. It calls the functions to initialize threads and prints
the final value of the variable 'count', that is manipulated by all threads. Since this version of the code doesn't
use semaphores, it is expected to print wrong values of 'count'.
At last, the main function frees the memory spaces allocated and finalizes the manager.
----------------------------------------------------------------------------------------------------------------------
*/
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


/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeThreads
Parameters: 
  -> nThreads: number of threads the user wants to create
Returns: nothing

Description: This function allocates the memory space to threadArray and to idArray (arrays with length of 'nThreads')
It calls the thread's functions and make the main thread wait for their conclusion.
----------------------------------------------------------------------------------------------------------------------
*/
void initializeThreads (int nThreads) {
  int i;
  int * id;

  threadArray = (pthread_t*) malloc (nThreads * sizeof(pthread_t));
  if (threadArray == NULL) {
    printf("Error during threadArray alloc\n");
  }

  /* This vector is important to every thread have a unique id and to be possible to free the allocated memory space
  after */
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

/*
----------------------------------------------------------------------------------------------------------------------
Function: freeThreads
Parameters: 
  -> nThreads: number of threads created by the program
Returns: nothing

Description: This function frees the memory space allocated by this program
----------------------------------------------------------------------------------------------------------------------
*/
void freeThreads (int nThreads) {
  free(threadArray);
  for(int i=0;i<nThreads;i++) {
    free(idArray[i]);
  }
  free(idArray);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: threadFunction
Parameters: 
  -> id: contains the address to this thread's id
Returns: nothing

Description: This function is called by every thread created by the user. It simulates an increment operation that can
be wrong if the user doesn't protect the access to the variable 'count'. To simulate this, the value of 'count' is
stored in 'aux'. This is the event on state "ReadCount". After that, 'count' receives the value of 'aux' incremented
by 1. This is the event on state "Update Count". If the state's file makes two threads execute the state "ReadCount"
before any of them execute "UpdateCount", they will overwrite one increment and the final result will be wrong. 
If you check the file "main_with_semaphore.c", you'll see what happens if this operation is protected by semaphores.
----------------------------------------------------------------------------------------------------------------------
*/
void * threadFunction (void * id) {
  int myId = *((int *) id);
  int aux;
  printf("---Thread %d started\n", myId);
  checkCurrentEvent("ThreadStarts");

  checkCurrentEvent("ReadCount");
  aux = count;
  checkCurrentEvent("UpdateCount");
  count = aux + 1;
  printf("---Thread %d wrote %d\n", myId, count);

  pthread_exit(NULL);
}
