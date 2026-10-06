#ifndef COMPILADOR_H
#define COMPILADOR_H

#include <QString>

// Ponte entre a IDE e os analisadores gerados pelo GALS.
// A interface só conhece esta classe, então trocar/estender as fases
// (ex.: adicionar o semântico) não exige mexer na janela.
struct ResultadoCompilacao
{
    bool sucesso = false;
    QString mensagem;
    int linha = 0;   // 1-based; 0 quando não se aplica
    int coluna = 0;  // 1-based; 0 quando não se aplica
};

class Compilador
{
public:
    ResultadoCompilacao compilar(const QString &codigoFonte);
};

#endif
