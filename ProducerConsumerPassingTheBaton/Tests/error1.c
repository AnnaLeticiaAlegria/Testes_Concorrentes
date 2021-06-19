/*
Erro: vetor de lidos e vetor de faltaLer com tamanho de numCons ao invés de numPos
Erro: checa se tem que terminar depois de se colocar em espera (alguns podem ficar em espera eterna)
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>

/* Variáveis globais */

pthread_t * threads = NULL;
int * buffer;
int * consEsperando;
int * lidos;
int * faltaLer;
int numPos, numProd, numCons, numItens;
int prodEsperando;
int escritos = 0;
int ** threadConsId;
int ** threadProdId;

sem_t * e;
sem_t * semProd;
sem_t ** semCons;

/* Protótipos de funções */

void inicializaVetores (int numpos, int numprod, int numcons);
void iniciaSemaforos (void);
void iniciaThreads (int numprod, int numcons);
void * produtor (void * p_Id);
void * consumidor (void * p_Id);
void deposita (int item, int id);
int consome (int meuid);
void liberaVetores (void);
void liberaSemaforos (void);

/* Main */

int main (int argc, char** argv) {
  int N, P, C, I;
  int i;

  if (argc != 5) {
    printf("Parâmetros passados incorretamente! (N P C I)\n");
    return 0;
  }
  N = strtol(argv[1], NULL, 10);
  P = strtol(argv[2], NULL, 10);
  C = strtol(argv[3], NULL, 10);
  I = strtol(argv[4], NULL, 10);

  numPos = N;
  numProd = P;
  numCons = C;
  numItens = I;

  inicializaVetores (N, P, C);
  
  iniciaSemaforos();
  
  iniciaThreads (P, C);

  for(i = 0; i < P + C; i++) {
    pthread_join(threads[i],NULL);
  }

  liberaVetores();
  liberaSemaforos();
  return 0;
}

/* Funções auxiliares */
int * alocaVetorInteiro (int n) {
  int * vet;
  vet = (int *) malloc (n * sizeof(int));
  if(vet == NULL) {
    printf("Erro na alocação de memória do vetor\n");
  }

  return vet;
}

void preencheVetor (int * vetor, int n, int num) {
  int i;

  for(i = 0 ; i < n ; i++) {
    vetor[i] = num;
  }
}

char * randomizaNome (char * nome, int nLetras) {
  int i, tamNome;
  char * nomeRandomizado;
  tamNome = strlen(nome);
  nomeRandomizado = (char *) malloc ((tamNome + nLetras + 1) * sizeof(char));
  if (nomeRandomizado == NULL) {
    printf("Erro na alocacao do nome randomizado\n");
    exit(0);
  }

  strcpy(nomeRandomizado, nome);
  for(i=0;i<nLetras;i++){
    srand(time(NULL));
    nomeRandomizado[tamNome + i] = 'A' + (char) rand()%46;
  }
  nomeRandomizado[i] = '\0';
  return nomeRandomizado;
}

/* Funções */

void inicializaVetores(int numpos, int numprod, int numcons) {
  buffer = alocaVetorInteiro(numpos);

  consEsperando = alocaVetorInteiro(numcons);
  preencheVetor (consEsperando, numcons, 0);

  lidos = alocaVetorInteiro(numcons);
  preencheVetor (lidos, numcons, 0);

  faltaLer = alocaVetorInteiro(numpos);
  preencheVetor (faltaLer, numcons, 0);

  threadConsId = (int **) malloc (numcons * sizeof(int*));
  threadProdId = (int **) malloc (numprod * sizeof(int*));
}

void iniciaSemaforos (void) {
  char * nomeSemaforo;

  nomeSemaforo = randomizaNome ("semEntrada", 5);
  e = sem_open(nomeSemaforo, O_CREAT, S_IRUSR | S_IWUSR, 1); 
  free(nomeSemaforo);
  //semaforo e criado como um named semaphore, com permissão de escrita e leitura, inicializado com valor 1

  nomeSemaforo = randomizaNome ("semProdutor", 5);
  semProd = sem_open(nomeSemaforo, O_CREAT, S_IRUSR | S_IWUSR, 0); 
  free(nomeSemaforo);

  semCons = (sem_t **) malloc (numCons * sizeof(sem_t*));
  for (int i = 0; i< numCons; i++) {
    nomeSemaforo = randomizaNome ("vetorSemConsumidor", 5 + i);
    semCons[i] = sem_open(nomeSemaforo, O_CREAT, S_IRUSR | S_IWUSR, 0); 
    free(nomeSemaforo);
    //controla o acesso dos consumidores. Inicializado com valor 0
  }
}

