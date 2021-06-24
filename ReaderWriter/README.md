# Exemplo ReaderWriter

## Descrição

Este exemplo demonstra o funcionamento do módulo _StateManager_ no exemplo de leitores e escritores. Neste exemplo, escritores só podem acessar o _buffer_ para escrever algo quando nenhum outro escritor estiver escrevendo nem nenhum leitor estiver lendo. Porém, leitores podem ler do _buffer_ ao mesmo tempo, já que não modificam seu conteúdo.

Testando este programa com diferentes arquivos de configuração, é possível perceber alguns casos que não foram tratados da forma correta. Este é um bom exemplo para mostrar o uso do módulo _StateManager_ para descobrir detalhes do código que não estão funcionando da forma adequada.

## Funcionamento

Para controlar o acesso dos leitores e dos escritores ao _buffer_, é necessário o uso de semáforos. No programa, o semáforo que controla este acesso é o _rw_. 

Quando um escritor vai escrever, ele adquire este semáforo, garantindo que nenhum outro escritor ou leitor acesse o _buffer_ ao mesmo tempo que ele.

Já quando um leitor vai ler, ele confere se já existe algum leitor lendo o _buffer_, conferindo o valor da variável _activeReaders_. Se ele for o primeiro leitor a ler o _buffer_, ele adquire o semáforo _rw_. Se já existirem outros leitores realizando a leitura, e garantido que o primeiro já adquiriu a permissão do semáforo _rw_, então este novo leitor pode ler o _buffer_ sem se preocupar em ter algum escritor escrevendo. Ao terminar a leitura, o leitor confere se existe algum leitor lendo o _buffer_. Se existir, ele só segue em frente. Se não existir, quer dizer que ele é o último leitor a deixar a região crítica, tendo que liberar o semáforo _rw_ para que os escritores possam escrever. É importante notar que a variável _activeReaders_ também é protegida por um semáforo, chamado _mutexR_, já que todos os leitores podem modificar seu valor e não podem fazer isso ao mesmo tempo.

## Arquivos

### main.c

Este arquivo possui uma função _main_ e sete funções auxiliares.

Na _main_, o código trata o caso do programa ter sido executado sem ter sido passado, por linha de comando, o número de leitores, o número de escritores e o caminho para o arquivo de configuração dos estados. Além disso, são chamadas as funções de iniciar o _StateManager_, inicializar os semáforos e as threads. Quando estas terminarem a execução, as funções de liberar as threads, os semáforos e de finalizar o _StateManager_ são chamadas.

As threads chamadas por escritores executam a função _writeTask_. Nela, o escritor executa um while eterno (que no caso de executado com o _StateManager_ é encerrado quando todos os eventos foram cumpridos) no qual ele adquire o semáforo _rw_, gera um número aleatório e o coloca na próxima posição do _buffer_ circular. Após exibir qual número colocou, libera o semáforo _rw_.

Já as threads chamadas por leitores executam a função _readTask_. Ela também contém um while eterno (também encerrado pelo _StateManager_ quando acabam os estados) no qual o leitor adquire o semáforo _mutexR_ para acessar a variável _activeReaders_. Caso só este leitor esteja acessando o _buffer_ neste momento, ou seja, se _activeReaders_ = 1, ele adquire o semáforo _rw_. Após o acesso à variável _activeReaders_, o semáforo _mutexR_ é liberado. O leitor checa se há algo a ser lido no buffer, ou seja, se n > 0. Se tiver, ele realiza a leitura, exibindo o valor lido. Para encerrar, o leitor adquire novamente o semáforo _mutexR_ para conferir se é o último leitor a deixar a região crítica, ou seja, se _activeReaders_ = 0. Caso seja, libera o semáforo _rw_ e incrementa a próxima posição a ser lida, na variável _m_. Para encerrar a execução nesta iteração, libera o semáforo _mutexR_.

## Sobre pares de estados


Quanto aos estados criados pelo usuário, são 8:

>WriterWantsToStart<br/>WriterStarts<br/>WriterWrites<br/>WriterEnds<br/>ReaderWantsToStart<br/>ReaderStarts<br/>ReaderReads<br/>ReaderEnds<br/>

Os estados _WriterWantsToStart_ e _ReaderWantsToStart_ podem parecer desnecessários mas não são. Caso eles não existissem, poderia ocorrer a seguinte situação:

O usuário escreve um arquivo de configuração com a ordem:

>WriterStarts<br/>...<br/>ReaderStarts<br/>...<br/>

