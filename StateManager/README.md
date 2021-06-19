# Módulo State Manager

## Descrição
Este módulo contém o código do projeto 'Determinando ordens de execução para testes de programas concorrentes'.
Ele permite que o usuário defina estados (ou eventos) em seu programa concorrente e a ordem na qual estes estados devem ser executados. O usuário pode definir se deseja que dois ou mais estados sejam executados pela mesma thread ou se não podem ser executados por uma thread específica. Para entender como escrever o arquivo de ordem de estados, [clique aqui](../README.md#-Como-escrever-o-arquivo-stateFile.txt).
É importante ressaltar que o programa do usuário precisa implementar a concorrência utilizando a biblioteca em C pthread.
Este módulo foi implementado em alguns programas reais. Para saber mais sobre estes exemplos, [clique aqui](../README.md#-Exemplos-de-uso).

## Funcionamento
