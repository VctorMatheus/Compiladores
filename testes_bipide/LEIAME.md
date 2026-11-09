# Testes do assembly da Roman++ no Bipide 3.0

Cada caso tem o programa em Roman++ (`Gxx.roman.txt`) e o assembly que o nosso compilador gerou
(`Gxx.asm`). Para testar: abra o `.asm` no Bipide (ou cole o texto), execute, digite os valores da coluna
**Entrada** quando o programa ler de `$in_port`, um por leitura, e compare o que aparecer em
`$out_port` com a coluna **Saída esperada**, na ordem.

As saídas esperadas vieram do simulador que escrevemos para os testes, a partir dos slides. O objetivo
aqui é confirmar que o Bipide real concorda.

| Caso | O que testa | Entrada | Saída esperada |
| --- | --- | --- | --- |
| G01 | `.data` com variáveis e vetor, saída de inteiros, char e lógico | (nenhuma) | 55 0 0 31 5 90 1 0 |
| G02 | Entrada em variável e em vetor (`v[n]`, `v[n - 1]`) | 2 10 20 30 | 2 10 30 20 |
| G03 | Atribuições a variáveis e a vetores (índice calculado) | (nenhuma) | 7 7 7 300 307 -5 |
| G04 | Soma e subtração com parênteses (usa temporários 1000+) | 10 3 4 | 9 11 13 9 -9 |
| G05 | Bit a bit: `AND`, `OR`, `XOR`, `NOT`, `ANDI`, `SLL`, `SRL` | 12 10 | 8 14 6 -13 92 6 -12 |
| G06 | Constantes fora de −1024..1023 no `.data` (`const_1502`) | (nenhuma) | 1502 -1498 -32768 |
| G07 | Literal acima de 16 bits (40000 vira −25536) | (nenhuma) | -25536 |
| G08 | Contas entre constantes feitas pelo compilador | (nenhuma) | 12 3 3 1 10 |

## O que mais interessa conferir

1. **G05, `NOT 0`:** o Bipide aceita o `NOT` com operando `0`? Se der erro de montagem, como ele
   espera o `NOT` (sem operando, ou outro valor)?
2. **G01 a G05, vetor no `.data`:** a forma `v : 0, 0, 0` (com vírgulas) é aceita?
3. **G06 e G07, nomes `const_1502`, `const_n32768`:** rótulos com `_` e números são aceitos?
4. **G02 e G05, `$indr`, `LDV` e `STOV`:** os elementos certos do vetor são lidos e escritos?

Se algum caso der diferente, anote o caso, a mensagem do Bipide ou a saída obtida.