Porém, como há a aquisição de um semáforo antes do leitor e do escritor começarem, o _StateManager_ não conseguiria controlar estes estados. Vamos supor que a thread do leitor adquira o semáforo antes da thread do escritor. O leitor esperaria na _checkState_ o evento _ReaderStarts_ eternamente e não liberaria o semáforo. Assim, o escritor não conseguiria adquirir o semáforo para executar o estado _WriterStarts_, causando um _deadlock_.

Entretanto, a existência dos estados _WriterWantsToStart_ e _ReaderWantsToStart_ não tornam os estados _WriterStarts_ e _ReaderStarts_ desnecessários. Caso eles não existissem, poderia ocorrer a seguinte situação:

O usuário escreve um arquivo de configuração com a ordem:

>WriterWantsToStart<br/>ReaderWantsToStart<br/>...<br/>

O que poderia acontecer é o leitor executar o estado _WriterWantsToStart_ e liberar o escritor a executar seu estado _ReaderWantsToStart_ e a adquirir o semáforo _rw_ antes que o escritor adquira. Assim, a ordem dos eventos especificada pelo usuário não seria a ordem real do que aconteceu, já que, neste caso, o leitor começaria a executar antes do escritor e poderia admitir sequências de estados que são conhecidamente erradas.

Portanto, fica clara a necessidade de haver os pares de estados (_WriterWantsToStart_, _WriterStarts_) e (_ReaderWantsToStart_, _ReaderStarts_), englobando a aquisição do semáforo para garantir que os testes realizados correspondam ao que está ocorrendo no programa de fato.

## Testes

Nesta pasta, há um _script_ de teste deste exemplo, assim como os arquivos de configuração utilizados por ele. Para executar este _script_, basta que o usuário modifique o caminho para as bibliotecas de Lua em C de seu computador. No script, elas estão na variável de ambiente LUA_CDIR.

Este script realiza 6 testes. Os resultados dos testes são impressos no terminal e em um arquivo _.log_.

### Teste 1

Arquivo de configuração deste teste:

> WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>ReaderWantsToStart 1<br/>ReaderStarts 1<br/>ReaderReads 1<br/>ReaderEnds 1<br/>ReaderWantsToStart !1<br/>ReaderStarts !1<br/>ReaderReads !1<br/>ReaderEnds !1

Este teste executa com 2 leitores e 1 escritor.

Nele, o escritor escreve 1 item, dado pelos eventos sequenciais: _WriterWantsToStart_, _WriterStarts_, _WriterWrites_ e _WriterEnds_.

Em seguida, dois leitores diferentes executam o seguinte bloco de estados, um leitor de cada vez: _ReaderWantsToStart_, _ReaderStarts_, _ReaderReads_ e _ReaderEnds_.

O primeiro leitor lê o que o escritor escreveu e encerra atualizando o valor da variável _m_, que guarda a próxima posição a ser lida pelo grupo de leitores.

Quando o segundo leitor vai ler, ele lê um valor inexistente, já que o escritor não escreveu ainda nesta posição.

Este exemplo mostra que o programa não está controlando da forma adequada quando um leitor pode ler. Só trata o caso do _buffer_ estar vazio.

### Teste 2

Arquivo de configuração deste teste:

>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>ReaderWantsToStart 1<br/>
ReaderWantsToStart !1<br/>ReaderStarts 1<br/>
ReaderStarts !1<br/>ReaderReads 1<br/>
ReaderReads !1<br/>ReaderEnds 1<br/>ReaderEnds !1<br/>

Este teste executa com 2 leitores e 1 escritor.

Nele, o escritor escreve 1 item, dado pelos eventos sequenciais: _WriterWantsToStart_, _WriterStarts_, _WriterWrites_ e _WriterEnds_.

Em seguida, dois leitores diferentes executam os seguintes estados, intercalando entre si: _ReaderWantsToStart_, _ReaderStarts_, _ReaderReads_ e _ReaderEnds_.

O primeiro leitor adquire o _lock_ do semáforo _rw_ e libera a região crítica para ele e para o segundo leitor. Ambos leem o que o escritor escreveu e encerram.

Neste teste, diferentemente do anterior, as leituras ocorreram sem problemas porque a próxima posição a ser lida foi atualizada pelo último leitor a deixar a região crítica.

### Teste 3

Arquivo de configuração deste teste:

