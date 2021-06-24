# Exemplo FirstExample

## Descrição

Este exemplo demonstra o funcionamento do módulo _StateManager_ da forma mais simples possível. Nele, duas threads são criadas e ambas incrementam uma variável global chamada _count_. 

Caso não haja controle de acesso à variável global, seu valor final pode não ser igual ao esperado, por conta de condições de corrida. Este é o caso no programa _main.c_. Porém, se a região crítica tiver seu acesso protegido por semáforos, o programa sempre terá o resultado correto, como demonstrado no arquivo _main\_with\_semaphores.c_.

## Funcionamento

Os arquivos _main.c_ e _main\_with\_semaphores.c_ possuem o mesmo código base, tendo o segundo funções a mais por trabalhar com semáforos. 

Para ser capaz de reproduzir os possíveis erros de concorrência causados pelo programa sem semáforos, o valor da variável _count_ é carregado em uma variável auxiliar chamada _aux_ e, depois, é incrementado a partir do valor de _aux_. 

Separando essas operações, é possível simular a operação de incremento realizada pelo sistema operacional. Nesta operação, se a thread A começa a executar, carrega o valor de _count_ em um registrador. Pode ocorrer casos em que, antes de realizar a operação de somar um ao valor carregado no registrador, a thread interrompida pelo sistema operacional. Caso a thread B execute neste meio tempo, poderia carregar o mesmo valor de _count_ em um registrador. Quando ambas realizassem a operação de soma, por terem guardado o mesmo valor no registrador, uma dessas operações de soma seria sobrescrita pela outra, fazendo com que _count_ possua o valor '1' ao invés do valor '2' ao final do programa. Isto se deve ao fato do sistema operacional não garantir a atomicidade desta operação. 

Utilizando semáforos para proteger a leitura e escrita na variável global _count_, este possível erro é evitado, já que uma thread não conseguiria entrar na região crítica caso outra thread já estivesse ali.

