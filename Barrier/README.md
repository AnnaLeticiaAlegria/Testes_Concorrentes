# Exemplo Barrier

## Descrição

Este exemplo demonstra o funcionamento do módulo _StateManager_ no problema de sincronização da barreira. O código utilizado foi elaborado pela professora Silvana Rossetto e modificado para chamar as mesmas funções auxiliares que os outros exemplos utilizam.

Neste exemplo, as threads executam um número de passos, só podendo continuar para o próximo caso quando todas as threads tiverem executado este passo. Uma função de barreira é utilizada para realizar esta sincronização. Neste programa, existem 3 funções de barreira. As duas primeiras estão incorretas e a terceira está correta. Pelos testes realizados pelo _StateManager_, é possível verificar isso.

## Funcionamento

Para realizar a espera na barreira, as threads esperam a liberação do semáforo _cond_, que é inicializado com valor 0. Esta liberação só ocorre quando a última thread chega na função da barreira. Além disso, o programa utiliza um semáforo chamado _mutex_ para controlar o acesso das threads à variável _arrived_.

## Arquivos

### main.c

Este arquivo possui uma função _main_ e nove funções auxiliares.

Na _main_, o código trata o caso do programa ter sido executado sem ter sido passado, por linha de comando, o número da função de barreira a ser chamada (1 para a função _barrier\_v1_, 2 para a função _barrier\_v2_ e 3 para a função _barrier\_v3_), o número de threads a serem criadas, o número de passos que cada uma deve executar e o caminho para o arquivo de configuração dos estados. Além disso, são chamadas as funções de iniciar o _StateManager_, inicializar os semáforos e as threads. Quando estas terminarem a execução, as funções de liberar as threads, os semáforos e de finalizar o _StateManager_ são chamadas.

Todas as threads do programa chamam a função _threadFunction_. Nela, cada thread executa _nSteps_ vezes o código de dentro do _for_. Este código inclui o incremento da variável local para cada thread _aux1_ e a chamada da função de barreira desejada.

A função de barreira _barrier\_v1_ está incorreta. Nela, a thread adquire o semáforo _mutex_ para acessar a variável _arrived_. 
* Se ela não foi a última a chegar, ela libera este semáforo e espera no semáforo _cond_.
* Se ela foi a última a chegar, libera todas as threads que estão esperando no semáforo _cond_, atribui 0 à variável _arrived_ e libera o semáforo _mutex_.

