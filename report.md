# PAGINADOR DE MEMÓRIA - RELATÓRIO

1. Termo de compromisso

Os membros do grupo afirmam que todo o código desenvolvido para este
trabalho é de autoria própria.  Exceto pelo material listado no item
3 deste relatório, os membros do grupo afirmam não ter copiado
material da Internet nem ter obtido código de terceiros.

2. Membros do grupo e alocação de esforço

Preencha as linhas abaixo com o nome e o e-mail dos integrantes do
grupo.  Substitua marcadores `XX` pela contribuição de cada membro
do grupo no desenvolvimento do trabalho (os valores devem somar
100%).

  * Arthur de Oliveira Lima <arthurdeolima@gmail.com> 50%
  * Leandro Guatimosim Gripp <lgg.ufmg@gmail.com> 50%

3. Referências bibliográficas
* http://nitish712.blogspot.com/2012/10/thread-library-using-context-switching.html
* https://linux.die.net/man/3/swapcontext
* https://man7.org/linux/man-pages/man2/timer_create.2.html
* https://www.ibm.com/docs/en/i/7.3?topic=ssw_ibm_i_73/apis/sigaset.html
* https://www.ibm.com/docs/en/zos/2.3.0?topic=descriptions-sigaction


4. Estruturas de dados  
    1. Descreva e justifique as estruturas de dados utilizadas para
     gerência das threads de espaço do usuário (partes 1, 2 e 5).
     
     As estruturas de dados utilizadas foram listas simplesmente encadeadas, com apontadores auxiliares para o primeiro e para o último nó. Essa escolha deriva do fato de que as operações realizadas com maior frequência são remoção do primeiro elemento e adição de um elemento ao final da fila. Com essa estrutura de dados, ambas as operações podem ser realizadas em tempo constante, o que não seria possível caso fossem utilizados arrays. 
     
     Além disso, a única situação na qual pode-se precisar remover um elemento no meio da lista, o que tornaria mais eficiente o uso de uma lista duplamente encadeada, é no caso em que necessita-se remover uma thread da lista de "waiting" ou de "sleeping". Contudo, nessa situação já seria necessário varrer a lista para encontrar o nó correspondete à thread de toda forma. Assim, mesmo com a lista duplamente encadeada teríamos uma complexidade O(n) para essa operação. A escolha de utilizar uma lists simplesmente encadeada acarreta, nesse caso, a necessidade de se manter o nó anterior em uma variável durante a iteração, mas isso é um _overhead_ bem menor do que ter que armazenar o nó anterior para todas as listas.

    2. Descreva o mecanismo utilizado para sincronizar chamadas de
     dccthread_yield e disparos do temporizador (parte 4).
     
     Para resolver o problema de condições de corrida que poderia ocorrer com o estouro do _timer_ enquanto a função dccthread_yield está executando, a solução adotada foi de bloquear a recepção do sinal do estouro do timer por parte do _handler_, como a primeira operação da função dccthread_yield. Assim, quando a função executa ela passa o controle para o _scheduler_, que por sua vez realiza tanto o trabalho de colocar a thread de volta na fila de prontos quanto o de reiniciar o _timer_ e de desbloquear a sua recepção. É interessante observar que, do ponto de vista do _scheduler_, é irrelevante se o _timer_ foi mascarado por conta da função dccthread_yield ou se houve um estouro do temporizador.
    