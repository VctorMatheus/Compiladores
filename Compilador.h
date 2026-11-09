#ifndef COMPILADOR_H
#define COMPILADOR_H

#include "TabelaSimbolos.h"

#include <QString>

#include <vector>

// Ponte entre a IDE e os analisadores gerados pelo GALS.
// A interface só conhece esta classe, então trocar/estender as fases
// (ex.: adicionar o semântico) não exige mexer na janela.
struct ResultadoCompilacao
{
    bool sucesso = false;
    QString mensagem;
    int linha = 0;   // 1-based; 0 quando não se aplica
    int coluna = 0;  // 1-based; 0 quando não se aplica
    int avisos = 0;  // avisos do semântico (listados na mensagem)
    // Tabela de símbolos montada pelo semântico (parcial se houve erro)
    std::vector<Simbolo> simbolos;
    // Assembly do BIP (vazio se houve erro ou se o programa usa algo que ainda não gera código;
    // nesse caso semAssembly explica o motivo, com linha e coluna)
    QString assembly;
    QString semAssembly;
};

class Compilador
{
public:
    ResultadoCompilacao compilar(const QString &codigoFonte);
};

#endif
