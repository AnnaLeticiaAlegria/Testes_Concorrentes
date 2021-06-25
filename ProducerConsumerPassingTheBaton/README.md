# Exemplo ProducerConsumerPassingTheBaton

## Descrição

Este exemplo demonstra o funcionamento do módulo _StateManager_ em um caso mais complexo. 

Neste problema, há produtores e consumidores. Os produtores produzem itens para um _buffer_ circular e os consumidores consomem itens desse _buffer_. Entretanto, diferentemente do caso [ReaderWriter](../ReaderWriter/README.md), um produtor só pode sobrescrever uma posição do _buffer_ caso todos os consumidores tenham consumido o item daquela posição.

Além disso, este problema implementa a passagem de bastão. O que significa que, quando um produtor/consumidor terminar de produzir/consumir, ele passa o bastão para outro produtor/consumidor assumir o comando. A forma como isso é implementada é descrita na seção a seguir.


## Funcionamento

Este programa recebe como parâmetros a quantidade de posições que o _buffer_ deve ter, a quantidade de produtores, a quantidade de consumidores e a quantidade de itens a serem produzidos/consumidos.

Os consumidores não consomem, necessariamente, o mesmo item a cada momento. Por isso, é importante guardar, para cada posição do _buffer_ a quantidade de consumidores que faltam consumir este item. Isto está no vetor _consumersToRead_. Além disso, existe um vetor com a quantidade de itens lidos por cada produtor, o vetor _hasRead_. a partir desta quantidade e do tamanho do _buffer_, é possível calcular a próxima posição que cada consumidor deve consumir.

Entretanto, os produtores não precisam de um vetor para indicar a posição do próximo item a ser produzido. Esta posição é guardada em uma variável do tipo _int_ já que é a mesma para todos os produtores.

Antes de começar a produzir, o produtor checa se o _buffer_ circular está cheio, ou seja, só com itens que ainda não foram produzidos por 1 ou mais consumidores. Caso esteja cheio, ele se coloca em espera.

Antes de começar a consumir, o consumidor checa se o _buffer_ circular está vazio para ele, ou seja, se ele já consumiu a mesma quantidade de itens que já foram produzidos. Caso esteja, ele se coloca em espera.

A passagem de bastão acontece quando um consumidor/produtor detecta que há algum consumidor/produtor em espera e o libera desta espera antes de encerrar sua iteração. É importante notar que o semáforo que controla o _buffer_, semáforo _e_, não é liberado pelo consumidor/produtor que passou o bastão. Logo, o consumidor/produtor que recebeu o bastão, ou seja, o que acordou da espera, não precisa adquirir o semáforo _e_. Como o consumidor/produtor que passou o bastão já estava na região crítica definida pelo semáforo _e_, ele garante que não havia mais nenhuma thread acessando o _buffer_ além dele. Então, ao passar o bastão e encerrar sua iteração, é garantido que somente a thread que recebeu o bastão vai estar na região crítica. Caso não haja nenhum consumidor nem nenhum produtor esperando, o consumidor/produtor que está atuando simplesmente libera o semáforo _e_, já que não tem que passar o bastão para ninguém.

## Arquivos

### main.c

Este arquivo possui uma função _main_ e onze funções auxiliares.

Na _main_, o código trata o caso do programa ter sido executado sem ter sido passado, por linha de comando, a quantidade de posições que o _buffer_ deve ter, a quantidade de produtores, a quantidade de consumidores, a quantidade de itens a serem produzidos/consumidos e o arquivo de configuração de estados. Além disso, são chamadas as funções de iniciar o _StateManager_, inicializar os semáforos, os vetores e as threads. Quando estas terminarem a execução, as funções de liberar as threads, os vetores e os semáforos e de finalizar o _StateManager_ são chamadas.

As threads chamadas por produtores executam a função _producer_. Nela, a cada iteração, o produtor gera um número aleatório que é o item a ser inserido no _buffer_. Então, é chamada a função _deposits_.

Já as threads chamadas por consumidores executam a função _consumer_. Nela, a cada iteração, o consumidor chama a função _consumes_, que retorna o item consumido. Em seguida, consumidor imprime este item.

