# EventManager
## Descrição

Esta ferramenta auxilia o teste de programas concorrentes e foi projetada para ser utilizada em ambiente didático. Ela consiste em uma biblioteca com funções a serem chamadas pelo programa do usuário. Essas funções controlam o andamento das threads do programa através de eventos e de sequências possíveis desses eventos, descritas em um script escrito pelo usuário.

Apesar de ter sido desenvolvido pensando em ser utilizado em disciplinas de graduação, o módulo pode ser utilizado em qualquer programa concorrente que utilize a biblioteca pthread em C.

O usuário marca os eventos ao longo de seu código. Exemplos de eventos são pontos do programa antes da leitura de uma variável, antes e depois de uma região crítica, antes de uma thread ser colocada em espera, dentre outros. Cabe ao usuário definir estes eventos e especificar no script as ordens possíveis deles.

A ferramenta inclui alguns exemplos de uso que podem ajudar o usuário a determinar bons eventos para o seu programa e boas ordens para checar a corretude do mesmo. Eles estão documentados com explicações sobre seu funcionamento, forma de compilar e executar e com os testes realizados e seus significados.


### Requisitos funcionais

  * Fazer a leitura do script do usuário.
  * Garantir que o programa do usuário siga a ordem especificada pelo arquivo de configuração.
  * Garantir que as threads executem os estados associados a elas pelo script.
  * Permitir que o usuário defina o tempo máximo de um estado.
  * Finalizar o programa do usuário caso algum evento demore mais do que o tempo especificado.
  * Mostrar o possíveis próximos eventos esperados caso o programa seja encerrado por conta de algum bloqueio.

### Requisitos não funcionais

  * O programa do usuário deve implementar a concorrência utilizando a linguagem de programação C e a biblioteca pthread.
  * O programa deve executar em uma máquina com sistema operacional Linux ou MacOS.
  * O usuário deve instalar Lua 5.3 na máquina, caso já não esteja instalado.

### Projeto do programa

Este projeto consiste no módulo _EventManager_ que deve ser chamado pelo programa do usuário. O usuário pode nomear o script da forma que quiser, só deve informar ao módulo o caminho para o arquivo.

O programa assume que o módulo _EventManager está em um diretório diferente do diretório no qual o programa do usuário é compilado e executado, como mostrado a seguir. Entretanto, o usuário pode modificar este caminho no arquivo _eventManager.h_.

. <br/>
|---Diretório externo <br/>
| |---Pasta com programa do usuário <br/>
| | |---main.c <br/>
| | |---script.txt <br/>
| |---EventManager <br/>
| | |---eventManager.c <br/>
| | |---eventManager.h <br/>
| | |---luaMain.lua <br/>
| | |---[outros módulos auxiliares]


## Como utilizar a EventManager

### Configuração

Primeiramente, o usuário deve baixar a pasta com o módulo _EventManager_ e, idealmente, organizá-la como é descrito na seção [Projeto do programa](#projeto-do-programa). Para casos diferentes de organização, basta modificar a variável _luaMainFilePath_, presente no arquivo _eventManager.h_.

Além disso, o usuário deve se certificar de que todos os [requisitos não funcionais](#requisitos-não-funcionais) foram cumpridos.

### Como escrever o script

O próximo passo é escrever um script válido. Este arquivo contém as possíveis ordens de exeucução dos eventos do programa e regras de execução para as threads.


### Chamadas às funções

Para utilizar o _EventManager_, o usuário deve fazer chamadas às suas funções no seu programa.

Para inicializar o módulo, basta chamar a função _initializeManager_, passando como argumentos o caminho para o script e o caminho para o arquivo de configuração (se existir).

Chamadas à função _checkCurrentEvent_ devem ser feitas em pontos chave do programa, nos quais o usuário quer se certificar de que determinado evento ocorra em determinado momento. Ele deve passar como argumento para esta função o nome do evento que ele deseja que seja associado àquele ponto do programa. Este nome deve existir no script.

Por último, antes de encerrar seu programa, o usuário deve chamar a função _finalizeManager_, para garantir que os espaços alocados pelo módulo _EventManager_ sejam liberados.

Para mais detalhes acerca destas funções, cheque a [documentação](./EventManager/README.md) do módulo _EventManager_.

Agora, o usuário está pronto para compilar, executar e interpretar os resultados obtidos.

### Compilando e executando

É necessário compilar o código do _EventManager_ em conjunto com o código do usuário. Caso a disposição das pastas esteja de acordo com o que é mostrado em [Projeto do programa](#projeto-do-programa), para compilar o código, basta escrever o seguinte comando no terminal bash:

> gcc -Wall -o programa main.c ../StateManager/stateManager.c -I/$LUA_CDIR -llua5.3

A variável de ambiente LUA_CDIR deve ser o local no qual os arquivos lua.h, lauxlib.h e lualib.h se encontram.

Após compilado o programa, basta executar, passando os argumentos pedidos pelo programa do usuário via linha de comando.

> ./programa arg1 arg2

## Sobre os resultados

Cabe ao usuário interpretar os resultados obtidos. Uma sequência de eventos é dita aceita pelo programa caso ele tenha conseguido executar todos os eventos da sequência sem ocorrer nenhum bloqueio.

Caso ele tenha feito uma sequência de eventos que sabe ser válida e o programa não aceitou, sabe que seu programa está errado. Entretanto, caso o programa tenha aceitado a sequência inteira, isto não significa, necessariamente, que o programa do usuário está correto. É necessário checar outras sequências de eventos.

Caso o programa aceite alguma sequência que o usuário sabe estar errada, é garantido que o programa do usuário possui algum erro. 
