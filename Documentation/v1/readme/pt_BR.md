## Documentação
[English Documentation](Documentation/v1/docs/en_US.md) \
[Documentação em Português](Documentation/v1/docs/pt_BR.md) \
[ドキュメント](Documentation/v1/docs/ja_JP.md)
## Outros Idiomas (README)
[Português](Documentation/v1/readme/pt_BR.md) \
[日本語](Documentation/v1/readme/ja_JP.md)
## Introdução
SAH (Stack as Heap) é uma pequena biblioteca C de baixo nível que fornece um alocador baseado em pilha, construído sobre primitivas de memória virtual. É projetada para alocações temporárias, arenas, parsers, runtimes, máquinas virtuais e qualquer código crítico de desempenho que se beneficie de um gerenciamento de memória previsível e rápido.

Este não é um substituto de heap de uso geral.
## Funcionamento
SAH mapeia uma região de memória de tamanho fixo e posiciona uma página guarda abaixo da pilha para capturar estouros no nível do sistema operacional. Alocação e desalocação são O(1) e sem branches. A pilha cresce para baixo, seguindo a semântica real de pilha de CPU, gerenciada por meio de um ponteiro de base e um ponteiro de pilha explícitos.

Estouro não é detectado pela biblioteca. Se o ponteiro de pilha cruzar para a página guarda, o SO levanta uma falha de segmentação. Isso é intencional — bugs falham rápido, e o alocador permanece sem branches.

Duas interfaces de alocação são fornecidas: uma interface raw, onde o chamador gerencia os tamanhos manualmente, e uma interface estruturada, que armazena metadados para que os tamanhos não precisem ser rastreados pelo chamador.
## Layout
SAH mapeia duas regiões contíguas. A primeira é uma página guarda, posicionada na base da alocação, sem permissões de acesso. A segunda é a região de pilha utilizável, com 4096 bytes por padrão. O ponteiro de pilha começa no topo da região utilizável e se move para baixo a cada alocação.
```
Endereço alto  [ Região de pilha - 4096 bytes ]  <-- BP, SP começa aqui
               [ Página guarda  - 1 página    ]  <-- PROT_NONE / PAGE_NOACCESS
Endereço baixo
```
## Objetivos
- Overhead mínimo, sem verificações de limites nos caminhos críticos
- Semântica real de pilha: crescimento para baixo, BP e SP explícitos
- Detecção de estouro pelo SO via página guarda
- Comportamento simples e previsível com uma superfície de API reduzida
- Adequado para alocações temporárias e padrões de uso no estilo arena
## Exemplo
```c
#define SAH_IMPLEMENTATION
#include "sah.h"
int main(void)
{
    struct sah_stack* s = screate();
    /* push/pop raw — chamador controla o tamanho */
    int* x = push(s, sizeof(int));
    *x = 123;
    /* push/pop estruturado — tamanho armazenado internamente */
    char* buf = spush(s, 32);
    /* usar buf ... */
    spop(s);              /* libera buf */
    pop(s, sizeof(int)); /* libera x   */
    sdestroy(s);
    return 0;
}
```
Para usar o SAH, defina `SAH_IMPLEMENTATION` em exatamente uma unidade de tradução antes de incluir o header.
## Suporte a Plataformas
| Plataforma | Status     |
|------------|------------|
| Linux      | Disponível |
| Windows    | Disponível |

No Linux, SAH utiliza `mmap` e `mprotect`. No Windows, SAH utiliza `VirtualAlloc` e `VirtualProtect`. A API é idêntica em ambas as plataformas.
## Licença
BSD 3-Clause
## Observação
SAH é um projeto pessoal, escrito para aprendizado e diversão. É experimental e deve ser tratado como tal. Use em produção por sua própria conta e risco.
