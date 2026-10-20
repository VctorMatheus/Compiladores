# Roman++ — notas para o Claude

Compilador + IDE (Qt 6, C++17) para a linguagem **Roman++**: estilo C, palavras-chave em latim. Projeto da
disciplina de Compiladores. O README cobre como compilar, as regras semânticas e a regeneração; este arquivo
guarda o que **não** dá para ver no código: armadilhas, decisões e pendências.

## Contexto da disciplina

A especificação e os slides ficam fora do repositório, na pasta acima (`..\Especificação-dos-Trabalhos.pdf`,
`..\1. Análise semântica...pdf`, `..\2. Análise Semântica...pdf`). Calendário:
T1 léxico (18/08) · T2 IDE + sintático (15/09; fonte 14 no editor e nas mensagens) ·
**T3 semântico (20/10/2026)**: tabela de símbolos com tipo/modalidade/escopo, escopos sem vazamento, unicidade,
avisos de não usado e de não inicializado, **tabela visível na IDE**, compatibilidade de tipos ·
T4 a T6: geração de código para o processador **BIP** (Bipide 3.0), com aba de assembly na IDE (10/11, 01/12, 09/12).

## Arquitetura em uma linha por parte

- `Roman++.vgls` → Web-GALS gera `Lexico.*`, `Sintatico.*` (SLR), `Constants.*` (tabelas), `Token.h`, `*Error.h`.
- `Semantico.*` + `TabelaSimbolos.*` + `Tipos.h`: semântico **escrito à mão**, dirigido pelas ações `#n` da gramática.
- `SemanticTable.h`: tabela de compatibilidade no **formato do professor** (`expTable`, `atribTable`,
  `resultType`, `atribType`), preenchida com as regras estilo C + coluna `BIT`. É a única fonte das regras
  de tipo das operações e atribuições; o `Semantico` só traduz `Tipo` ↔ índice e explica os erros.
- `Compilador.*`: ponte IDE ↔ analisadores (posição → linha/coluna, lista de avisos).
- `MainWindow.*`, `Estandarte.*`, `main.cpp`: interface (abas Saída e Tabela de Símbolos).
- `casos_de_teste.txt` + `testes/rodar_casos.cpp`: testes automáticos.

## Como validar qualquer mudança

```bash
cmake --build build --target testar
```

Roda todos os casos do `casos_de_teste.txt` sem Qt e sai com código 1 se algum falhar. Para um programa
avulso: `build/rodar_casos -p programa.txt` (mostra também a tabela de símbolos).
Se a IDE estiver aberta, o link do `CompiladorIDE.exe` falha com "Permission denied": feche a IDE antes. Ao mudar regras, **acrescente casos** ao `casos_de_teste.txt`
(formato no topo de `testes/rodar_casos.cpp`; `AVISO (linha N)` = compila com aviso naquela linha).

## Armadilhas do GALS / Web-GALS (aprendidas na prática)

- **Ignorar ≠ `:!`** (documentação oficial: lia-univali.github.io/Web-GALS/docs). Um token é **ignorado**
  quando é declarado **sem nome** (`: regex`, estado 0 no `TOKEN_STATE`). Já o `:!` significa **sem
  retrocesso**: começou o token, ele tem que terminar, senão é erro léxico (estados -2 no `TOKEN_STATE`).
  Os dois combinam: comentário de bloco é `:! [/][\*]...` (ignorado + `/*` sem fechar vira erro léxico,
  e não `/` e `*` soltos). `NOME :! regex` não é ignorado: vira token e chega ao parser.
- O `.vgls` é o formato de projeto do Web-GALS (o `.gals` é o do GALS 2003); **não tem relação com
  Python**: `Language = ...` no cabeçalho é só a linguagem-alvo salva (o exemplo oficial usa Python).
- A mensagem de erro léxico de token sem nome é "Erro identificando <ignorar>"; o `Compilador.cpp`
  troca por "Comentário de bloco /* não foi fechado". Mensagens do GALS ficam no `Constants.cpp`, que é
  sobrescrito: personalize no `Compilador`, não lá.
