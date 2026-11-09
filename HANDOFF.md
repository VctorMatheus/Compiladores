# Handoff Roman++ — T4 (BIP)

*09/11/2026*

O T4 (geração de código para o BIP, apresentação em **10/11/2026**) está implementado, testado num
simulador e publicado no `main` do GitHub (commit `3c34f5f`). Falta só conferir três detalhes de
sintaxe no Bipide 3.0 real (ver [Pendências](#pendências-antes-da-apresentação)).

## Situação das etapas

| Etapa | Apresentação | Escopo | Estado |
| --- | --- | --- | --- |
| T1 | 18/08 | Léxico | Pronto |
| T2 | 15/09 | IDE + sintático (SLR) | Pronto |
| T3 | 20/10 | Semântico, tabela de símbolos na IDE | Pronto (commit `905f797`) |
| T4 | 10/11 | Geração de código BIP: declarações, E/S, atribuições, + − e bit a bit, aba de asm | Pronto (commit `3c34f5f`), falta conferir no Bipide |
| T5 | 01/12 | Relacionais, `si`, `si/aliter`, `dum`, `iterum`, `pro`, aninhamento | Não iniciado |
| T6 | 09/12 | Sub-rotinas, parâmetros por cópia, retorno em expressões | Não iniciado |

Repositório: [VctorMatheus/Compiladores](https://github.com/VctorMatheus/Compiladores), branch `main`.
Testes: 96 casos, todos passando.

## O que o T4 entrega

| Requisito (pontos) | Exemplo em Roman++ | Assembly gerado |
| --- | --- | --- |
| Declarações (1) | `atomus x, v[3];` | `x : 0` e `v : 0, 0, 0` no `.data` |
| Entrada (1) | `legere(x, v[i]);` | `LD $in_port` / `STO x`; vetor: índice em `$indr` e `STOV v` |
| Saída (1) | `inscribe(55, x, v[1]);` | `LDI 55` / `LD x` / `LDV v`, depois `STO $out_port` |
| Atribuições (3) | `x = 7; v[a - 6] = x;` | `STO x`; vetor: índice em `$indr` e `STOV v` |
| Aritmética e bit a bit (3) | `a + b`, `a - (b - c)`, `&`, `\|`, `^`, `~`, `<< 2` | `ADD`/`ADDI`, `SUB`/`SUBI`, `AND`/`ANDI`, `OR`/`ORI`, `XOR`/`XORI`, `NOT`, `SLL`/`SRL` |
| Asm na IDE (1) | botão Compilar | aba **Assembly** + botão **Salvar .asm** |

Exemplo completo:

```
principium                          .data
  atomus x, v[3];                      x : 0
  legere(x);                           v : 0, 0, 0
  v[1] = x + 2;                     .text
  x = (x - v[1]) | 4;               _PRINCIPAL:
  inscribe(x, v[1]);                   LD $in_port     LDI 1         ORI 4
terminus                               STO x           STO $indr     STO x
                                       LD x            LDV v         LD x
                                       ADDI 2          STO 1000      STO $out_port
                                       STO 1000        LD x          LDI 1
                                       LDI 1           SUB 1000      STO $indr
                                       STO $indr                     LDV v
                                       LD 1000                       STO $out_port
                                       STOV v                        HLT 0
```

**Arquivos novos:** `GeradorBIP.h/.cpp` (o gerador) e `testes/simulador_bip.h` (simulador do BIP usado
só nos testes).
**Alterados:** `Semantico.*`, `Compilador.*`, `MainWindow.*`, `CMakeLists.txt`, `testes/rodar_casos.cpp`,
`casos_de_teste.txt`, `README.md`, `CLAUDE.md`.
A gramática (`Roman++.vgls`) **não mudou**: não foi preciso regenerar no Web-GALS.

## Como a geração funciona

- **Mesmas ações semânticas.** Não há `#n` novos: cada ação que já existia valida e depois chama o
  `GeradorBIP`. Nos slides o professor usa ações próprias para gerar código porque a gramática dele
  não tem semântico; na apresentação, a resposta é "a mesma ação valida e gera".

  | Ação | Semântico | Geração |
  | --- | --- | --- |
  | #3 / #4 | declara variável / tamanho do vetor | linha no `.data` |
  | #11 / #12 | uso de variável / de `v[i]` | operando / `STO $indr`, `LDV v` |
  | #20–#24 | tipo do literal | operando imediato |
  | #29, #31–#39 | confere tipos da operação | `ADD`, `SUB`, `AND`, `OR`, `XOR`, `SLL`… |
  | #40 / #41 / #42 | destino e compatibilidade da atribuição | `STO x` / `STOV v` |
  | #43 / #44 / #45 | `legere` / `inscribe` | `LD $in_port` / `STO $out_port` |
  | #82 | fecha os escopos | `HLT 0` |

- **Operandos "preguiçosos".** Literais e variáveis entram numa pilha sem gerar código; o código sai
  quando o operador é aplicado. Assim `x + 1` vira `LD x` / `ADDI 1`, como nos slides, e parênteses e
  precedência funcionam (o esquema `flagOp` dos slides só funciona da esquerda para a direita).
- **Temporários.** O resultado de uma subexpressão fica no acumulador e só vai para um temporário
  (endereços 1000, 1001…, fora do `.data`, como nos slides) quando outra conta precisa do ACC.
- **Constantes.** Contas entre constantes são feitas pelo compilador (`v[2 * 5]`, `a = 3 * 4`). O
  imediato do BIP vai de −1024 a 1023; valores maiores viram uma posição do `.data` (`const_2000 : 2000`).
  Tudo é de 16 bits com sinal; literal acima de 32767 gera aviso.
- **Nomes no `.data`.** O nome original; se repetir em outro escopo ou coincidir com uma instrução
  (`add`, `sub`…), ganha o escopo como sufixo (`x_principal1`).

## O que não gera código

O programa **continua compilando**; a Saída e a aba Assembly mostram o motivo e a linha.

| Construção | Motivo |
| --- | --- |
| `*`, `/`, `%` entre variáveis | O BIP não tem essas instruções (entre constantes funciona) |
| `<<` / `>>` por variável | `SLL`/`SRL` só aceitam quantidade fixa |
| `fractio`, `catena` (inclusive texto no `inscribe`) | O BIP só trabalha com inteiros |
| Relacionais, `et`/`vel`/`non`, `si`, laços | Ficam para o T5 |
| Funções e chamadas | Ficam para o T6 |

**Decisão em aberto (com os colegas):** multiplicação e divisão não são pedidas em nenhuma etapa. Dá
para fazer por software (sub-rotina de deslocar-e-somar para `*`, subtrações sucessivas para `/` e
`%`), com cerca de 150 linhas no gerador; por enquanto dão a mensagem de "não suportado".

## Pendências antes da apresentação

O assembly foi validado só no simulador dos testes, escrito a partir dos slides. Conferir no Bipide 3.0
(instalador `setup_v3.msi`, 3,5 MB, em [sourceforge.net/projects/bipide](https://sourceforge.net/projects/bipide/files/)):

- [ ] `~x` gera `NOT 0`: o Bipide aceita o operando `0` no `NOT`?
- [ ] Vetor no `.data` como `v : 0, 0, 0` (com vírgulas)
- [ ] Nomes como `const_2000` para constantes fora de −1024..1023 (teste com `a = 2000;`)
- [ ] Rodar G01–G05 do `casos_de_teste.txt` no Bipide e comparar com as saídas esperadas

Se algo falhar, a correção fica em `GeradorBIP.cpp` (funções `emitir`, `declarar`, `constante`).

## Próximos passos (T5 e T6)

- **T5 vai precisar de ações novas no `.vgls`** e de regenerar o `Constants.*` no Web-GALS:
  - `si ... aliter`: uma ação entre o bloco do `si` e o `aliter` (emitir `JMP FIMSE` e o rótulo do
    senão) e outra no fim do comando.
  - `pro`: o passo (`i = i + 1`) aparece no texto antes do corpo mas executa depois; guardar o código
    do passo e emiti-lo no fim do laço.
  - `dum` e `iterum` já têm quase tudo: #51 no início, #50 na condição, #52 no fim.
  - Desvios testam a última operação da ULA: comparar com `SUB` antes de `BEQ`/`BGT`… (como nos slides).
- **T6** deve aproveitar as ações de funções e chamadas que já existem (#13–#16, #70–#77).

## Como compilar, testar e onde está cada coisa

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.11.2/mingw_64
cmake --build build                    # IDE (feche a IDE antes, senão o link falha)
cmake --build build --target testar    # 96 casos, sem abrir a IDE
build/rodar_casos -p programa.txt "5 3"   # tabela de símbolos, assembly e saída simulada
```

- Casos de geração em `casos_de_teste.txt`: `ENTRADA:` (valores do `legere`), `SAIDA:` (o que o
  assembly deve escrever) e `GERACAO: NAO (linha N)` (compila sem gerar).
- Detalhes de decisões e armadilhas do GALS: `CLAUDE.md`. Regras da linguagem e da geração: `README.md`.
- Material do professor (fora do repositório, na pasta acima): especificação dos trabalhos e os slides
  "Conjunto de Instruções da Arquitetura BIP" e "GCOD1 BIP SEQ".
