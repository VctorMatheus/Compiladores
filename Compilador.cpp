#include "Compilador.h"

#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"
#include "LexicalError.h"
#include "SyntacticError.h"
#include "SemanticError.h"

#include <algorithm>
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

    Lexico lexico(fonte.c_str());
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

    Lexico lexico(fonte.c_str());
    Sintatico sintatico;
    Semantico semantico;

    try {
        sintatico.parse(&lexico, &semantico);
        r.sucesso = true;

        // Avisos chegam fora de ordem (ex.: "nunca usada" só ao fechar o escopo)
        std::vector<Aviso> avisos = semantico.getAvisos();
        std::stable_sort(avisos.begin(), avisos.end(),
                         [](const Aviso &a, const Aviso &b) { return a.posicao < b.posicao; });
        r.avisos = static_cast<int>(avisos.size());

        if (avisos.empty()) {
            r.mensagem = QStringLiteral("Programa compilado com sucesso!");
        } else {
            r.mensagem = QStringLiteral("Programa compilado com sucesso, com %1 aviso(s):\n")
                             .arg(avisos.size());
            for (const Aviso &a : avisos) {
                int linha, coluna;
                calcularLinhaColuna(fonte, a.posicao, linha, coluna);
                r.mensagem += QStringLiteral("\nAviso na linha %1, coluna %2: %3")
                                  .arg(linha).arg(coluna).arg(QString::fromStdString(a.mensagem));
            }
        }
    } catch (const LexicalError &e) {
        calcularLinhaColuna(fonte, e.getPosition(), r.linha, r.coluna);
        QString msg = QString::fromUtf8(e.getMessage());
        // O comentário de bloco é um token sem nome (":!" no .vgls), e o GALS gera para ele
        // a mensagem "Erro identificando <ignorar>"; aqui ela vira algo que o usuário entende.
        if (e.getPosition() >= 0 && fonte.compare(e.getPosition(), 2, "/*") == 0)
            msg = QStringLiteral("Comentário de bloco /* não foi fechado (falta */)");
        r.mensagem = formatarErro(QStringLiteral("léxico"), msg, r.linha, r.coluna);
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

    r.simbolos = semantico.getSimbolos();
    return r;
}