- **Mesmo número de ação no fim de produções diferentes gera conflito SLR**: o FOLLOW de uma ação é a
  união de todos os lugares onde ela aparece. Por isso cada nível de precedência tem a sua (#31 a #39).
  Regra prática: se várias alternativas começam com `ID`, todas levam a **mesma** ação logo após o `ID`
  (#10); ações que diferem ficam onde o próximo token já decide a alternativa.
- **Conflito = `prompt()`**: o Web-GALS pergunta "Executar ação semântica N / Empilhar X". Nunca gere
  código aceitando o padrão; corrija a gramática.
- **A ação recebe o último token lido** (`previousToken`). Ação logo após `ID` recebe o nome; no fim de
  uma produção recebe o último token dela. A análise é bottom-up: expressões usam pilhas
  (tipos, operadores, identificadores, chamadas, alvos de atribuição).
- **Regenerar = copiar só `Constants.cpp` e `Constants.h`.** Os demais arquivos gerados saem idênticos.
  **Nunca copie `Semantico.*`** do zip: é um stub vazio.
- Opções de geração ficam no cabeçalho `#Options` do `.vgls` (C++, léxico + sintático, SLR,
  `Input = String`, `ScannerTable = Full`, `ScannerCaseSensitive = true`): ao abrir o arquivo no
  Web-GALS e clicar em Gerar Código, não é preciso ajustar nada no diálogo.
- A lista do que cada `#n` faz está no início da seção `#Grammar` do `.vgls`; o `switch` em
  `Semantico::executeAction` segue a mesma numeração.

## Decisões de projeto (já combinadas com o usuário)

- Linguagem **estilo C**. `opus <tipo> nome(...)` devolve valor; `munus nome(...)` é procedimento.
  Funções só no início do programa, antes dos comandos.
- Tamanho de vetor: **só constante** inteira positiva (com cálculo de constantes, `v[2*5]` vale). Sem VLA.
- `catena`: **sem operadores** (nem `+` nem `==`); só atribuição, argumento, `legere`/`inscribe`.
- `reditus` **só dentro de função**; no corpo principal é erro.
- Conversões entre numéricos são livres; `fractio` → `atomus`/`caracter` é **aviso**, não erro.
- O primeiro erro interrompe a compilação (como no léxico/sintático); avisos são acumulados.
- Escopos têm rótulo: `global` (programa e funções), nome da função, `principal`, e blocos numerados por
  função (`principal1`, `fat2`...). A tabela guarda todos os símbolos até o fim (não apaga ao fechar escopo).
- Avisos (não erros): variável não usada / nunca lida, função nunca chamada (chamada recursiva não conta),
  uso sem valor (pela ordem do texto, pode dar
  falso positivo em laços), função sem `reditus`, índice constante fora dos limites, divisão por zero.
  `legere(x)` marca `x` como inicializada, mas não como usada.

## Pendências

- Léxico (opcionais, não pedidos): escapes (`'\n'`), char com acento (`'ç'` é UTF-8 de 2 bytes),
  real sem parte inteira (`.5`), string multilinha é aceita hoje, matriz, inicialização na declaração,
  `pro` com elemento de vetor na inicialização/passo.
- Mensagens sintáticas são só "Erro estado N" (a IDE acrescenta o token encontrado); dá para listar os
  tokens esperados a partir da `PARSER_TABLE`.
- No `.vgls`: o comentário "OBS" antigo e o `#SimulatorText` (sintaxe velha de `opus`) ainda estão lá.
- Tokens que existem mas nunca são aceitos pela gramática: `opta`, `casus`, `ergo`, `:`.

## Convenções

- Código, comentários, mensagens e commits em **português**. Mensagens de erro/aviso citam nomes entre
  aspas simples (`'x'`) e usam os nomes de tipo da linguagem (`atomus`, não `int`).
- Os finais de linha variam entre arquivos (o `casos_de_teste.txt` é CRLF, os gerados pelo GALS são LF);
  preserve o que o arquivo já usa ao editar.
