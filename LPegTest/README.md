# LPegTest

## Descrição

Este arquivo contém um teste retirado do site http://lua-users.org/wiki/LpegRecipes para testar a biblioteca LPeg, utilizada neste projeto. O código encontrado neste repositório só possui alguns comentários e _prints_ a mais com relação ao código original.

Neste teste, o objetivo é criar um _parser_ que capture comentários feitos em códigos da linguagem C.

## Funcionamento

Comentários em C são definidos por trechos de código entre os conjuntos de caracteres '/*' e '*/'. Assim, o código armazena nas variáveis os seguintes padrões:

* BEGIN_COMMENT: Armazena o padrão '/*', que define o início de um comentário.
* END_COMMENT: Armazena o padrão '*/', que define o final de um comentário.
* NOT_BEGIN: Qualquer padrão que não seja BEGIN_COMMENT.
* NOT_END: Qualquer padrão que não seja END_COMMENT.
* FULL_COMMENT_CONTENTS: Padrão definido quando encontrados os seguintes padrões, nesta ordem: BEGIN_COMMENT, NOT_END, END_COMMENT.

## Testes

Em seguida, o código realiza alguns testes. Todos os testes foram aplicados na string "codigoA/\*comentario\*/codigoB".

* (NOT_BEGIN * lpeg.C(FULL_COMMENT_CONTENTS))^0: Este código procura o padrão NOT_BEGIN_ e captura o padrão FULL_COMMENT_CONTENTS. Ou seja, quando aplicado em uma string retorna os trechos que correspondem ao padrão FULL_COMMENT_CONTENTS.
  * No teste, retornou "/\*comentario\*/".

* (lpeg.C(NOT_BEGIN) * FULL_COMMENT_CONTENTS)^0 * lpeg.C(NOT_BEGIN): Este código captura tudo que vem antes de um comentário e tudo que vem depois. Portanto, retorna dois valores. Caso não haja algo antes ou depois do comentário, retorna _nil_ para o valor correspondente.
  * No teste, retornou "codigoA" e "codigoB".

* (lpeg.C(FULL_COMMENT_CONTENTS) + 1)^0: Este código também captura somente os comentários.
  * No teste, retornou "/\*comentario\*/".

* ((1 - lpeg.P"/")^0 * (lpeg.C(FULL_COMMENT_CONTENTS) + 1))^0: Segundo o criador deste exemplo, este código foi sugestão do professor Roberto para realizar a captura dos comentários de forma mais rápida que o código acima.
  * No teste, retornou "/\*comentario\*/".