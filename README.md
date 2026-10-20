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
| `Compilador.*` | Ponte entre a IDE e os analisadores (erros → linha/coluna) |
| `MainWindow.*`, `main.cpp` | Interface: editor, botão Compilar e abas Saída / Tabela de Símbolos |
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