void iniciaThreads (int numprod, int numcons) {
  int i;
  int * id;

  threads = (pthread_t*) malloc ((numprod + numcons) * sizeof(pthread_t));
  if (threads == NULL) {
    printf("Erro na alocação das threads\n");
  }

  for (i = 0; i < numprod ; i++) {
    id = (int *) malloc (sizeof(int));
    *id = i;
    threadProdId[i] = id;
    pthread_create(&threads[i], NULL, produtor, (void*) id);
  }

  for (i = 0; i < numcons ; i++) {
    id = (int *) malloc (sizeof(int));
    *id = i;
    threadConsId[i] = id;
    pthread_create(&threads[i + numprod], NULL, consumidor, (void*) id);
  }
}


void * produtor (void * p_Id) {
  int item;
  int id = *((int *) p_Id);
  printf("Produtor %d iniciando\n", id);

  while(1) {
    item = rand() % 200;
    deposita(item, id);
  }
}

void * consumidor (void * p_Id) {
  int item;
  int id = *((int *) p_Id);
  printf("Consumidor %d iniciando\n", id);

  while(1) {
    item = consome (id);
    if (item >= 0) {
      printf("Consumidor %d consumiu %d\n", id, item);
    }
  }
}

void deposita (int item, int id) {
  int pos, flagTermina;
  sem_wait(e); //P(e)

  pos = escritos % numPos;
  if (faltaLer[pos] > 0) { //checa se todos os consumidores já leram aquela posição
    prodEsperando ++;
    sem_post(e); //V(e)

    sem_wait(semProd); //P(semProd)
    //sem_wait(e); //P(e) --------------------- precisa pegar novamente o lock? senão pode haver conc entre produtores?
  }

  flagTermina = (escritos >= numItens) ? 1 : 0;
  if (!flagTermina) { //só escreve se não ultrapassar a quantidade de itens.
  // Não coloquei para retornar aqui caso a quantidade de itens tenha sido superada pois as linhas abaixo são de liberação das outras threads
    buffer[pos] = item;
    printf("Produtor %d escreveu %d\n", id, item);
  }

  faltaLer[pos] = numCons; //atribui com o número de consumidores a consumirem o item daquela posição
  escritos ++;

  //signal --> libera todos os consumidores
  for (int c = 0; c < numCons; c++) {
    if (consEsperando[c] && escritos > lidos[c]) {
      consEsperando[c] = 0;
      sem_post(semCons[c]); //V(semCons)
      return;
    }
  }

  if(faltaLer[escritos%numPos] == 0 && prodEsperando > 0) { //se todos os consumidores já leram e tem algum produtor esperando
    prodEsperando --;
    sem_post(semProd); 
  }
  else {
    sem_post(e); //V(e)
  }
  if (flagTermina) {
    printf("Produtor %d finalizando\n", id);
    pthread_exit(NULL);
  }
}

int consome (int meuid) { 
  int item, pos;
  int flagTermina;

  pos = lidos[meuid] % numPos;
  sem_wait(e); //P(e)
  if(escritos <= lidos[meuid]) { //checa se tem algo para ler
    consEsperando[meuid] = 1;
    sem_post(e); //V(e)
    sem_wait(semCons[meuid]); //P(semCons) ---> espera produtor liberar para ler
    //return -1; //retorna para conferir se já leu todos
  }

  flagTermina = (lidos[meuid] >= numItens) ? 1 : 0;
  if(!flagTermina) {
    item = buffer[pos];
  }

  faltaLer[pos] --; //indica ao produtor que um consumidor já leu dessa posição
  lidos[meuid] ++; //número de itens que esse consumidor leu
  sem_post(e); //V(e)

  if (flagTermina) {
    printf("Consumidor %d finalizando\n", meuid);
    pthread_exit(NULL);
  }

  return item;
}

void liberaVetores (void) {
  free(threads);
  free(buffer);
  free(consEsperando);
  free(lidos);
  free(faltaLer);
  for(int i=0;i<numCons;i++) {
    free(threadConsId[i]);
  }
  for(int i=0;i<numProd;i++) {
    free(threadProdId[i]);
  }
  free(threadConsId);
  free(threadProdId);
}

void liberaSemaforos (void) {
  sem_close(e);
  sem_close(semProd);
  for (int i = 0; i< numCons; i++) { 
    sem_close(semCons[i]);
  }
  free(semCons);
}