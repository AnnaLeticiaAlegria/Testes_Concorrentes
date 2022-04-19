/*
Module: main.c
Author: Prof. Silvana Rossetto (this example is used by class Computacao Concorrente of UFRJ)
Modified by: Anna Leticia Alegria
Last Modified at: 23/12/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the main file of the Barrier example. At each step, a thread increments the variable
'aux'. The threads can only go on if every thread has reached the barrier. It is a syncronization point. After 'nSteps'
steps, all threads end.
There are 3 barrier's implementations on this module. Two of them are wrong and the last one is correct.
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

sem_t * mutex;
sem_t * cond;

int nThreads, nSteps;
int functionToUse;
int arrived = 0;

/* Encapsulated function's declarations */

void * threadFunction (void * id);

void barrier_v1(int numThreads);
void barrier_v2(int numThreads);
void barrier_v3(int id, int numThreads);

/*
----------------------------------------------------------------------------------------------------------------------
Function: main
Parameters: 
  -> argc: number of parameters passed during execution
  -> argv: list of parameters passed
Returns:
  -> 0: In case of success
  -> 1: In case of error

Description: This main function receives the arguments passed by the command line. It receives the number of the
barrier function to be called (1 for barrier_v1, 2 for barrier_v2 and 3 for barrier_v3), the number of threads, the
number of steps and the states file name. It calls the functions to initialize threads, to
initialize semaphores and to initialize the manager.
At last, the main function frees the memory spaces allocated and finalizes the manager.
----------------------------------------------------------------------------------------------------------------------
*/
int main (int argc, char** argv) {
  
  pthread_t * threadArray;
  int ** idArray = NULL;

  if (argc != 5) {
    printf("Program needs 4 parameters: functionToUse, nThreads, nSteps and statesFileName \n");
    return 0;
  }
  functionToUse = strtol(argv[1], NULL, 10);
  nThreads = strtol(argv[2], NULL, 10);
  nSteps = strtol(argv[3], NULL, 10);

  initializeManager (argv[4], NULL);

  mutex = initializeSemaphore("/mutex", 1);
  cond = initializeSemaphore("/cond", 0);

  threadArray = initializeThreads (nThreads, idArray, threadFunction);

  joinThreads(threadArray, nThreads);

  freeThreads (threadArray, nThreads, idArray);

  freeSemaphore(mutex);
  freeSemaphore(cond);

  finalizeManager();
  return 0;
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: threadFunction
Parameters: 
  -> id: contains the address to this thread's id
Returns: nothing

Description: This function is called by every thread created by the user. Each thread executes 'nSteps' times the code
inside the for. It increments the local variable 'aux1' and calls a barrier function, based on 'functionToUse's value.
After that, the thread does some calculations.
----------------------------------------------------------------------------------------------------------------------
*/
void * threadFunction (void * id) {
  int myId = *((int *) id);
  int aux1 = 0, aux2, aux3;

  for (int i=0; i < nSteps; i++) {
    aux1++;
    // printf("Thread %d: step=%d\n", myId, i);

    /* barreira */
    switch(functionToUse) {
      case 1:
        barrier_v1(nThreads);
        break;
      case 2:
        barrier_v2(nThreads);
        break;
      default:
        barrier_v3(myId, nThreads);
        break;
    }

    /* do something... */
    aux2=100; aux3=-100; while (aux3 < aux2) aux3++;
  }
  pthread_exit(NULL);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: barrier_v1
Parameters: 
  -> numThreads: the total number of threads
Returns: nothing

Description: This function is the first version of a barrier. It is incorrect.
The thread gets the lock of semaphore 'mutex' to access the variable 'arrived'. If it isn't the last one to arrive,
it releases the semaphore 'mutex' and waits on semaphore 'cond'. If it is the last one to arrive, it releases every
thread that is waiting on 'cond', sets 'arrived' to 0 and releases the semaphore 'mutex'
This version is wrong because the last thread to arrive stacks sem_post's calls and then, releases the semaphore 
'mutex'. If this threads executes the next step before the other threads consume the sem_post on semaphore 'cond', it 
can acquire the semaphore 'mutex' and will not wait on the barrier, since the sem_wait on 'cond' won't work.
----------------------------------------------------------------------------------------------------------------------
*/
void barrier_v1(int numThreads) {
  sem_wait(mutex);
  checkCurrentEvent("ThreadStarts");

  arrived++;
  if (arrived < numThreads) {
    sem_post(mutex);

    checkCurrentEvent("ThreadWaits");
    sem_wait(cond);

    checkCurrentEvent("ThreadPassed");
  } 
  else {
    for(int i=1; i<numThreads; i++)
    {
      checkCurrentEvent("WantsToRelease");
      sem_post(cond); 
      checkCurrentEvent("ReleaseAThread");
    }
    arrived = 0;
    sem_post(mutex);
  }
}


/*
----------------------------------------------------------------------------------------------------------------------
Function: barrier_v2
Parameters: 
  -> numThreads: the total number of threads
Returns: nothing

Description: This function is the second version of a barrier. It is incorrect.
The thread gets the lock of semaphore 'mutex' to access the variable 'arrived'. If it isn't the last one to arrive,
it releases the semaphore 'mutex' and waits on semaphore 'cond'. If it is the last one to arrive, it releases one 
thread that is waiting on 'cond' and decrements 'arrived'.
The thread that was waiting decrements 'arrived' and releases another thread. If it is the last Thread, it releases
the semaphore 'mutex'.
This version is wrong because the decrement of 'arrived' by the last thread to arrive is being done after the sem_post.
If this thread is interrupted and all the other threads execute before this one returns, no thread will enter the
arrived == 0 condition, not releasing the semaphore 'mutex'. There will be a deadlock.
----------------------------------------------------------------------------------------------------------------------
*/
void barrier_v2(int numThreads) {
  sem_wait(mutex);
  checkCurrentEvent("ThreadStarts");

  arrived++;
  if (arrived < numThreads) {
    sem_post(mutex);

    checkCurrentEvent("ThreadWaits");
    sem_wait(cond);

    checkCurrentEvent("ThreadPassed");
    arrived--;
    if (arrived==0) {
      checkCurrentEvent("LastThreadPosts");
      sem_post(mutex);
    }
    else {
      sem_post(cond); 
    }
  } else {
    sem_post(cond); 
    checkCurrentEvent("EveryThreadArrived");
    arrived--; //it should have been called before previous line
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: barrier_v3
Parameters: 
  -> numThreads: the total number of threads
Returns: nothing

Description: This function is the second version of a barrier. It is correct!!!
The thread gets the lock of semaphore 'mutex' to access the variable 'arrived'. If it isn't the last one to arrive,
it releases the semaphore 'mutex' and waits on semaphore 'cond'. If it is the last one to arrive, it decrements 
'arrived' and releases one thread that is waiting on 'cond'.
The thread that was waiting decrements 'arrived' and releases another thread. If it is the last Thread, it releases
the semaphore 'mutex'.
----------------------------------------------------------------------------------------------------------------------
*/
void barrier_v3(int id, int numThreads) {
  sem_wait(mutex);
  checkCurrentEvent("ThreadStarts");

  arrived++;
  if (arrived < numThreads) {
    sem_post(mutex);

    checkCurrentEvent("ThreadWaits");
    sem_wait(cond);

    checkCurrentEvent("ThreadPassed");
    arrived--;
    if (arrived==0) {
      checkCurrentEvent("LastThreadPosts");
      sem_post(mutex);
    }
    else {
      sem_post(cond); 
    }
  } else {
    arrived--;
    sem_post(cond);
    checkCurrentEvent("EveryThreadArrived");
  }
}
