# Roman++

Compilador e IDE para a linguagem **Roman++**, uma linguagem com palavras-chave em latim
(`principium`, `si`, `dum`, `opus`, `terminus`...). Projeto da disciplina de Compiladores.

Os analisadores léxico e sintático (SLR) são gerados pelo [Web-GALS](https://lia-univali.github.io/Web-GALS/)
a partir de [`Roman++.vgls`](Roman++.vgls); a IDE é feita em C++ com Qt 6 Widgets.

## Exemplo

```
principium
  opus dobro(atomus n) sic
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
| `Lexico.*`, `Sintatico.*`, `Semantico.*`, `Constants.*`, `Token.h`, `*Error.h` | Código gerado pelo GALS |
| `Compilador.*` | Ponte entre a IDE e os analisadores (erros → linha/coluna) |
| `MainWindow.*`, `main.cpp` | Interface: editor, botão Compilar e área de saída |
| `fontes/` | Fontes embutidas (Cinzel, Marcellus SC, Almendra, Alegreya — licença SIL OFL) |
| `imagens/` | Fundo de papiro e o script Python que o gera |
| `casos_de_teste.txt` | Cenários de sucesso e de erro léxico/sintático |

## Como compilar

Requisitos: Qt 6 (kit MinGW 64-bit), CMake e Ninja — todos instaláveis pelo Qt Online Installer.

**Qt Creator:** *File → Open File or Project* → `CMakeLists.txt` → kit *Desktop Qt 6 MinGW 64-bit* → `Ctrl+R`.

**Terminal:**

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH=C:/Qt/6.11.2/mingw_64
cmake --build build
```

O build roda o `windeployqt` automaticamente, então `build/CompiladorIDE.exe` abre direto pelo Explorer.

## Regenerando os analisadores

Ao gerar pelo Web-GALS, use **linguagem C++** com geração de **analisador léxico** ativada e substitua os
arquivos gerados na raiz. Atenção: o `Semantico.cpp` é sobrescrito — salve sua implementação antes.
