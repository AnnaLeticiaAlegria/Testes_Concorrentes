/*
Module: main.c
Author: Anna Leticia Alegria
Last Modified at: 24/06/2021

----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
Description: This module contains the main file of the Producer Consumer Passing the Baton Example.
In this example, there are 'numProd' producers and 'numCons' consumers. Producers produce itens and insert them on the
circular buffer and consumers consume the itens from the buffer. 
In this case, a producer can only overwrite a value from the buffer if all consumers have consumed that item. If they 
cannot produce anything because the buffer is full, they keep waiting until a consumer tells them they can go on. 
A consumer can only consume an item if it exists. If the buffer is empty (or every consumer has consumed every item),
the consumer keep waiting until a producer tells them to go on.
The baton is passed is a expression to a consumer/producer getting the semaphore 'e', consuming/producing and releasing
a consumer or a producer to go on without releasing the semaphore 'e'. It is certain that there will be no other thread
accessing the buffer since they will be waiting on either semaphore 'e' or on the semphores that control the waiting
threads.
----------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

#include "../StateManager/stateManager.h"

/* Global variable's declaration */

pthread_t * threads = NULL;
int * buffer; // buffer to have itens produced or consumed
int * consumersWaiting; // vector with a flag that is 0 if this consumer is not waiting and 1 if it is. It has 'numCons' length
int * hasRead; // vector with number of itens read by a consumer. It has 'numCons' length
int * consumersToRead; // vector with number of consumers that have to read each buffer's item. It has 'numPos' length
int numPos, numProd, numCons, numItens;
int producersWaiting; // number of producers waiting to produce
int written = 0;
int ** threadConsId;
int ** threadProdId;

sem_t * e;
sem_t * semProd;
sem_t ** semCons;

/* Encapsulated function's declarations */

int * allocateIntVector (int n);
char * nameRandomize (char * name, int nLetters);

void initializeVectors (int numpos, int numprod, int numcons);
void initializeSemaphores (void);
void initializeThreads (int numprod, int numcons);
void freeVectors (void);
void freeSemaphores (void);

void * producer (void * p_Id);
void * consumer (void * p_Id);
void deposits (int item, int id);
void consumes (int myId);

/*
----------------------------------------------------------------------------------------------------------------------
Function: main
Parameters: 
  -> argc: number of parameters passed during execution
  -> argv: list of parameters passed
Returns:
  -> 0: In case of success
  -> 1: In case of error

Description: This main function receives the arguments passed by the command line. It receives the number of positions
of the buffer, the number of producers, the number of consumers, the number of itens to be produced/consumed and the 
path of the states file. It calls the functions to initialize threads, to initialize the manager and to initialize 
semaphores. At last, the main function frees the memory spaces allocated and finalizes the manager.
----------------------------------------------------------------------------------------------------------------------
*/

