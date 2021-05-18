#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "stateManager.h"

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

/* Protótipos de funções */

void checkState (char * state);

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
    int * num =;
		pthread_create(&readers_thread[i], NULL, readTask, (void*) num);
	}
	
	for (i=0;i<N_WRITERS;i++) {
		pthread_create(&writers_thread[i], NULL, writeTask, (void*) &i);
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

void* writeTask (void * num)
{
  int id = (int) num;
	int element;

	while(1)
	{
    checkState("EscritorQuerComecar");

    sem_wait(rw); //P(rw)

    checkState("EscritorComeca");
    
    /* write the database */
		if (n < BUFFER_SIZE)
		{
			element = rand()%200;
			n = (n+1)%BUFFER_SIZE;
			buffer[n] = element;
      printf("Escritor escreveu %d\n", element);
		}

    checkState("EscritorTermina");

    sem_post(rw); //V(rw)
	}

	pthread_exit(NULL); 
}

void* readTask (void * num)
{
	int id = (int) num;
	int element;

	while(1)
	{
    checkState("LeitorQuerComecar");

    sem_wait(mutexR); //P(mutexR)

    active_readers ++;
    if (active_readers == 1) { //if first, get lock
      sem_wait(rw); //P(rw)
    }
    sem_post(mutexR); //V(mutexR)

    checkState("LeitorComeca");

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

    checkState("LeitorTermina");
    
    sem_post(mutexR); //V(mutexR)
	}

	pthread_exit(NULL); 
}

void semInit (void) {
  rw = sem_open("lockDatabase19", O_CREAT, S_IRUSR | S_IWUSR, 1); 

  mutexR = sem_open("lockActiveReaders19", O_CREAT, S_IRUSR | S_IWUSR, 1); 

  mutexState = sem_open("lockState19", O_CREAT, S_IRUSR | S_IWUSR, 1); 
}

void semFree (void) {
  sem_close(rw);
  sem_close(mutexR);
  sem_close(mutexState);
}