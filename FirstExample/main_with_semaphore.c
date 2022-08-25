/*
Module: main_with_semaphore.c
Author: Anna Leticia Alegria
Last Modified at: 17/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the main file of the First Example using semaphores.
In this example, there is a variable called 'count'. All the threads manipulate the same variable (They increment by 1
it's value). In this case, it uses semaphore to protect the access to the variable 'count'. This example simulates 
the operation of read a value and update this value. This example simulates that by assigning 'count' value to 'aux' 
and then changing 'count' value. Inserting events before each one of this operating, it is possible to notice that a 
thread can be interrupted and another thread could read the same value of 'count' if semaphores were not being used.
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#include "../EventManager/eventManager.h"
#include "../ConcurrencyModule/concurrency.h"

/* Global variable's declaration */

sem_t * semaphoreE;

int count = 0; // Variable that all threads increment

/* Encapsulated function's declarations */

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
the user wishes to create and the path of the states file. It calls the functions to initialize threads and to
initialize semaphores and prints the final value of the variable 'count', that is manipulated by all threads.
At last, the main function frees the memory spaces allocated and finalizes the manager.
----------------------------------------------------------------------------------------------------------------------
*/
int main (int argc, char** argv) {

  pthread_t * threadArray;
  int ** idArray;
  int nThreads;

  if (argc != 3) {
    printf("Program needs 2 parameters: nThreads and eventFileName \n");
    return 0;
  }
  nThreads = strtol(argv[1], NULL, 10);
  initializeManager (argv[2], NULL);

  semaphoreE = initializeSemaphore("/fSemE", 1);
  
  threadArray = initializeThreads (nThreads, idArray, threadFunction);

  joinThreads(threadArray, nThreads);

  printf("---Final value of count: %d\n",count);

  freeThreads (threadArray, nThreads, idArray);

  freeSemaphore(semaphoreE);
  finalizeManager();
  return 0;
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
by 1. This is the event on state "Update Count". Both of this operations are protected by semaphores. So, if the 
state's file makes two threads execute the state "ReadCount" before any of them execute "UpdateCount", the program
won't accept this as a valid order, causing a deadlock. The state Manager detects that any state is being executed
for the last 'deadLockDetectTime' (which is 5, by default) and ends the program.
Valid sequences are executed without any problems.
----------------------------------------------------------------------------------------------------------------------
*/
void * threadFunction (void * id) {
  int myId = *((int *) id);
  int aux;
  printf("---Thread %d started\n", myId);

  checkCurrentEvent("ThreadStarts");
  sem_wait(semaphoreE); // P(semaphoreE)
  checkCurrentEvent("ReadCount");
  aux = count;
  checkCurrentEvent("UpdateCount");
  count = aux + 1;
  printf("---Thread %d wrote %d\n", myId, count);
  
  sem_post(semaphoreE); // V(semaphoreE)

  pthread_exit(NULL);
}