Na função _deposits_, o produtor adquire o semáforo _e_. Depois, calcula a posição do item a ser produzido.

* Se algum consumidor ainda não consumiu o item daquela posição ainda, o produtor libera o semáforo _e_ e se coloca em espera no semáforo 'semProd'. Quando algum produtor/consumidor liberar este semáforo, ocorre o recebimento da passagem de bastão.
* Se todos os consumidores consumiram o item, o produtor só segue em frente.

Depois de produzir o item (caso ainda faltem itens a serem produzidos), o produtor checa se tem algum consumidor esperando.

* Se tem, ele passa o bastão para o primeiro consumidor que estiver esperando.
* Se não tem, ele checa se todos os consumidores já leram a próxima posição e se tem algum produtor esperando para produzir.
  * Se tem, ele passa o bastão para o produtor produzir (e como ele só entra nesse caso se a próxima posição está pronta para receber um novo item, o produtor que recebeu o bastão pode simplesmente produzir).
  * Se não tem, ele libera o semáforo _e_, não passando o bastão para ninguém.

Na função _consumes_, o consumidor adquire o semáforo _e_. Depois, ele checa se tem algo para ser lido por ele (se o número de itens lidos por este consumidor é menor do que o número total de itens escritos).

* Se tem o que consumir, continua
* Se não tem, libera o semáforo _e_ e se coloca em espera no semáforo _semCons[myId]_. Quando um produtor/consumidor liberar este semáforo, ocorre o recebimento da passagem de bastão.

Depois de consumir o item (caso ainda faltem itens a serem consumidos), o consumidor checa se tem algum consumidor esperando.

* Se tem, ele passa o bastão para o primeiro consumidor que estiver esperando.
* Se não tem, ele checa se todos os consumidores já leram a próxima posição e se tem algum produtor esperando para produzir.
  * Se tem, ele passa o bastão para o produtor produzir (e como ele só entra nesse caso se a próxima posição está pronta para receber um novo item, o produtor que recebeu o bastão pode simplesmente produzir).
  * Se não tem, ele libera o semáforo _e_, não passando o bastão para ninguém.
  

## Testes

Nesta pasta, há um _script_ de teste deste exemplo, assim como os arquivos de configuração utilizados por ele. Para executar este _script_, basta que o usuário modifique o caminho para as bibliotecas de Lua em C de seu computador. No script, elas estão na variável de ambiente LUA_CDIR.

Este script realiza 2 testes. Os resultados dos testes são impressos no terminal e em um arquivo _.log_ na pasta Tests/Logs/

