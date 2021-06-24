# Exemplo FirstExample

## Descrição

Este exemplo demonstra o funcionamento do módulo _StateManager_ no exemplo de leitores e escritores. Neste exemplo, escritores só podem acessar o _buffer_ para escrever algo quando nenhum outro escritor estiver escrevendo nem nenhum leitor estiver lendo. Porém, leitores podem ler do _buffer_ ao mesmo tempo, já que não modificam seu conteúdo.

Testando este programa com diferentes arquivos de configuração, é possível perceber alguns casos que não foram tratados da forma correta. Este é um bom exemplo para mostrar o uso do módulo _StateManager_ para descobrir detalhes do código que não estão funcionando da forma adequada.

## Funcionamento



## Arquivos

### main.c

Este arquivo possui uma função _main_ e sete funções auxiliares.

## Testes

Nesta pasta, há um _script_ de teste deste exemplo, assim como os arquivos de configuração utilizados por ele. Para executar este _script_, basta que o usuário modifique o caminho para as bibliotecas de Lua em C de seu computador. No script, elas estão na variável de ambiente LUA_CDIR. Na minha máquina (MacOS Mojave), pela forma como instalei Lua, estes arquivos estão na pasta: Users/annaleticiaalegria/lua-5.3.5/src
