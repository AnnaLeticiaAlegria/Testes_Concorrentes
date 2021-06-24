/*
Module: main.c
Author: Anna Leticia Alegria
Last Modified at: 24/06/2021

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

#include "../StateManager/stateManager.h"

#define BUFFER_SIZE 40


/* Global variable's declaration */

int buffer [BUFFER_SIZE];
int nReaders, nWriters;
int n = 0, m = 0;
int activeReaders = 0;

pthread_t * readersThreads;
pthread_t * writersThreads;

int ** idArray;

sem_t * rw ;
sem_t * mutexR;

/* Encapsulated function's declarations */

char * nameRandomize (char * name, int nLetters);
void initializeSemaphore (void);
void freeSemaphore (void);
void initializeThreads (void);
void freeThreads (void);

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
	if (argc != 4) {
    printf("Program needs 3 parameters: nReaders, nWriters and statesFileName \n");
    return 0;
  }
  nReaders = strtol(argv[1], NULL, 10);
  nWriters = strtol(argv[2], NULL, 10);
  initializeManager (argv[3], nReaders + nWriters);

  initializeSemaphore ();
  initializeThreads ();
	
	finalizeManager();
  freeSemaphore();
  freeThreads();
	return 0;
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

Description: This function initializes the semaphores used in this program. Since this program can end without closing
the semaphore (in case of deadlocks), the name of the semaphore is random generated, so it doesn't choose a name that 
it was previously chosen and not released.
----------------------------------------------------------------------------------------------------------------------
*/
void initializeSemaphore (void) {
  char * semName;

  semName = nameRandomize ("/semRW", 10);
  sem_unlink(semName);
  rw = sem_open(semName, O_CREAT, S_IRUSR | S_IWUSR, 1); 
  if (rw == SEM_FAILED){
    printf("Error opening semaphore rw\n");
    free(semName);
    exit(0);
  }
  free(semName);

  semName = nameRandomize ("/semActR", 10);
  sem_unlink(semName);
  mutexR = sem_open(semName, O_CREAT, S_IRUSR | S_IWUSR, 1); 
  if (mutexR == SEM_FAILED){
    printf("Error opening semaphore mutexR\n");
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

Description: This function calls the function sem_close to free the semaphores used.
----------------------------------------------------------------------------------------------------------------------
*/
void freeSemaphore (void) {
  sem_close(rw);
  sem_close(mutexR);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeThreads
Parameters: none
Returns: nothing

Description: This function allocates the memory space to readersThreads, to writersThreads and to idArray 
(arrays with length of '(nReaders + nWriters)') It calls the threads' functions and makes the main thread wait 
for their conclusion.
----------------------------------------------------------------------------------------------------------------------
*/
void initializeThreads () {
  int i;
  int * id;

  readersThreads = (pthread_t*) malloc (nReaders * sizeof(pthread_t));
  if (readersThreads == NULL) {
    printf("Error during readersThreads alloc\n");
  }

  writersThreads = (pthread_t*) malloc (nWriters * sizeof(pthread_t));
  if (writersThreads == NULL) {
    printf("Error during writersThreads alloc\n");
  }

  idArray = (int**) malloc ((nReaders + nWriters) * sizeof(int*));
  if (idArray == NULL) {
    printf("Error during idArray alloc\n");
  }

  for (i = 0; i < nReaders ; i++) {
    id = (int *) malloc (sizeof(int));
    if (id == NULL) {
      printf("Error during id alloc\n");
    }
    *id = i;
    idArray[i] = id;
    pthread_create(&readersThreads[i], NULL, readTask, (void*) id);
  }

  for (; (i - nReaders) < nWriters; i++) {
    id = (int *) malloc (sizeof(int));
    if (id == NULL) {
      printf("Error during id alloc\n");
    }
    *id = (i - nReaders);
    idArray[i] = id;
    pthread_create(&writersThreads[(i - nReaders)], NULL, writeTask, (void*) id);
  }

  for(i = 0; i < nReaders; i++) {
    pthread_join(readersThreads[i],NULL);
  }

  for(i = 0; i < nWriters; i++) {
    pthread_join(writersThreads[i],NULL);
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: freeThreads
Parameters: none
Returns: nothing

Description: This function frees the memory space allocated by this program
----------------------------------------------------------------------------------------------------------------------
*/
void freeThreads (void) {
  free(readersThreads);
  free(writersThreads);

  for(int i=0;i<(nReaders + nWriters);i++) {
    free(idArray[i]);
  }
  free(idArray);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: writeTask
Parameters: 
  -> num: pointer with thread's number
Returns: nothing

Description: This function is called by every thread that is a writer.  The writer goes into the forever while and is
ended by stateManager module, when there is no state left to be executed. The writer waits on semaphore 'rw' for their
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
    checkState("WriterWantsToStart");

    sem_wait(rw); //P(rw)

    checkState("WriterStarts");
    
    /* write the database */
		if (n < BUFFER_SIZE)
		{
      checkState("WriterWrites");
			element = rand()%200;
			buffer[n] = element;
			n = (n+1)%BUFFER_SIZE;
      printf("----Writer %d writes %d\n", id, element);
		}

    checkState("WriterEnds");

    sem_post(rw); //V(rw)
	}

	pthread_exit(NULL); 
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: writeTask
Parameters: 
  -> num: pointer with thread's number
Returns: nothing

Description: This function is called by every thread that is a reader. The reader goes into the forever while and is
ended by stateManager module, when there is no state left to be executed. Readers can read the buffer at the same
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
    checkState("ReaderWantsToStart");

    sem_wait(mutexR); //P(mutexR)

    activeReaders ++;
    if (activeReaders == 1) { //if first, get lock
      sem_wait(rw); //P(rw)
    }
    sem_post(mutexR); //V(mutexR)

    checkState("ReaderStarts");

    /* read the database */
		if (n > 0)
		{
      checkState("ReaderReads");
			element = buffer[m];
      printf("----Reader %d reads %d\n", id, element);
		}
    sem_wait(mutexR);
    activeReaders --;
    if (activeReaders == 0) { //if last, release lock
			m = (m+1)%BUFFER_SIZE;
      sem_post(rw); //V(rw)
    }

    checkState("ReaderEnds");
    
    sem_post(mutexR); //V(mutexR)
	}

	pthread_exit(NULL); 
}