Para entender sobre o porquê da necessidade dos pares de eventos (_ConsumerWantsToStart_ e _ConsumerStarts_) e (_ProducerWantsToStart_, ProducerStarts_), verifique [nesta seção](../ReaderWriter/README.md#sobre-pares-de-estados) do exemplo anterior, é análogo a este exemplo.

### Teste 1

Arquivo de configuração deste teste:

> ConsumerWantsToStart *<br/>ConsumerStarts *<br/>ConsumerWaits *<br/>ProducerWantsToStart *<br/>ProducerStarts *<br/>ProducerProduces *<br/>ProducerEnds *<br/>ConsumerWantsToStart *<br/>ConsumerStarts *<br/>ConsumerConsumes *<br/>ConsumerEnds *<br/>ConsumerConsumes *<br/>ConsumerEnds *<br/>

Este teste é executado com um _buffer_ de tamanho 1, 1 produtor, 2 consumidores e 1 item a ser produzido.

Neste caso, o primeiro consumidor tenta consumir, pela sequência de estados _ConsumerWantsToStart_ e _ConsumerStarts_ mas não há itens a serem produzidos, então ele se coloca em espera, no estado _ConsumerWaits_. Caso esse estado não fosse o próximo no arquivo de configuração, a sequência não seria aceita, pois esta thread estaria esperando este estado.

O produtor realiza o bloco de estados que o faz produzir um item: _ProduzerWantsToStart_, _ProducerStarts_ e _ProducerProduces_. Como só foi pedido para 1 item ser produzido, este produtor finaliza, caracterizado pelo estado _ProducerEnds_. Porém, antes de finalizar, ele detecta que um consumidor está esperando e passa o bastão para ele. Por isso o consumidor executa o estado _ConsumerConsumes_ sem precisar passar pelos estados de _ConsumerWantsToStart_ e _ConsumerStarts_.

Depois de consumir, este consumidor encerra com _ConsumerEnds_. Agora, o outro consumidor também consome o item do produtor, executando o bloco de estados: _ConsumerWantsToStart_, _ConsumerStarts_, _ConsumerConsumes_ e _ConsumerEnds_.

### Teste 2

Arquivo de configuração deste teste:

> ProducerWantsToStart 1<br/>ProducerStarts 1<br/>ProducerProduces 1<br/>ProducerWantsToStart *<br/>ProducerStarts *<br/>ProducerWaits *<br/>ConsumerWantsToStart 2<br/>ConsumerStarts 2<br/>ConsumerConsumes 2<br/>ConsumerWantsToStart !2<br/>ConsumerStarts !2<br/>ConsumerConsumes !2<br/>ProducerProduces *<br/>ProducerEnds *<br/>ConsumerWantsToStart 2<br/>ConsumerStarts 2<br/>ConsumerConsumes 2<br/>ConsumerEnds 2<br/>ConsumerWantsToStart !2<br/>ConsumerStarts !2<br/>ConsumerConsumes !2<br/>ConsumerEnds !2<br/>ProducerWantsToStart *<br/>ProducerStarts *<br/>ProducerEnds *<br/>

Este teste é executado com um _buffer_ de tamanho 1, 2 produtores, 2 consumidores e 2 itens a serem produzidos.

Neste caso, o primeiro produtor produz 1 item, caracterizado pelo bloco de eventos: _ProducerWantsToStart_, _ProducerStarts_ e _ProducerProduces_.

Quando o próximo produtor a produzir (que pode ser, ou não, o mesmo que acabou de produzir) tenta produzir, ele encontra o _buffer_ cheio. Então, se coloca em espera. Pela sequência de eventos ter sido cumprida, percebe-se que está funcionando: _ProducerWantsToStart_, _ProducerStarts_ e _ProducerWaits_.

Em seguida, um consumidor consome o item do _buffer_, seguindo o bloco de estados: _ConsumerWantsToStart_, _ConsumerStarts_ e _ConsumerConsumes_.

É preciso fazer o outro consumidor consumir o item para a passagem de bastão para o produtor que está esperando poder ser testada. Por isso, é importante definir no arquivo de ordem de estados que o consumidor a consumir o bloco a seguir não pode ser o consumidor que acabou de executar: _ConsumerWantsToStart_, _ConsumerStarts_ e _ConsumerConsumes_. Isto é algo importante pois, caso o primeiro consumidor executasse os estados _ConsumerWantsToStart_ e _ConsumerStarts_, ele estaria esperando o estado _ConsumerWaits_, uma vez que já tinha consumido todos os itens disponíveis (neste caso, 1 item). Assim, o programa entraria em _deadlock_ já que o próximo estado do arquivo de configuração é _ConsumerConsumes_.

Agora que todos os consumidores consumiram o item, o segundo consumidor realiza a passagem de bastão para o produtor que estava esperando. Este executa os estados _ProducerProduces_ e _ProducerEnds_, já que os 2 itens pedidos já foram produzidos. Como ocorreu a passagem de bastão, o produtor não passou pelos eventos _ProducerWantsToStart_ e _ProducerStarts_, demonstrando que a técnica está funcionando.

Para finalizar, os dois consumidores leem o item produzido e encerram, realizando o bloco de estados, um de cada vez: _ConsumerWantsToStart_, _ConsumerStarts_, _ConsumerConsumes_ e _ConsumerEnds_.

Para finalizar o outro produtor, ele precisa iniciar. Mas como checa que não há nada mais a ser produzido, encerra direto: _ProducerWantsToStart_, _ProducerStarts_ e _ProducerEnds_.