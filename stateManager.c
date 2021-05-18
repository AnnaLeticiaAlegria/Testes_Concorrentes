#include "stateManager.h"

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t conditionLock = PTHREAD_MUTEX_INITIALIZER;

int currentState = 0;
int totalStates = 0;
char ** statesArray;

void checkState (char * state) {

  while (1) {
    pthread_mutex_lock(&conditionLock); //P(conditionLock)

    /* in case there are no states left */
    if (currentState == totalStates) {
      pthread_mutex_unlock(&conditionLock); //V(conditionLock)
      pthread_exit(NULL); //end thread
    }

    if(!strcmp(state, statesArray[currentState])) {
      printf("%s\n", state);
      currentState ++;
      pthread_cond_broadcast(&condition);
      pthread_mutex_unlock(&conditionLock); //V(conditionLock)
      return;
    }

    pthread_cond_wait(&condition, &conditionLock);

    pthread_mutex_unlock(&conditionLock); //V(conditionLock)
  }
  
}

void readFile (FILE * statesFile) {
  int count = 0;
  char state [80];

  while (fscanf(statesFile, " %[^\n]", state) == 1) {
    totalStates ++;
  }

  statesArray = (char**) malloc (totalStates * sizeof(char*));
  if (statesArray == NULL) {
    printf("Erro na alocação do vetor\n");
    exit(0);
  }

  rewind(statesFile); //reset pointer of the file

  while (fscanf(statesFile, " %[^\n]", state) == 1) {
    statesArray[count] = (char*) malloc ((strlen(state) + 1) * sizeof(char*));
    if (statesArray[count] == NULL) {
      printf("Erro na alocação da entrada do vetor\n");
      exit(0);
    }

    strcpy(statesArray[count], state);
    count ++;
  }
}