## O Passo a Passo do Código

**1. Criando a área compartilhada (`shm_open` e `ftruncate`)**
* **O que faz:** O `shm_open` pede ao sistema operacional para criar uma região de memória compartilhada (o "quadro de avisos"). Depois, o `ftruncate` define o tamanho exato que essa região precisa ter (o tamanho da matriz C, que é $N \times P$).
* **Como explicar:** *"Professor, o primeiro passo foi criar o objeto de memória compartilhada e dimensioná-lo para caber todos os inteiros da matriz resultado."*

**2. Mapeando para a memória do processo (`mmap`)**
* **O que faz:** O objeto criado no passo anterior existe no sistema, mas seu programa ainda não sabe como acessá-lo como se fosse uma variável normal. O `mmap` "espelha" essa região pública para dentro da memória do seu processo, criando o ponteiro `*C`.
* **Como explicar:** *"Usamos o `mmap` para trazer essa região de memória compartilhada para o espaço de endereçamento virtual do processo pai, permitindo que a gente a trate como um array (vetor) normal em C."*

**3. A Fábrica de Processos (`fork()`)**
* **O que faz:** O código usa dois laços `for` para iterar sobre a matriz resultado. Para cada posição (cada $C_{ij}$), ele chama o `fork()`, criando um processo filho exclusivamente para calcular aquele número.
* **Como explicar:** *"Fizemos um loop pelas dimensões da matriz C. Para cada elemento, o pai faz um `fork()`. Se a matriz é $2 \times 2$, criamos 4 processos filhos."*

**4. O Trabalho do Filho (Cálculo e escrita)**
* **O que faz:** O filho pega a linha da matriz A e a coluna da matriz B, multiplica e soma tudo. A mágica acontece aqui: ele escreve o resultado *direto* no ponteiro `C` da memória compartilhada. Depois de escrever, o filho usa `exit(0)` para morrer, pois seu único trabalho foi concluído.
* **Como explicar:** *"Cada filho calcula o produto interno (linha $\times$ coluna). Como a matriz C está mapeada na memória compartilhada, a escrita que o filho faz sobrevive mesmo depois que ele dá o `exit(0)`."*

**5. A Sincronização do Pai (`wait()`)**
* **O que faz:** O processo pai não pode imprimir a matriz e fechar o programa antes de os filhos terminarem, senão ele vai imprimir lixo da memória. O laço com `wait(NULL)` faz o pai dormir até que todos os filhos tenham finalizado.
* **Como explicar:** *"O processo pai entra em um laço de espera chamando `wait()` o mesmo número de vezes que chamou `fork()`. Isso garante que a impressão final da matriz só ocorra quando 100% dos cálculos estiverem na memória."*

**6. Devolvendo o recurso (`munmap` e `shm_unlink`)**
* **O que faz:** É a faxina. Desfaz o mapeamento da memória e deleta a região compartilhada do sistema operacional.
* **Como explicar:** *"Por fim, para não deixar lixo no sistema (vazamento de memória), usamos `munmap` para desfazer a ponte, e `shm_unlink` para destruir a memória compartilhada."*

---

## Respondendo às perguntas conceituais do Professor

No documento, o roteiro de avaliação pede que você saiba explicar pontos específicos. Aqui estão os argumentos prontos para você usar na apresentação:

**Por que C precisa estar na memória compartilhada?**
> *"Porque quando usamos o `fork()`, o filho ganha uma cópia das variáveis do pai. Se C fosse uma matriz normal, cada filho escreveria na sua própria cópia, e o pai nunca veria os resultados. A memória compartilhada é a ponte entre eles."*

**Por que não dá conflito de escrita e não precisa de Mutex?**
> *"Não dá conflito porque isolamos o domínio do problema. Cada processo filho tem uma 'coordenada' única e exclusiva baseada nas variáveis `i` e `j` do laço. O filho responsável pelo índice `[0][0]` nunca vai tentar escrever na posição `[0][1]`. Como ninguém escreve no mesmo lugar, não existe Condição de Corrida (Race Condition), e o Mutex se torna desnecessário."*

**Por que essa abordagem tem um alto custo e escala mal?**
> *"Criar um processo exige muito do SO (overhead). Ele precisa alocar um PID, copiar tabelas de memória e colocar o processo no escalonador. Se fôssemos multiplicar duas matrizes $1000 \times 1000$, estaríamos pedindo ao SO para criar 1 milhão de processos. O computador ia travar gastando mais tempo gerenciando processos do que fazendo a matemática."*

*Dica extra:* Se você conseguir explicar a diferença entre o espaço de memória que o `fork()` copia e o espaço que o `mmap` compartilha, o professor vai ficar muito satisfeito.
