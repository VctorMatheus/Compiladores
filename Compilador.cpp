#include "Compilador.h"

#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"
#include "LexicalError.h"
#include "SyntacticError.h"
#include "SemanticError.h"

#include <algorithm>
#include <sstream>
#include <memory>

namespace {

// Converte a posição (offset em bytes UTF-8) usada pelo GALS em linha/coluna.
void calcularLinhaColuna(const std::string &fonte, int posicao, int &linha, int &coluna)
{
    if (posicao < 0) {
        linha = coluna = 0;
        return;
    }
    size_t pos = std::min(static_cast<size_t>(posicao), fonte.size());

    linha = 1;
    size_t inicioLinha = 0;
    for (size_t i = 0; i < pos; ++i) {
        if (fonte[i] == '\n') {
            ++linha;
            inicioLinha = i + 1;
        }
    }
    // Conta caracteres (não bytes) para acentos não deslocarem a coluna
    coluna = QString::fromUtf8(fonte.data() + inicioLinha,
                               static_cast<int>(pos - inicioLinha)).size() + 1;
}

// Lexema do token que começa em 'posicao', para enriquecer erros sintáticos.
QString lexemaEm(const std::string &fonte, int posicao)
{
    if (posicao < 0 || static_cast<size_t>(posicao) >= fonte.size())
        return QStringLiteral("fim do programa");

    std::istringstream in(fonte);
    Lexico lexico(in);
    lexico.setPosition(static_cast<unsigned>(posicao));
    try {
        std::unique_ptr<Token> token(lexico.nextToken());
        if (token)
            return QStringLiteral("'%1'").arg(QString::fromStdString(token->getLexeme()));
    } catch (const LexicalError &) {
    }
    return QStringLiteral("fim do programa");
}

QString formatarErro(const QString &fase, const QString &msg, int linha, int coluna)
{
    if (linha > 0)
        return QStringLiteral("Erro %1 na linha %2, coluna %3: %4")
            .arg(fase).arg(linha).arg(coluna).arg(msg);
    return QStringLiteral("Erro %1: %2").arg(fase, msg);
}

} // namespace

ResultadoCompilacao Compilador::compilar(const QString &codigoFonte)
{
    ResultadoCompilacao r;
    const std::string fonte = codigoFonte.toStdString();

    std::istringstream in(fonte);
    Lexico lexico(in);
    Sintatico sintatico;
    Semantico semantico;

    try {
        sintatico.parse(&lexico, &semantico);
        r.sucesso = true;
        r.mensagem = QStringLiteral("Programa compilado com sucesso!");
    } catch (const LexicalError &e) {
        calcularLinhaColuna(fonte, e.getPosition(), r.linha, r.coluna);
        r.mensagem = formatarErro(QStringLiteral("léxico"),
                                  QString::fromUtf8(e.getMessage()), r.linha, r.coluna);
    } catch (const SyntacticError &e) {
        calcularLinhaColuna(fonte, e.getPosition(), r.linha, r.coluna);
        QString msg = QStringLiteral("%1 (encontrado %2)")
                          .arg(QString::fromUtf8(e.getMessage()), lexemaEm(fonte, e.getPosition()));
        r.mensagem = formatarErro(QStringLiteral("sintático"), msg, r.linha, r.coluna);
    } catch (const SemanticError &e) {
        calcularLinhaColuna(fonte, e.getPosition(), r.linha, r.coluna);
        r.mensagem = formatarErro(QStringLiteral("semântico"),
                                  QString::fromUtf8(e.getMessage()), r.linha, r.coluna);
    }

    return r;
}
