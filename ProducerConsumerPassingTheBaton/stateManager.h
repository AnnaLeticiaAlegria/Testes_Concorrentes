#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

extern pthread_cond_t condition;
extern pthread_mutex_t conditionLock;

extern int currentState;
extern int totalStates;

void checkState (const char * state);

void initializeManager (char * fileName, int nThreads);

void finalizeManager (void);