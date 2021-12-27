#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>

char * nameRandomize (char * name, int nLetters);

sem_t * initializeSemaphore (char * name, int semValue);

void freeSemaphore (sem_t * sem);

pthread_t * initializeThreads (int nThreads, int ** idArray, void* threadFunction (void *));

void joinThreads (pthread_t * threadArray, int nThreads);

void freeThreads (pthread_t * threadArray, int nThreads, int ** idArray);