> WriterWantsToStart 1<br/>WriterStarts 1<br/>WriterWrites 1<br/>WriterEnds 1<br/>WriterWantsToStart !1<br/>WriterStarts !1<br/>WriterWrites !1<br/>WriterEnds !1<br/>ReaderWantsToStart 2<br/>ReaderWantsToStart !2<br/>ReaderStarts 2<br/>ReaderStarts !2<br/>ReaderReads 2<br/>ReaderReads !2<br/>ReaderEnds 2<br/>ReaderEnds !2<br/>

Este teste executa com 2 leitores e 2 escritores.

Nele, cada escritor escreve 1 item, dado pelos eventos sequenciais: _WriterWantsToStart_, _WriterStarts_, _WriterWrites_ e _WriterEnds_. O segundo escritor espera o primeiro terminar de escrever para começar.

Em seguida, dois leitores diferentes executam os seguintes estados, intercalando entre si: _ReaderWantsToStart_, _ReaderStarts_, _ReaderReads_ e _ReaderEnds_.

Neste exemplo, apesar de estarem escritos dois itens no _buffer_, um por cada escritor, os leitores leram o mesmo item (o que foi escrito pelo primeiro escritor). Seria necessária mais uma iteração de leitores para eles lerem o segundo item.

Isto mostra que nem todos os itens serão consumidos, necessariamente.

### Teste 4

Arquivo de configuração deste teste:

> WriterWantsToStart 1<br/>WriterStarts 1<br/>WriterWantsToStart !1<br/>WriterStarts !1<br/>WriterWrites 1<br/>WriterEnds 1<br/>WriterWrites !1<br/>WriterEnds !1<br/>ReaderWantsToStart 2<br/>ReaderWantsToStart !2<br/>ReaderStarts 2<br/>ReaderStarts !2<br/>ReaderReads 2<br/>ReaderReads !2<br/>ReaderEnds 2<br/>ReaderEnds !2<br/>

Este teste executa com 2 leitores e 2 escritores.

Neste teste, os escritores tentam intercalar a escrita entre si. O primeiro escritor executa o evento _WriterWantsToStart_ e _WriterStarts_, adquirindo o _lock_ do semáforo _rw_. O segundo escritor executa o evento _WriterWantsToStart_ mas não consegue executar o próximo estado esperado, _WriterStarts_, gerando um _deadlock_ causado e detectado pelo _StateManager_.

Isso indica que o arquivo de configuração não contém uma ordem de estados válida para o programa, o que era esperado.

### Teste 5

Arquivo de configuração deste teste:

> ReaderWantsToStart *<br/><br/>ReaderStarts *<br/>ReaderReads *<br/>ReaderEnds *<br/>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>

Este teste executa com 1 leitor e 1 escritor.

Neste teste, o leitor começa a execução, executando os estados _ReaderWantsToStart_ e _ReaderStarts_. Entretanto, como não há nada a ser lido do _buffer_, o leitor não consegue executar o estado _ReaderReads_, gerando um _deadlock_ causado e percebido pelo _StateManager_, que encerra o programa.

Este teste mostra que o caso de um leitor não poder ler um _buffer_ vazio foi tratado no programa, já que a sequência de estados do arquivo de configuração não foi aceita.

### Teste 6

Arquivo de configuração deste teste:

> WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>WriterWantsToStart *<br/>WriterStarts *<br/>WriterWrites *<br/>WriterEnds *<br/>ReaderWantsToStart *<br/><br/>ReaderStarts *<br/>ReaderReads *<br/>ReaderEnds *<br/>

Este teste executa com 1 leitor e 1 escritor. É importante notar que o _buffer_ está definido como tendo tamanho 5.

Neste exemplo, o escritor executa 6 vezes o bloco de estados _WriterWantsToStart_, _WriterStarts_, _WriterWrites_ e _WriterEnds_.

Em seguida, o leitor realiza uma leitura, dada pelo bloco de estados: _ReaderWantsToStart_, _ReaderStarts_, _ReaderReads_ e _ReaderEnds_.

Neste exemplo, o escritor escreve 6 vezes seguidas no _buffer_ circular, que possui 5 posições. Ou seja, antes do leitor ler o que tinha na primeira posição do _buffer_, este valor foi sobrescrito pela sexta execução do escritor. Isto é verificado conferindo que o leitor lê o último valor escrito pelo escritor, ao invés do primeiro.

Este teste mostra que o programa do usuário não está tratando o caso (propositalmente ou não) de valores escritos poderem ser sobrescritos antes de serem lidos pelo leitor.