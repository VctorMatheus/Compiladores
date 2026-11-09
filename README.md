# Roman++

Compilador e IDE para a linguagem **Roman++**, uma linguagem com palavras-chave em latim
(`principium`, `si`, `dum`, `opus`, `terminus`...). Projeto da disciplina de Compiladores.

Os analisadores léxico e sintático (SLR) são gerados pelo [Web-GALS](https://lia-univali.github.io/Web-GALS/)
a partir de [`Roman++.vgls`](Roman++.vgls); a IDE é feita em C++ com Qt 6 Widgets.

## Exemplo

```
principium
  opus atomus dobro(atomus n) sic
    reditus n * 2;
  cis

  atomus x;
  x = dobro(21);
  si (x >= 42) fac {
    inscribe("Ave, Roma!", x);
  }
terminus
```

## Estrutura

| Arquivo | Conteúdo |
|---|---|
| `Roman++.vgls` | Gramática (tokens e produções) para o GALS |
| `Lexico.*`, `Sintatico.*`, `Constants.*`, `Token.h`, `*Error.h` | Código gerado pelo GALS |
| `Semantico.*`, `TabelaSimbolos.*`, `Tipos.h` | Analisador semântico (tipos, escopos, funções, avisos) |
| `SemanticTable.h` | Tabela de compatibilidade de tipos (modelo do professor, preenchido para a Roman++) |
| `GeradorBIP.*` | Geração de código assembly para o processador BIP (Bipide 3.0) |
| `Compilador.*` | Ponte entre a IDE e os analisadores (erros → linha/coluna) |
| `MainWindow.*`, `main.cpp` | Interface: editor, botões Compilar e Salvar .asm, abas Saída / Tabela de Símbolos / Assembly |
| `fontes/` | Fontes embutidas (Cinzel, Marcellus SC, Almendra, Alegreya — licença SIL OFL) |
| `imagens/` | Fundo de papiro e o script Python que o gera |
| `casos_de_teste.txt` | Cenários de sucesso, de erro léxico/sintático/semântico e de avisos |

## Como compilar

Requisitos: Qt 6 (kit MinGW 64-bit), CMake e Ninja — todos instaláveis pelo Qt Online Installer.

**Qt Creator:** *File → Open File or Project* → `CMakeLists.txt` → kit *Desktop Qt 6 MinGW 64-bit* → `Ctrl+R`.

**Terminal:**

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.11.2/mingw_64
cmake --build build
```

O build roda o `windeployqt` automaticamente, então `build/CompiladorIDE.exe` abre direto pelo Explorer.

**Testes** (todos os casos de `casos_de_teste.txt`, sem abrir a IDE):

```bash
cmake --build build --target testar
```

Os casos de geração de código (`G..`) executam o assembly gerado num simulador do BIP
(`testes/simulador_bip.h`) e comparam o que sai em `$out_port`. Para ver o assembly de um programa
e executá-lo com entradas:

```bash
build/rodar_casos -p programa.txt "5 3"
```

## Regenerando os analisadores

Abra o `Roman++.vgls` no [Web-GALS](https://lia-univali.github.io/Web-GALS/) e clique em **Gerar Código**:
as opções (C++, léxico e sintático, SLR, entrada string) já estão no cabeçalho `#Options` do arquivo.
O `.vgls` é o formato de projeto do Web-GALS (o `.gals` é o do GALS 2003) e serve para qualquer
linguagem-alvo; a linha `Language` só guarda a escolhida. Do zip gerado, copie para a raiz **só o
`Constants.cpp` e o `Constants.h`** (os demais arquivos não mudam). **Nunca copie o `Semantico.*`**: o do
GALS é vazio e apagaria o analisador semântico.

Se o Web-GALS perguntar qual ação executar ("Executar ação semântica N" ou "Empilhar"), a gramática tem
um conflito: não gere o código, corrija a posição das ações. A causa mais comum é usar o mesmo número de
ação no fim de produções diferentes; por isso cada nível de precedência das expressões tem a sua (#31 a #39).

## Regras semânticas (estilo C)

- **Tipos:** `atomus` (int), `fractio` (float), `caracter` (char), `logicum` (bool), `catena` (string).
  char e bool são promovidos a int; com um `fractio` na conta, o resultado é `fractio`. As regras estão na
  `SemanticTable.h` (`expTable` para operações, `atribTable` para atribuições, argumentos e `reditus`).
- **Conversões:** permitidas entre os tipos numéricos; `fractio` → `atomus`/`caracter` gera aviso.
  `catena` não se converte e não tem operadores.
- **Escopos:** cada bloco abre um escopo; um nome interno esconde o externo; repetir no mesmo escopo é erro.
  Parâmetros e corpo da função dividem o escopo. Funções enxergam só os próprios nomes, a si mesmas (recursão) e as funções anteriores.
- **Vetores:** tamanho constante e positivo (`v[2 * 5]` vale), índice inteiro; sem índice, só como argumento
  para um parâmetro `tipo v[]` do mesmo tipo.
- **Funções:** quantidade e tipo dos argumentos conferidos; `munus` não devolve valor e não entra em expressões;
  `reditus` só dentro de função e compatível com o tipo dela. `exi` só dentro de laço.
- **Avisos:** variável não usada ou nunca lida, função nunca chamada, uso sem valor, truncamento de `fractio`,
  função sem `reditus`, índice constante fora dos limites e divisão por zero.
- **Tabela de símbolos:** a aba *Tabela de Símbolos* da IDE mostra, a cada compilação, todos os identificadores
  (programa, funções, parâmetros, variáveis e vetores) com tipo, modalidade, escopo (`global`, `principal`,
  nome da função, `principal1`... para blocos), inicialização, uso, posição do parâmetro e tamanho do vetor.

## Geração de código para o BIP

A cada compilação sem erros, a aba *Assembly* mostra o programa para o processador BIP, pronto para o
Bipide 3.0 (o botão *Salvar .asm* grava o arquivo). O código é gerado pelas mesmas ações semânticas,
no formato dos slides da disciplina:

```
principium                         .data
  atomus x, v[3];                     x : 0
  legere(x);                          v : 0, 0, 0
  v[1] = x + 2;                    .text
  inscribe(v[1]);                  _PRINCIPAL:
terminus                              LD $in_port
                                      STO x
                                      LD x
                                      ADDI 2
                                      STO 1000
                                      LDI 1
                                      STO $indr
                                      LD 1000
                                      STOV v
                                      ...
                                      HLT 0
```

- **Gerado:** declarações (`.data`, vetores com um `0` por posição), `legere` e `inscribe` com
  variáveis, vetores e inteiros, atribuições a variáveis e a vetores, `+`, `-`, `&`, `|`, `^`, `~`,
  `-` unário e `<<`/`>>` por constante (`SLL`/`SRL`).
- **Operandos:** literais e variáveis só viram código quando o operador é aplicado, o que permite as
  formas imediatas (`x + 1` → `LD x` / `ADDI 1`). Resultados intermediários ficam no acumulador e vão
  para temporários (endereços 1000, 1001...) só quando necessário. Contas entre constantes são feitas
  pelo compilador (`v[2 * 5]`, `a = 3 * 4`).
- **16 bits:** o imediato do BIP vai de −1024 a 1023; constantes maiores viram uma posição do `.data`
  (`const_2000 : 2000`). Literais fora de 16 bits geram aviso.
- **Não gerado** (o programa compila e a IDE explica o motivo no lugar do assembly): `*`, `/` e `%`
  entre variáveis (o BIP não tem essas instruções), deslocamento por variável, `fractio` e `catena`
  (o BIP só trabalha com inteiros) e, por enquanto, relacionais, lógicos, `si`, laços e funções
  (etapas T5 e T6).
