# Módulo State Manager

## Descrição

Este módulo contém o código do projeto 'Determinando ordens de execução para testes de programas concorrentes'.
Ele permite que o usuário defina estados (ou eventos) em seu programa concorrente e a ordem na qual estes estados devem ser executados. O usuário pode definir se deseja que dois ou mais estados sejam executados pela mesma thread ou se não podem ser executados por uma thread específica. Para entender como escrever o arquivo de ordem de estados, também chamado de arquivo de configuração, [clique aqui](../README.md#como-escrever-o-arquivo-de-configuração).
É importante ressaltar que o programa do usuário precisa implementar a concorrência utilizando a biblioteca pthread em C.
Este módulo foi implementado em alguns programas reais. Para saber mais sobre estes exemplos, [clique aqui](../README.md#exemplos-de-uso).

## Funcionamento

Este módulo controla as threads criadas pelo usuário de acordo com os estados especificados no arquivo de ordem de estados. Para saber como utilizar o módulo _State Manager_, veja a seção [Como utilizar o módulo StateManager](../README.md#como-utilizar-o-módulo-statemanager).
Os estados especificados no arquivo devem estar ao longo do código do usuário em chamadas para a função _checkState_. Nesta função, o programa checa se é a vez daquela thread executar aquele estado ou não. 
  * Caso seja sua vez, o programa permite que a thread continue e imprime no terminal o estado que ela executou. Em seguida, essa thread envia um sinal por _broadcast_ acordando todas as outras threads que estavam em espera. 
  * Caso não seja a vez daquela thread ou daquele thread, ela fica em espera até que outra thread sinalize que ela pode continuar.

Percebe-se que, caso o usuário coloque uma ordem de estados que não é válida pelo seu programa, nenhuma thread vai satisfazer as condições de ser sua vez. Então, todas ficarão em espera eterna, ocasionando em um _deadlock_. Quando nenhuma thread executa um estado por mais de um determinado tempo (configurado no código pela variável _deadLockDetectTime_, cujo valor padrão é 5 segundos), o programa considera que uma situação de _deadlock_ ocorreu, imprime na tela qual seria o próximo estado e encerra. Discussões acerca do que uma situação dessas representa podem ser encontradas [aqui](../README.md#sobre-os-resultados).

A seguir, uma breve descrição dos componentes do módulo _StateManager_. Estes e outros detalhes podem ser encontrados como comentários no código, também.

## Arquivos do módulo

### stateManager.h

Contém as declarações das funções e variáveis exportadas pelo módulo. Nele, o usuário pode modificar o caminho para o arquivo readStates.lua, caso esteja utilizando o módulo com uma organização de diretórios diferente da padrão:

. <br/>
|---Diretório externo <br/>
| |---Pasta com programa do usuário <br/>
| | |---main.c <br/>
| | |---statesFile.txt <br/>
| |---StateManager <br/>
| | |---stateManager.c <br/>
| | |---stateManager.h <br/>
| | |---readStates.lua <br/>


Além disso, o usuário também pode modificar o tempo a partir do qual o programa considera que houve um _deadlock_. Basta modificar a variável _deadLockDetectTime_ para algum número inteiro que represente este tempo em segundos.

Neste _header_, também se encontram as declarações das funções _initializeManager_, _checkState_ e _finalizeManager_. Elas serão discutidas no tópico a seguir.

### stateManager.c

Este arquivo contém os códigos das funções declaradas no _header_ e de outras funções encapsuladas pelo módulo.

A variável global _conditionLock_ é utilizada para controlar o acesso das threads à variável global _currentState_. Já a _condition_ é a variável que controla as threads do programa do usuário, mantendo-as em espera enquanto não for o turno delas e permitindo que a thread que está executando seu turno as notifique via _broadcast_.

A variável _currentState_ armazena a posição do vetor em que se encontra qual o nome do próximo estado a ser executado e qual o seu _id_. As threads acessam essa variável para saber se é a vez delas de executarem. Em _totalStates_, fica armazenada a quantidade total de estados do vetor de estados.

Os vetores _statesArray_, _statesIdArray_ e _threadIdArray_ são alocados e preenchidos na função _initializeManager_. O primeiro vetor contém os nomes dos estados na ordem definida pelo arquivo de estados criado pelo usuário. O segundo contém os _ids_ dos estados, sendo cada posição dele correspondente ao estado definido em _statesArray_. Por último, _threadIdArray_ contém os _ids_ das threads do usuário. Mais detalhes acerca dos _ids_ dos estados e das threads são discutidos mais à frente.

A função _initializeManager_ recebe o caminho para o arquivo de estados e a quantidade de threads criadas pelo usuário. Esta função chama a função em Lua definida no arquivo readStates.lua, explorado no próximo tópico. Na _initilizeManager_ é feito o preparo da pilha de execução para a chamada da função em Lua, mandando o caminho do arquivo. A função retorna, colocando na pilha, a ordem dos estados, dos _ids_ dos estados e a quantidade total de estados. Assim, os vetores globais correspondentes são preenchidos com essas informações. A seguir, a _initilizeManager_ aloca o espaço do vetor de _ids_ das threads e o preenche com zero em todas as posições. Por último, esta função configura um alarme com o tempo de _deadLockDetectTime_. Caso este alarme seja ativado, significa que o primeiro estado já não conseguiu ser executado e levou a um _deadlock_

A função _checkState_ recebe o nome do estado que a thread deseja executar. Esta função confere se é a vez desta thread e deste estado, chamando a função _compareState_. 
  * Caso a função retorne 1, a thread imprime o nome do estado que ela está executando, acorda as threads que estejam dormindo e configura um novo alarme com o tempo de _deadLockDetectTime_. Como só pode haver um alarme ativo por vez, qualquer alarme configurado anteriormente é apagado. Assim, é garantido que este alarme só é ativado caso algum estado não tenha conseguido ser executado dentro do tempo especificado. Após isso, a thread retorna para executar os comandos do programa do usuário.
  * Caso a função retorne 0, a thread espera até que outra a permita continuar. Quando ela continuar, vai checar novamente se é a sua vez de ser executada.

Na função _finalizeManager_, o programa libera os espaços alocados por ele e cancela qualquer alarme que poderia estar ativo.

A função _getLuaResults_ é uma função auxiliar para retirar da pilha de execução os resultados da chamada da função em Lua.

Para seu funcionamento, a função _compareStates_ precisa receber o nome do estado que deseja executar e a posição do estado atual. O usuário pode definir no arquivo de ordem de estados se deseja que uma thread específica execute uma sequência de estados, se quer que algum estado seja executado por qualquer thread menos por alguma em específico ou se aquele estado pode ser executado por qualquer thread (Mais detalhes podem ser encontrados em [Como escrever o arquivo de configuração](../README.md#como-escrever-o-arquivo-de-configuração).). Assim, essa função precisa checar, além do nome, o _id_ do estado:
  * Caso o estado possua _id_ igual a 0, ele pode ser executado por qualquer thread. Por isso, a função somente compara se o nome do estado passado como parâmetro é igual ao nome do próximo estado a ser executado.
  * Caso o estado possua _id_ positivo, ele deve ser executado pela thread com aquele _id_. É importante notar que, por exemplo, se _id_ for igual a 1, não é garantido que aquela thread é a primeira a ser executada. Porém, ela vai ser considerada a primeira quando entrar nesta função e não existir nenhuma thread associada ainda como primeira thread no vetor _threadIdArray_, preenchendo esta posição do vetor com seu _id_ real.
  * Caso o estado possua _id_ negativo, ele pode ser executado por qualquer thread menos pela thread que possui aquele _id_.

  
Por último, a função _signalHandler_ é a função que manipula o sinal SIGALRM, gerado quando o alarme configurado é ativado. Ela imprime na tela qual seria o próximo estado a ser executado e chama a _finalizeManager_, além de encerrar o programa.

### readStates.lua

Este arquivo contém a função readStates. Esta função é chamada pelo código em C e utiliza a biblioteca LPeg de Lua para realizar a leitura do arquivo e converter para um vetor com a ordem dos estados e seus _ids_. 

Por enquanto, a função detecta o caracter do espaço em branco e utiliza isso para separar o nome do estado de seu _id_. Além disso, converte a linguagem que define o _id_ para um número inteiro. (Mais detalhes sobre essa linguagem podem ser encontrados em [Como escrever o arquivo de configuração](../README.md#como-escrever-o-arquivo-de-configuração).)

Isto é algo simples para o número de possibilidades que a biblioteca LPeg oferece. Entretanto, foi importante começar a utilizá-la para permitir uma implementação mais simples dos [passos futuros](../README.md#passos-futuros).
