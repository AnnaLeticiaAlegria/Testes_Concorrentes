#include "concurrency.h"

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
Parameters: 
    -> name: semaphore's name
Returns:
    -> newSem: the created semaphore

Description: This function initializes the semaphore with the name passed as an argument. Since the program can end 
without closing the semaphore (in case of deadlocks), the name of the semaphore is random generated, so it doesn't 
choose a name that it was previously chosen and not released.
----------------------------------------------------------------------------------------------------------------------
*/
sem_t * initializeSemaphore (char * name, int semValue) {
  sem_t * newSem;
  char * semName;

  semName = nameRandomize (name, 10);
  sem_unlink(semName);
  newSem = sem_open(semName, O_CREAT, S_IRUSR | S_IWUSR, semValue); 
  if (newSem == SEM_FAILED){
    printf("Error opening semaphore %s\n", name);
    free(semName);
    exit(0);
  }
  free(semName);

  return newSem;
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: freeSemaphore
Parameters:
    -> sem: semaphore to be released
Returns: nothing

Description: This function calls the function sem_close to free the semaphore sem.
----------------------------------------------------------------------------------------------------------------------
*/
void freeSemaphore (sem_t * sem) {
  sem_close(sem);
}


/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeThreads
Parameters:
    -> nThreads: number of threads
    -> idArray: array of threads id's
Returns:
    -> threadArray: array with threads

Description: This function allocates the memory space to threads'array and to idArray 
(arrays with length of nThreads) It calls the threads' functions and makes the main thread wait for their conclusion.
----------------------------------------------------------------------------------------------------------------------
*/
pthread_t * initializeThreads (int nThreads, int ** idArray, void* threadFunction (void *)) {
  int i;
  int * id;
  pthread_t * threadArray;

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

  return threadArray;
}


/*
----------------------------------------------------------------------------------------------------------------------
Function: joinThreads
Parameters:
    -> threadArray: array of threads
    -> nThreads: number of threads
Returns: nothing

Description: This function makes the main thread wait for the created threads
----------------------------------------------------------------------------------------------------------------------
*/
void joinThreads (pthread_t * threadArray, int nThreads) {
  for(int i = 0; i < nThreads; i++) {
    pthread_join(threadArray[i],NULL);
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: freeThreads
Parameters:
    -> threadArray: array of threads to be released
    -> nThreads: number of threads allocated
    -> idArray: array with de threads'ids
Returns: nothing

Description: This function frees the memory space allocated by the threads initialization
----------------------------------------------------------------------------------------------------------------------
*/
void freeThreads (pthread_t * threadArray, int nThreads, int ** idArray) {
  free(threadArray);

  for(int i=0;i<nThreads;i++) {
    free(idArray[i]);
  }
  free(idArray);
}