Esta versão está incorreta. A última thread a chegar empilja várias chamadas de liberação do semáforo _cond_ e depois, libera o semáforo _mutex_. Ao fazer isto, ela consegue executar o próximo passo e passar da barreira sem que as outras threads tenham passado pelo passo anterior. Isto acarreta em problemas nos quais cada thread pode estar em um passo diferente, o que não caracteriza o intuito do problema de deixar todas as threads executarem um passo antes de irem para o próximo. Isto pode ser visto no [teste 1](#teste-1).

A função de barreira _barrier\_v2_ está incorreta. Nela, a thread adquire o semáforo _mutex_ para acessar a variável _arrived_. 
* Se ela não foi a última a chegar, ela libera este semáforo e espera no semáforo _cond_.
* Se ela foi a última a chegar, libera uma thread que esteja esperando no semáforo _cond_ e decrementa o valor da variável _arrived_ em 1.

A thread que foi liberada decrementa a variável _arrived_ e libera outra thread. Se é a última thread a ser liberada, esta libera o semáforo _mutex_.

Esta versão está errada porque o decremento da variável _arrived_ feito pela última thread a chegar está sendo feito depois de ela liberar o semáforo _cond_. Se esta thread for interrompida entre a liberação do semáforo e o decremento da variável, todas as outras threads podem executar antes que ela volte a ser executada. Neste caso, nenhuma destas threads vai entrar na condição de ser a última thread (_arrived_ == 0), não liberando o semáforo _mutex_. Isto gera um _deadlock_, como visto no [teste 2](#teste-2).

Além disso, ainda poderia haver condições de corrida na variável _arrived_, por ela poder ser modificada pela última thread que chegou na barreira ao mesmo tempo que é modificada por outra thread.

A função de barreira _barrier\_v3_ está correta. Nela, a thread adquire o semáforo _mutex_ para acessar a variável _arrived_. 
* Se ela não foi a última a chegar, ela libera este semáforo e espera no semáforo _cond_.
* Se ela foi a última a chegar, decrementa o valor da variável _arrived_ em 1 e libera uma thread que esteja esperando no semáforo _cond_.

A thread que foi liberada decrementa a variável _arrived_ e libera outra thread. Se é a última thread a ser liberada, esta libera o semáforo _mutex_.

Nesta versão, é garantido que, quando a última thread a passar da barreira checa o valor de _arrived_, ele é 0.

## Testes

Nesta pasta, há um _script_ de teste deste exemplo, assim como os arquivos de configuração utilizados por ele. Para executar este _script_, basta que o usuário modifique o caminho para as bibliotecas de Lua em C de seu computador. No script, elas estão na variável de ambiente LUA_CDIR.

Este script realiza 4 testes. Os resultados dos testes são impressos no terminal e em um arquivo _.log_ na pasta './Tests/Logs/'.

### Teste 1

Arquivo de configuração deste teste:

> ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWantsToStart 1<br/>ThreadStarts 1<br/>WantsToRelease 1<br/>ReleaseAThread 1<br/>WantsToRelease 1<br/>ReleaseAThread 1<br/>ThreadWantsToStart 1<br/>ThreadStarts 1<br/>ThreadWaits 1<br/>ThreadPassed 1<br/>ThreadWaits *<br/>ThreadPassed *<br/>ThreadWaits *<br/>ThreadPassed *<br/>

Este teste executa a função de barreira _barrier\_v1_ com 3 threads e 2 passos.

No passo 1, as duas primeiras threads executam o bloco _ThreadWantsToStart_ e _ThreadStarts_. Elas ficam esperando o evento _ThreadWaits_. Este evento não foi chamado ainda no arquivo de configuração para a liberação de cada thread pelo semáforo _cond_ ser controlada.

Também no passo 1, a terceira e última thread executa _ThreadWantsToStart_, _ThreadStarts_ e liberaria as outras 2 threads que estariam esperando no semáforo _cond_, ou seja, executa _sem\_post(cond)_ duas vezes. Isto é dado pelo par de eventos _WantsToRelease_ e _ReleaseAThread_ sendo executado duas vezes.

Esta thread libera o semáfoto _mutex_ e começa a executar o passo 2. 

Entretanto, ela entra na função da barreira e consegue executar o bloco _ThreadWantsToStart_ e _ThreadStarts_ pois o semáforo _mutex_ não estava adquirido por nenhuma thread.

Esta terceira thread consegue passar pela barreira por conta das chamadas acumuladas à função _sem\_post(cond)_ que ainda não foram consumidas pelas primeiras threads. Ou seja, a terceira thread passou da barreira no passo 2 antes que as outras threads tenham passado no passo 1. Isso é mostrado pelos eventos _ThreadWaits_ e _ThreadPassed_ sendo forçadamente executados por esta thread.

Além disso, quando as outras duas threads tentam passar pela barreira do passo 1, somente uma consegue, pois a chamada à função _sem\_post(cond)_ da segunda thread foi consumida pela terceira thread.

Isto é exemplificado pelos estados _ThreadWaits_ e _ThreadPassed_ terem sido executados uma vez, correspondendo à primeira thread passando da barreira e a segunda thread conseguindo executar _ThreadWaits_ mas nunca conseguindo executar _ThreadPassed_, causando um _deadlock_.

### Teste 2

Arquivo de configuração deste teste:

> ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadPassed *<br/>ThreadPassed *<br/>ThreadPassed *<br/>ThreadPassed *<br/>LastThreadPosts *<br/>EveryThreadArrived *<br/>

Este teste executa a função de barreira _barrier\_v2_ com 5 threads e 1 passo.

Neste teste, 4 threads executam o seguinte bloco de estados, uma seguida da outra: _ThreadWantsToStart_, _ThreadStarts_ e _ThreadWaits_. Ou seja, elas começam a execução e esperam no semáforo _cond_.

A última thread a chegar executa o bloco: _ThreadWantsToStart_ e _ThreadStarts_. Ela não para no semáforo _cond_ por ter sido a última a chegar, então não executa _ThreadWaits_. Ela libera uma thread que estava esperando no semáforo _cond_. Porém, antes de decrementar a variável _arrived_, ela espera o evento _EveryThreadArrived_.

Todas as threads que estavam esperando vão sendo liberadas e executando o estado _ThreadPassed_. Entretanto, como o decremento da variável _arrived_ ainda não foi feito pela última thread que chegou na barreira, nenhuma thread consegue executar o estado _LastThreadPosts_, porque _arrived_ não é igual a 0. O programa gera um _deadlock_.

### Teste 3

Arquivo de configuração deste teste (O mesmo do teste 2):

> ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadPassed *<br/>ThreadPassed *<br/>ThreadPassed *<br/>ThreadPassed *<br/>LastThreadPosts *<br/>EveryThreadArrived *<br/>

Este teste executa a função de barreira _barrier\_v3_ com 5 threads e 1 passo.

Neste teste, é usado o mesmo arquivo de configuração do teste anterior mas utilizando a função de barreira _barrier\_v3_, que está correta.

Assim, 4 threads executam o seguinte bloco de estados, uma seguida da outra: _ThreadWantsToStart_, _ThreadStarts_ e _ThreadWaits_. Ou seja, elas começam a execução e esperam no semáforo _cond_.

A última thread a chegar executa o bloco: _ThreadWantsToStart_ e _ThreadStarts_. Ela não para no semáforo _cond_ por ter sido a última a chegar, então não executa _ThreadWaits_. Ela libera uma thread que estava esperando no semáforo _cond_ depois de decrementar a variável _arrived_. Então, ela espera o evento _EveryThreadArrived_.

Todas as threads que estavam esperando vão sendo liberadas e executando o estado _ThreadPassed_. Como o decremento da variável _arrived_ foi feito de forma correta, a última thread a ser liberada consegue executar o estado _LastThreadPosts_. 

A última a thread a ter chegado na barreira finaliza o teste executando o estado EveryThreadArrived.

### Teste 4

Arquivo de configuração deste teste:

> ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadPassed *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadPassed *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadPassed *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadPassed *<br/>ThreadWantsToStart *<br/>ThreadStarts *<br/>ThreadWaits *<br/>ThreadPassed *<br/>LastThreadPosts *<br/>EveryThreadArrived *<br/>

Este teste executa a função de barreira _barrier\_v3_ com 5 threads e 1 passo.

Neste teste, cada thread tenta executar o seguinte bloco: _ThreadWantsToStart_, _ThreadStarts_, _ThreadWaits_ e _ThreadPassed_.

Esta ordem de estados não é uma ordem válida, já que faz com que uma thread tente passar da barreira antes de todas as threads chegarem.

Como esperado, somente os seguintes estados da primeira thread são executados: _ThreadWantsToStart_, _ThreadStarts_ e _ThreadWaits_. A thread não consegue executar o estado _ThreadPassed_ e gera um _deadlock_