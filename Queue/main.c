#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "../EventManager/eventManager.h"
#include "../ConcurrencyModule/concurrency.h"


typedef struct queue {
  int * buffer;
  int count;
  int head;
} Queue;

/* Global variable's declaration */

Queue * mainQueue;
int nThreads;
int nDequeuers, nEnqueuers;
int bufferSize;

pthread_cond_t waitCondition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t waitLock = PTHREAD_MUTEX_INITIALIZER;

/* Encapsulated function's declarations */

Queue * createQueue (void) ;
void deleteQueue(Queue * queue);

int tail (Queue * queue);
int isEmpty (Queue * queue);
int isFull (Queue * queue);

void* threadFunction (void * num);

void enqueue (int element);
int dequeue (void);

int main(int argc, char** argv) 
{ 
  pthread_t * threadsArray;

  int ** threadsIdArray = NULL;

	if (argc < 5 && argc > 6) {
    printf("Program needs 4 or 5 parameters: bufferSize, nEnqueuers, nDequeuers, eventFileName and configFileName(optional) \n");
    return 0;
  }
  bufferSize = strtol(argv[1], NULL, 10);
  nEnqueuers = strtol(argv[2], NULL, 10);
  nDequeuers = strtol(argv[3], NULL, 10);
  initializeManager (argv[4], argv[5]);
  nThreads = nEnqueuers + nDequeuers;

  mainQueue = createQueue ();

  threadsArray = initializeThreads (nThreads, threadsIdArray, threadFunction);
	
  joinThreads (threadsArray, nThreads);

	finalizeManager();

  deleteQueue(mainQueue);

  // freeThreads (threadsArray, nThreads, threadsIdArray);

	return 0;
}

Queue * createQueue (void) 
{
  Queue * newQueue = (Queue*) malloc (sizeof(Queue));
  if (newQueue == NULL) {
    printf("Error allocating queue space\n");
    exit(0);
  }

  newQueue->buffer = (int*) malloc (bufferSize * sizeof(int));
  if (newQueue->buffer == NULL) {
    printf("Error allocating buffer space\n");
    exit(0);
  }

  newQueue->count = 0;
  newQueue->head = 0;

  return newQueue;
}

void deleteQueue(Queue * queue) {
  if (queue != NULL) {
    free(queue->buffer);
    free(queue);
  }
}

int tail (Queue * queue) {
  return (queue->head + queue->count) % bufferSize;
}

int isEmpty (Queue * queue) {
  if (queue->count == 0) {
    return 1;
  }
  return 0;
}

int isFull (Queue * queue) {
  if (queue->count == bufferSize) {
    return 1;
  }
  return 0;
}

void* threadFunction (void * num) {
  int id = *((int *) num);
  int element;

	if(id < nEnqueuers) {
    element = rand() % 100;
    enqueue (element);
    printf("----------Thread %d enqueued element %d\n", id, element);
  }
  else {
    element = dequeue ();
    printf("----------Thread %d dequeued element %d\n", id, element);
  }
	pthread_exit(NULL); 
}

void enqueue (int element) {

  checkCurrentEvent("EnqueueWantsToStart");
  pthread_mutex_lock(&waitLock);
  checkCurrentEvent("EnqueueStarts");

  while (isFull(mainQueue)) {
    checkCurrentEvent("EnqueueWaits");
    pthread_cond_wait(&waitCondition, &waitLock);

    pthread_mutex_unlock(&waitLock);
    checkCurrentEvent("EnqueueContinues");
    pthread_mutex_lock(&waitLock);
  }

  checkCurrentEvent("EnqueueEnqueues");
  mainQueue->buffer[tail(mainQueue)] = element;
  (mainQueue->count) ++;

  checkCurrentEvent("EnqueueBroadcast");
  pthread_cond_broadcast(&waitCondition);

  checkCurrentEvent("EnqueueEnds");
  pthread_mutex_unlock(&waitLock);
}

int dequeue (void) {
  int element;
  
  checkCurrentEvent("DequeueWantsToStart");
  pthread_mutex_lock(&waitLock);
  checkCurrentEvent("DequeueStarts");

  while (isEmpty(mainQueue)) {
    checkCurrentEvent("DequeueWaits");
    pthread_cond_wait(&waitCondition, &waitLock);

    pthread_mutex_unlock(&waitLock);
    checkCurrentEvent("DequeueContinues");
    pthread_mutex_lock(&waitLock);
  }

  checkCurrentEvent("DequeueDequeues");
  element = mainQueue->buffer[mainQueue->head];
  mainQueue->head = (mainQueue->head + 1) % bufferSize;
  (mainQueue->count) --;

  checkCurrentEvent("DequeueBroadcast");
  pthread_cond_broadcast(&waitCondition);

  checkCurrentEvent("DequeueEnds");
  pthread_mutex_unlock(&waitLock);

  return element;
}