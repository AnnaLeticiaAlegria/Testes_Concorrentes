/*
Module: main.c
Author: Anna Leticia Alegria
Last Modified at: 23/12/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the main file of the ReaderWriter example. In this example, there are 'nReaders'
number of readers and 'nWriters' number of writers. There is a circular buffer where writers can write and readers can
read. The rule is writers must have exclusive access to the buffer, meaning they cannot write at the same time as
another writer writes ou readers read. But, readers can read simultaneously with another reads. They cannot read if
there is a writer writing. 

There are two semaphores used in this program. The semaphore 'rw' is used to control the access to the buffer. It
controls readers and writers. The semaphore 'mutexR' is used to control the access to the variable 'activeReaders'.
This variable is accessed only on reader's code.

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/

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

int buffer;
int nReaders, nWriters;
int activeReaders = 0;

sem_t * rw ;
sem_t * mutexR;

/* Encapsulated function's declarations */

void* writeTask (void * num);
void* readTask (void * num);

/*
----------------------------------------------------------------------------------------------------------------------
Function: main
Parameters: 
  -> argc: number of parameters passed during execution
  -> argv: list of parameters passed
Returns:
  -> 0: In case of success
  -> 1: In case of error

Description: This main function receives the arguments passed by the command line. It receives the number of readers,
the number of writers and the path of the states file. It calls the functions to initialize threads, to initialize 
semaphores and to initialize the manager.
At last, the main function frees the memory spaces allocated and finalizes the manager.
----------------------------------------------------------------------------------------------------------------------
*/

int main(int argc, char** argv) 
{ 
  pthread_t * readersThreads;
  pthread_t * writersThreads;

  int ** readersIdArray = NULL;
  int ** writersIdArray = NULL;

	if (argc < 4 || argc > 5) {
    printf("Program needs 3 or 4 parameters: nReaders, nWriters, eventFileName and configFileName (optional) \n");
    return 0;
  }
  nReaders = strtol(argv[1], NULL, 10);
  nWriters = strtol(argv[2], NULL, 10);
  initializeManager (argv[3], argv[4]);

  rw = initializeSemaphore("/semRW", 1);
  mutexR = initializeSemaphore("/semActR", 1);

  readersThreads = initializeThreads (nReaders, readersIdArray, readTask);
  writersThreads = initializeThreads (nWriters, writersIdArray, writeTask);
	
  joinThreads (readersThreads, nReaders);
  joinThreads (writersThreads, nWriters);

	finalizeManager();

  freeSemaphore(rw);
  freeSemaphore(mutexR);

  freeThreads (readersThreads, nReaders, readersIdArray);
  freeThreads (writersThreads, nWriters, writersIdArray);

	return 0;
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: writeTask
Parameters: 
  -> num: pointer with thread's number
Returns: nothing

Description: This function is called by every thread that is a writer.  The writer goes into the forever while and is
ended by eventManager module, when there is no state left to be executed. The writer waits on semaphore 'rw' for their
turn no access the buffer, waiting if there is any reader reading the buffer or another writer writing on it. When the
semaphore tells the writer to go on, it can write a random element on the next position of the buffer.
----------------------------------------------------------------------------------------------------------------------
*/
void* writeTask (void * num)
{
  int id = *((int *) num);
	int element;

	while(1)
	{
    checkCurrentEvent("WriterWantsToStart");

    sem_wait(rw); //P(rw)

    checkCurrentEvent("WriterStarts");
    
    /* write the database */
    checkCurrentEvent("WriterWrites");
    element = rand()%200;
    buffer = element;
    printf("----Writer %d writes %d\n", id, element);

    checkCurrentEvent("WriterEnds");

    sem_post(rw); //V(rw)
	}
	pthread_exit(NULL); 
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: readTask
Parameters: 
  -> num: pointer with thread's number
Returns: nothing

Description: This function is called by every thread that is a reader. The reader goes into the forever while and is
ended by eventManager module, when there is no state left to be executed. Readers can read the buffer at the same
time, but they can't read the buffer if a writer is accessing it. So, the reader waits the semaphore on variable
'mutexR' to access the variable 'activeReaders' without any problem. If is the first reader to reach this part, it
gets the lock of the database. If it isn't, another reader has already gotten the lock. Then, the readers can read the
buffer. After it, if there are readers still reading, it just leaves. If it is the last reader to read, it releases
the lock so writers can write.
----------------------------------------------------------------------------------------------------------------------
*/
void* readTask (void * num)
{
	int id = *((int *) num);
	int element;

	while(1)
	{
    checkCurrentEvent("ReaderWantsToStart");

    sem_wait(mutexR); //P(mutexR)

    activeReaders ++;
    if (activeReaders == 1) { //if first, get lock
      checkCurrentEvent("FirstReader");
      sem_wait(rw); //P(rw)
    }
    sem_post(mutexR); //V(mutexR)

    checkCurrentEvent("ReaderStarts");

    /* read the database */
    checkCurrentEvent("ReaderReads");
    element = buffer;
    printf("----Reader %d reads %d\n", id, element);

    sem_wait(mutexR);
    activeReaders --;
    if (activeReaders == 0) { //if last, release lock
      checkCurrentEvent("LastReader");
      sem_post(rw); //V(rw)
    }

    checkCurrentEvent("ReaderEnds");
    
    sem_post(mutexR); //V(mutexR)
	}

	pthread_exit(NULL); 
}