Utilizando o módulo _StateManager_ e escolhendo a ordem certa de eventos, é possível verificar que o programa _main.c_ admite resultados errados para a variável _count_, já que admite uma ordem de eventos que não deveria ser válida. Já o programa _main\_with\_semaphores.c_ não aceita a ordem inválida de eventos. Mais detalhes sobre os testes podem ser encontrados na seção [Testes](#testes) deste arquivo.

## Arquivos

### main.c

Este arquivo possui uma função _main_ e três funções auxiliares.

Na _main_, o código trata o caso do programa ter sido executado sem ter sido passado, por linha de comando, o número de threads e o caminho para o arquivo de configuração dos estados. Além disso, são chamadas as funções de iniciar o _StateManager_ e inicializar as threads. Quando estas terminarem a execução, a _main_ imprime o valor final de _count_ e as funções de liberar as threads e de finalizar o _StateManager_ são chamadas.

Cada thread deste arquivo executa a mesma função: a _threadFunction_. Nela, a thread imprime seu _id_ e espera o estado _ThreadStarts_ ser o próximo da ordem de estados para começar. Em seguida, a thread espera o estado _ReadCount_, para armazenar o valor de _count_ na variável auxiliar _aux_, passo necessário para realizar a simulação comentada no [tópico anterior](#funcionamento). Por último, a thread espera a vez do estado _UpdateCount_ para armazenar o novo valor de _count_, imprimindo na tela o valor armazenado e encerrando.

### main_with_semaphores.c

Este arquivo possui uma função _main_ e seis funções auxiliares.

Assim como no arquivo anterior, na _main_, o código trata o caso do programa ter sido executado sem ter sido passado, por linha de comando, o número de threads e o caminho para o arquivo de configuração dos estados. Além disso, são chamadas as funções de iniciar o _StateManager_, inicializar o semáforo e as threads. Quando estas terminarem a execução, a _main_ imprime o valor final de _count_ e as funções de liberar as threads, o semáforo e de finalizar o _StateManager_ são chamadas.

Cada thread deste arquivo executa a mesma função: a _threadFunction_. Nela, a thread imprime seu _id_ e espera o estado _ThreadStarts_ ser o próximo da ordem de estados para começar. A thread espera o semáforo _semaphoreE_ liberar seu acesso para a região crítica. Em seguida, a thread espera o estado _ReadCount_, para armazenar o valor de _count_ na variável auxiliar _aux_. Depois, a thread espera a vez do estado _UpdateCount_ para armazenar o novo valor de _count_, imprimindo na tela o valor armazenado. Assim, ela pode informar ao semáforo que está saindo da região crítica e ele liberar o acesso para outra thread acessar esta região. Por último, a thread encerra.

É importante notar que o estado _ThreadStarts_ neste caso não era necessário, mas em outros exemplos, como no [ReaderWriter](../ReaderWriter/README.md) ele se mostra essencial. A documentação desse exemplo explica o porquê.

## Testes

Nesta pasta, há _scripts_ de teste deste exemplo, assim como os arquivos de configuração utilizados por eles. Para executar estes scripts, basta que o usuário modifique o caminho para as bibliotecas de Lua em C de seu computador. No script, elas estão na variável LUA_CDIR. Na minha máquina (MacOS Mojave), pela forma como instalei Lua, estes arquivos estão na pasta: Users/annaleticiaalegria/lua-5.3.5/src

O _script_ _runMain.sh_ executa dois testes com o arquivo _main.c_, que não utiliza semáforos. Ambos os testes são feitos com 2 threads mas cada um recebe um arquivo de configuração. O primeiro recebe o arquivo _statesFile\_1.txt_ e o segundo recebe _statesFile\_2.txt_ .

Já o _script_ _runMain\_with\_semaphore.sh_ executa os mesmos testes que o arquivo anterior mas com o arquivo _main\_with\_semaphores.c_ .

A ordem dos eventos do arquivo de configuração _statesFile\_1.txt_ é:

> ThreadStarts 1<br/>ReadCount 1<br/>ThreadStarts !1<br/>ReadCount !1<br/>UpdateCount 1<br/>UpdateCount !1

Isto é, a thread chamada de 1 deve executar os estados _ThreadStarts_ e _ReadCount_. Em seguida, a outra thread deve executar os mesmos estados: _ThreadStarts_ e _ReadCount_. Por último, a thread 1 executa _UpdateCount_ e a thread 2 executa _UpdateCount_. Sabemos que esta ordem é uma ordem errada, pois gera uma condição de corrida na variável _count_.

Quando testado com o arquivo _main.c_, o programa aceita esta ordem e imprime um valor errado de _count_, já que ambas as threads leram o mesmo valor de _count_ no estado _ReadCount_. 

Já o arquivo _main\_with\_semaphores.c_ não aceita esta ordem, sendo encerrado por estar em _deadlock_. Isto ocorre porque quando a thread 1 executa o estado _ReadCount_, ela está na região crítica, ou seja, possui o _lock_ do semáforo. Quando a thread 2 tenta executar o estado _ReadCount_, ela não consegue pois o semáforo não a deixa entrar na região crítica, deixando a thread em espera eterna.

O fato do segundo programa não ter aceitado a ordem de estados do arquivo mostra que ele pode estar correto. Como o primeiro programa aceitou, temos certeza de que ele está errado.

Os testes com o arquivo de configuração _statesFile\_2.txt_ utilizam a ordem:

> ThreadStarts 1<br/>ReadCount 1<br/>UpdateCount 1<br/>ThreadStarts !1<br/>ReadCount !1<br/>UpdateCount !1

Ou seja, a thread chamada de 1 deve executar os estados _ThreadStarts_, _ReadCount_ e _UpdateCount_ em sequência. Em seguida, a outra thread deve executar os mesmos estados: _ThreadStarts_, _ReadCount_ e _UpdateCount_. Sabemos que esta ordem é uma ordem válida, pois garante a atomicidade da operação de leitura e escrita.

Ambos os programas testados aceitaram a ordem válida, imprimindo o valor correto de _count_. Caso algum deles não tivesse aceitado esta ordem, teríamos certeza de que ele estaria errado. Porém, o fato do programa ter aceitado uma ordem válida não garante que ele esteja certo (como já foi visto que o programa _main.c_ está errado).

Este exemplo mostra a importância de se ter consciência da ordem dos estados no arquivo de configuração e saber interpretar os resultados para garantir a corretude do programa.