int main (int argc, char** argv) {
  int N, P, C, I;

  srand(time(NULL));

  if (argc != 6) {
    printf("Program needs 5 parameters: nPositions, nProducers, nConsumers, nItens and statesFileName\n");
    return 0;
  }
  N = strtol(argv[1], NULL, 10);
  P = strtol(argv[2], NULL, 10);
  C = strtol(argv[3], NULL, 10);
  I = strtol(argv[4], NULL, 10);
  initializeManager (argv[5], P + C);

  numPos = N;
  numProd = P;
  numCons = C;
  numItens = I;

  initializeVectors (N, P, C);
  
  initializeSemaphores();
  
  initializeThreads (P, C);

  freeVectors();
  freeSemaphores();
  return 0;
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: allocateIntVector
Parameters: 
  -> n: number of positions the vector must have
Returns:
  -> vector: a pointer of int that contains the vector

Description: This function allocates the memory space of an int vector with 'n' positions.
----------------------------------------------------------------------------------------------------------------------
*/
int * allocateIntVector (int n) {
  int * vector;
  vector = (int *) malloc (n * sizeof(int));
  if(vector == NULL) {
    printf("Error during vector's alloc\n");
  }

  return vector;
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: allocateIntVector
Parameters: 
  -> vector: a pointer of int that contains the vector
  -> n: number of positions of the vector
  -> num: value to be written on every position
Returns:
  -> vector: a pointer of int that contains the vector

Description: This function writes on every position of the vector the value 'num'
----------------------------------------------------------------------------------------------------------------------
*/
int * fillsVector (int * vector, int n, int num) {
  int i;

  for(i = 0 ; i < n ; i++) {
    vector[i] = num;
  }

  return vector;
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
Function: initializeVectors
Parameters: 
  -> numpos: number of positions of the buffer
  -> numprod: number producers
  -> numcons: number of consumers
Returns: nothing

Description: This function calls 'allocateVector' and 'fillsVector' passing value 0 to the arrays: 'consumerWaiting',
'hasRead', 'consumersToRead' and 'buffer'. It also allocates the memory space to the arrays 'threadConsId' and
'threadProdId'.
----------------------------------------------------------------------------------------------------------------------
*/

void initializeVectors(int numpos, int numprod, int numcons) {
  buffer = allocateIntVector(numpos);

  consumersWaiting = allocateIntVector(numcons);
  consumersWaiting = fillsVector (consumersWaiting, numcons, 0);

  hasRead = allocateIntVector(numcons);
  hasRead = fillsVector (hasRead, numcons, 0);

  consumersToRead = allocateIntVector(numpos);
  consumersToRead = fillsVector (consumersToRead, numpos, 0);

  threadConsId = (int **) malloc (numcons * sizeof(int*));
  if(threadConsId == NULL) {
    printf("Error during threadConsId's alloc\n");
  }
  
  threadProdId = (int **) malloc (numprod * sizeof(int*));
  if(threadProdId == NULL) {
    printf("Error during threadProdId's alloc\n");
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeSemaphore
Parameters: none
Returns: nothing

Description: This function initializes the semaphore used to protect the access to the global variables. Since this
program can end without closing the semaphores (in case of deadlocks), the name of the semaphore is random generated,
so it doesn't choose a name that it was previously chosen and not released.
----------------------------------------------------------------------------------------------------------------------
*/
void initializeSemaphores (void) {
  char * semaphoreName;
  int numberOfLetters;

  numberOfLetters = (rand()%20) + 1;

  semaphoreName = nameRandomize ("/semmE", numberOfLetters);
  sem_unlink(semaphoreName);
  e = sem_open(semaphoreName, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 1);
  if (e == SEM_FAILED){
    printf("Error opening semaphore e. Name: %s\n", semaphoreName);
    free(semaphoreName);
    exit(0);
  }
  free(semaphoreName);

  semaphoreName = nameRandomize ("/semmP", numberOfLetters);
  sem_unlink(semaphoreName);
  semProd = sem_open(semaphoreName, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0); 
  if (semProd == SEM_FAILED){
    printf("Error opening semaphore semProd. Name: %s\n", semaphoreName);
    free(semaphoreName);
    exit(0);
  }
  free(semaphoreName);

  semCons = (sem_t **) malloc (numCons * sizeof(sem_t*));
  for (int i = 0; i< numCons; i++) {
    semaphoreName = nameRandomize ("/semmC", numberOfLetters + i);
    sem_unlink(semaphoreName);
    semCons[i] = sem_open(semaphoreName, O_CREAT|O_EXCL, S_IRUSR | S_IWUSR, 0); 
    if (semCons[i] == SEM_FAILED){
      printf("Error opening semaphore semCons[%d]. Name: %s\n", i, semaphoreName);
      free(semaphoreName);
      exit(0);
    }
    free(semaphoreName);
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: initializeThreads
Parameters: 
  -> numprod: number of threads to be Producers
  -> numprod: number of threads to be Consumers
Returns: nothing

Description: This function allocates the memory space to 'threads'.
It calls the thread's functions and make the main thread wait for their conclusion.
----------------------------------------------------------------------------------------------------------------------
*/

void initializeThreads (int numprod, int numcons) {
  int i;
  int * id;

  threads = (pthread_t*) malloc ((numprod + numcons) * sizeof(pthread_t));
  if (threads == NULL) {
    printf("Error during thread's alloc\n");
  }

  for (i = 0; i < numprod ; i++) {
    id = (int *) malloc (sizeof(int));
    *id = i;
    threadProdId[i] = id;
    pthread_create(&threads[i], NULL, producer, (void*) id);
  }

  for (i = 0; i < numcons ; i++) {
    id = (int *) malloc (sizeof(int));
    *id = i;
    threadConsId[i] = id;
    pthread_create(&threads[i + numprod], NULL, consumer, (void*) id);
  }

  for(i = 0; i < (numcons + numprod); i++) {
    pthread_join(threads[i],NULL);
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
void freeVectors (void) {
  free(threads);
  free(buffer);
  free(consumersWaiting);
  free(hasRead);
  free(consumersToRead);
  for(int i=0;i<numCons;i++) {
    free(threadConsId[i]);
  }
  for(int i=0;i<numProd;i++) {
    free(threadProdId[i]);
  }
  free(threadConsId);
  free(threadProdId);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: freeSemaphore
Parameters: none
Returns: nothing

Description: This function calls the function sem_close to free the semaphores used.
----------------------------------------------------------------------------------------------------------------------
*/
void freeSemaphores (void) {
  sem_close(e);
  sem_close(semProd);
  for (int i = 0; i< numCons; i++) { 
    sem_close(semCons[i]);
  }
  free(semCons);
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: producer
Parameters: 
  -> p_Id: pointer with thread's number
Returns: nothing

Description: This function is called by every producer. It generates a random item to be placed on the buffer. It
calls the function 'deposits' to access the buffer.
----------------------------------------------------------------------------------------------------------------------
*/

void * producer (void * p_Id) {
  int item;
  int id = *((int *) p_Id);

  while(1) {
    item = rand() % 200;
    deposits(item, id);
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: consumer
Parameters: 
  -> p_Id: pointer with thread's number
Returns: nothing

Description: This function is called by every consumer. It calls the function 'consumes'.
----------------------------------------------------------------------------------------------------------------------
*/

void * consumer (void * p_Id) {
  int id = *((int *) p_Id);

  while(1) {
    consumes (id);
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: deposits
Parameters: 
  -> item: item to be deposited on the buffer
  -> id: id of the producer
Returns: nothing

Description: This function is called by every producer. First, the producer gets the semaphore 'e'. It calculates
the position the item will be written. 

- If a consumer didn't consumed the item on this position yet, the producer
increments the 'producersWaiting', releases the semaphore 'e' and keeps waiting on semaphore 'semProd'. When a
producer or a consumer releases this producer of the waiting, they had the semaphore 'e' lock and didnt' release,
passing the baton to this producer go on.
- If every consumer has consumed the item, the producer just goes on.

The producer checks if it has already produced the total number of itens. If there are still itens to be produced,
the producer produces an item, putting 'numCons' on the position referent to the item's position on 'consumersToRead'.
When this variable reaches 0, it means every consumer has read the item.

Now, the producer checks if there are any consumer waiting. 

- If there are, it passes the baton to the first one that is waiting. If every item has been produced, the producer 
ends. At last, the producer returns.
- If there aren't, it checks if every consumer has read the next position and if there is a producer waiting to produce.

  - If there is, it passes the baton to the producer.
  - If there isn't, it releases the semaphore 'e', not passing the baton to anyone.
----------------------------------------------------------------------------------------------------------------------
*/

void deposits (int item, int id) {
  int pos, flagEnds;

  checkState("ProducerWantsToStart");
  sem_wait(e); //P(e)
  checkState("ProducerStarts");
  pos = written % numPos;

  if (consumersToRead[pos] > 0) { //Checks if all consumers have read this position
    producersWaiting ++;
    sem_post(e); //V(e)

    checkState("ProducerWaits");
    sem_wait(semProd); //P(semProd)
  }

  flagEnds = (written >= numItens) ? 1 : 0;

  if (!flagEnds) {
    checkState("ProducerProduces");
    pos = written % numPos;
    buffer[pos] = item;
    printf("---producer %d wrote %d\n", id, item);


    consumersToRead[pos] = numCons; //Number of consumers to consume the item at this position
    written ++;
  }

  flagEnds = (written >= numItens) ? 1 : 0;

  //signal --> releases all consumers
  for (int c = 0; c < numCons; c++) {
    if (consumersWaiting[c] && written > hasRead[c]) {
      consumersWaiting[c] = 0;
      sem_post(semCons[c]); //V(semCons)
      if (flagEnds) {
        printf("---producer %d ending\n", id);
        sem_post(e); //V(e)
        sem_post(semProd); 
        checkState("ProducerEnds");
        pthread_exit(NULL);
      }
      return;
    }
  }

  if(consumersToRead[written%numPos] == 0 && producersWaiting > 0) { //If all consumers have read and there is a producer waiting
    producersWaiting --;
    sem_post(semProd); 
  }
  else {
    sem_post(e); //V(e)
  }
  if (flagEnds) {
    checkState("ProducerEnds");
    printf("---producer %d ending\n", id);
    sem_post(semProd); 
    pthread_exit(NULL);
  }
}

/*
----------------------------------------------------------------------------------------------------------------------
Function: consumes
Parameters: 
  -> myId: id of the consumer
Returns: nothing

Description: This function is called by every consumer. First, the consumer gets the semaphore 'e'. It checks if there
is anything to read (if the number of items read by this consumer is lower than the number of total written items).

- If there is, it goes on
- If there isn't anything to read, it releases the semaphore 'e'. The consumer keeps waiting on semaphore
'semCons[myId]'. When a producer or a consumer releases this consumer of the waiting, they had the semaphore 'e' lock 
and didnt' release, passing the baton to this consumer go on.

Next, the consumer checks if it has read every item. If it hasn't, it can read the item at the next position of the
last item this consumer has read.

Now, the consumer checks if there are any consumer waiting. 

- If there are, it passes the baton to the first one that is waiting. If every item has been read, the consumer 
ends. At last, the consumer returns.
- If there aren't, it checks if every consumer has read the next position and if there is a producer waiting to produce.

  - If there is, it passes the baton to the producer.
  - If there isn't, it releases the semaphore 'e', not passing the baton to anyone.
----------------------------------------------------------------------------------------------------------------------
*/
void consumes (int myId) { 
  int item, pos;
  int flagEnds;

  pos = hasRead[myId] % numPos;
  checkState("ConsumerWantsToStart");
  sem_wait(e); //P(e)
  checkState("ConsumerStarts");

  if(written <= hasRead[myId]) { //check if there is anything to read
    consumersWaiting[myId] = 1;
    sem_post(e); //V(e)

    checkState("ConsumerWaits");
    sem_wait(semCons[myId]); //P(semCons) ---> waits until producer releases it to read
  }

  flagEnds = (hasRead[myId] >= numItens) ? 1 : 0;

  if (!flagEnds) {
    checkState("ConsumerConsumes");
    pos = hasRead[myId] % numPos;
    item = buffer[pos];

    printf("---consumer %d consumes %d\n", myId, item);

    consumersToRead[pos] --;
    hasRead[myId] ++;
  }

  flagEnds = (written >= numItens) ? 1 : 0;

  //signal
  for (int c = 0; c < numCons; c++) {
    if (consumersWaiting[c] && written > hasRead[c]) {
      consumersWaiting[c] = 0;
      if (flagEnds) {
        checkState("ConsumerEnds");
        printf("---consumer %d ending\n", myId);
        sem_post(e);
        pthread_exit(NULL);
      }
      sem_post(semCons[c]); //V(semCons)
      return;
    }
  }

  if(consumersToRead[written%numPos] == 0 && producersWaiting > 0) { //if all consumers have read and there is a producer waiting
    producersWaiting --;
    sem_post(semProd); 
  }
  else {
    sem_post(e); //V(e)
  }

  if (flagEnds) {
    checkState("ConsumerEnds");
    printf("---consumer %d finalizando\n", myId);
    pthread_exit(NULL);
  }

  return;
}