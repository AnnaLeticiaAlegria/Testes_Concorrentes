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

#include "../StateManager/stateManager.h"

/* Global variable's declaration */

pthread_t * threadArray;
int ** idArray;

sem_t * semaphoreE;

int count = 0; // Variable that all threads increment

/* Encapsulated function's declarations */

void initializeThreads (int nThreads);
void freeThreads (int nThreads);
void * threadFunction (void * id);
char * nameRandomize (char * name, int nLetters);
void initializeSemaphore (void);
void freeSemaphore (void);

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
Function: nameRandomize
Parameters: 
  -> name: string to be modified by adding random letters at it's end
  -> nLetters: number of random letters to be added at the end of 'name'
Returns:
  -> randomName: A string containing the string 'name' + 'nLetters' number of letters after 'name'

Description: This function copies the letters in 'name' and adds 'nLetters' of uppercase letters at it's end. In case
the alloc of memory space of 'randomName' goes wrong, this function ends the program.
----------------------------------------------------------------------------------------------------------------------
*/
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
  randomName[nameSize + i] = '\0'; // Add '\0' at the end so it is considered as a string

  return randomName;
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeSemaphore
Parameters: none
Returns: nothing

Description: This function initializes the semaphore used to protect the access to the variable 'count'. Since this
program can end without closing the semaphore (in case of deadlocks), the name of the semaphore is random generated,
so it doesn't choose a name that it was previously chosen and not released.
----------------------------------------------------------------------------------------------------------------------
*/
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

/*
----------------------------------------------------------------------------------------------------------------------
Function: freeSemaphore
Parameters: none
Returns: nothing

Description: This function calls the function sem_close to free the semaphore used.
----------------------------------------------------------------------------------------------------------------------
*/
void freeSemaphore (void) {
  sem_close(semaphoreE);
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

  checkState("ThreadStarts");
  sem_wait(semaphoreE); // P(semaphoreE)
  checkState("ReadCount");
  aux = count;
  checkState("UpdateCount");
  count = aux + 1;
  printf("---Thread %d wrote %d\n", myId, count);
  
  sem_post(semaphoreE); // V(semaphoreE)

  pthread_exit(NULL);
}
