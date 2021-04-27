#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#define N_READERS 2
#define N_WRITERS 1
#define BUFFER_SIZE 40


/* Variáveis globais */

int buffer [BUFFER_SIZE];
int n, m;
int active_readers = 0;

sem_t * rw ;
sem_t * mutexR;
sem_t * mutexState;

int currentState = 0;
int totalStates = 0;
char ** statesArray;

pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t conditionLock = PTHREAD_MUTEX_INITIALIZER;

/* Protótipos de funções */

int checkState (char * state);

void* writeTask (void * num);
void* readTask (void * num);

void semInit (void);
void semFree (void);

void readFile (FILE * statesFile);

/* Main */

int main() 
{ 
	pthread_t readers_thread[N_READERS];
	pthread_t writers_thread[N_WRITERS];
  int i;

  FILE * statesFile;
  statesFile = fopen("statesFile.txt", "r");
  readFile (statesFile);

	n = -1;
  m = 0;

  semInit ();
	
	for (i=0;i<N_READERS;i++) {
		pthread_create(&readers_thread[i], NULL, readTask, (void*) i);
	}
	
	for (i=0;i<N_WRITERS;i++) {
		pthread_create(&writers_thread[i], NULL, writeTask, (void*) i);
	}

	for(i=0;i<N_READERS;i++)
		pthread_join(readers_thread[i],NULL);
	
	for(i=0;i<N_WRITERS;i++)
		pthread_join(writers_thread[i],NULL);

  semFree();
  fclose(statesFile);
  
	return 0;
}

/* Funções auxiliares */

int checkState (char * state) {

  while (1) {
    pthread_mutex_lock(&conditionLock); //P(conditionLock)

    /* in case there are no states left */
    if (currentState == totalStates) {
      pthread_mutex_unlock(&conditionLock); //V(conditionLock)
      return 1;
    }

    if(!strcmp(state, statesArray[currentState])) {
      printf("%s\n", state);
      currentState ++;
      pthread_cond_broadcast(&condition);
      pthread_mutex_unlock(&conditionLock); //V(conditionLock)
      return 0;
    }

    pthread_cond_wait(&condition, &conditionLock);

    pthread_mutex_unlock(&conditionLock); //V(conditionLock)
  }
  
}


void* writeTask (void * num)
{
  int id = (int) num;
	int element;
  int shouldEnd;

	while(1)
	{
    shouldEnd = checkState("EscritorComeca");
    if(shouldEnd) {
      pthread_exit(NULL); 
    }

    sem_wait(rw); //P(rw)
    
    /* write the database */
		if (n < BUFFER_SIZE)
		{
			element = rand()%200;
			n = (n+1)%BUFFER_SIZE;
			buffer[n] = element;
      printf("Escritor escreveu %d\n", element);
		}

    shouldEnd = checkState("EscritorTermina");
    if(shouldEnd) {
      pthread_exit(NULL); 
    }

    sem_post(rw); //V(rw)
	}

	pthread_exit(NULL); 
}

void* readTask (void * num)
{
	int id = (int) num;
	int element;
  int shouldEnd;

	while(1)
	{
    shouldEnd = checkState("LeitorComeca");
    if(shouldEnd) {
      pthread_exit(NULL); 
    }

    sem_wait(mutexR); //P(mutexR)

    active_readers ++;
    if (active_readers == 1) { //if first, get lock
      sem_wait(rw); //P(rw)
    }
    sem_post(mutexR); //V(mutexR)

    /* read the database */
		if (n > 0)
		{
			element = buffer[m];
			m = (m+1)%BUFFER_SIZE;
      printf("Leitor leu %d\n", element);
		}
    sem_wait(mutexR);
    active_readers --;
    if (active_readers == 0) { //if last, release lock
      sem_post(rw); //V(rw)
    }

    shouldEnd = checkState("LeitorTermina");
    if(shouldEnd) {
      pthread_exit(NULL); 
    }
    sem_post(mutexR); //V(mutexR)
	}

	pthread_exit(NULL); 
}

void semInit (void) {
  rw = sem_open("lockDatabase6", O_CREAT, S_IRUSR | S_IWUSR, 1); 

  mutexR = sem_open("lockActiveReaders6", O_CREAT, S_IRUSR | S_IWUSR, 1); 

  mutexState = sem_open("lockState6", O_CREAT, S_IRUSR | S_IWUSR, 1); 
}

void semFree (void) {
  sem_close(rw);
  sem_close(mutexR);
  sem_close(mutexState);
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