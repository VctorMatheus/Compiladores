// Executa os casos de casos_de_teste.txt contra o léxico, o sintático e o semântico,
// sem a interface Qt. Uso:
//
//   rodar_casos <casos_de_teste.txt> [-v]    roda todos os casos (-v mostra também os que passam)
//   rodar_casos -p <programa.txt>             analisa um programa e mostra o resultado
//
// Formato de cada caso no arquivo:
//
//   === NOME DO CASO
//   ESPERADO: SUCESSO | AVISO (linha N) | ERRO LÉXICO/SINTÁTICO/SEMÂNTICO (linha N)
//   ---
//   <código até o próximo "===" ou "####">
//
// Termina com código 1 se algum caso falhar.

#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"
#include "LexicalError.h"
#include "SyntacticError.h"
#include "SemanticError.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Resultado
{
    std::string fase = "SUCESSO";   // SUCESSO, LEXICO, SINTATICO ou SEMANTICO
    int linha = 0;
    std::string mensagem;
    std::vector<std::pair<int, std::string>> avisos;   // (linha, mensagem)
    std::vector<Simbolo> simbolos;
};

int linhaDe(const std::string &fonte, int posicao)
{
    int linha = 1;
    for (int i = 0; i < posicao && i < static_cast<int>(fonte.size()); ++i)
        if (fonte[i] == '\n')
            ++linha;
    return linha;
}

Resultado analisar(const std::string &fonte)
{
    Lexico lexico(fonte.c_str());
    Sintatico sintatico;
    Semantico semantico;
    Resultado r;

    auto falhou = [&](const char *fase, const AnalysisError &e) {
        r.fase = fase;
        r.linha = linhaDe(fonte, e.getPosition());
        r.mensagem = e.getMessage();
    };

    try {
        sintatico.parse(&lexico, &semantico);
    } catch (const LexicalError &e) {
        falhou("LEXICO", e);
    } catch (const SyntacticError &e) {
        falhou("SINTATICO", e);
    } catch (const SemanticError &e) {
        falhou("SEMANTICO", e);
    }

    for (const Aviso &a : semantico.getAvisos())
        r.avisos.push_back({ linhaDe(fonte, a.posicao), a.mensagem });
    std::stable_sort(r.avisos.begin(), r.avisos.end(),
                     [](const auto &a, const auto &b) { return a.first < b.first; });
    r.simbolos = semantico.getSimbolos();
    return r;
}

std::string descrever(const Resultado &r)
{
    std::string s = r.fase;
    if (r.fase != "SUCESSO")
        s += " linha " + std::to_string(r.linha) + ": " + r.mensagem;
    for (const auto &a : r.avisos)
        s += "\n                aviso linha " + std::to_string(a.first) + ": " + a.second;
    return s;
}

// Compara o resultado com a linha "ESPERADO: ..."
bool confere(const std::string &esperado, const Resultado &r)
{
    std::string linha;
    const auto p = esperado.find("linha ");
    if (p != std::string::npos) {
        linha = esperado.substr(p + 6);
        linha = linha.substr(0, linha.find(')'));
    }

    if (esperado.rfind("SUCESSO", 0) == 0)
        return r.fase == "SUCESSO";

    if (esperado.rfind("AVISO", 0) == 0)
        return r.fase == "SUCESSO" &&
               std::any_of(r.avisos.begin(), r.avisos.end(),
                           [&](const auto &a) { return std::to_string(a.first) == linha; });

    // ERRO LÉXICO / SINTÁTICO / SEMÂNTICO (comparação sem depender dos acentos)
    std::string fase = "SINTATICO";
    if (esperado.find("XICO") != std::string::npos)
        fase = "LEXICO";
    else if (esperado.find("SEM") != std::string::npos)
        fase = "SEMANTICO";
    return r.fase == fase && (linha.empty() || std::to_string(r.linha) == linha);
}

std::string lerArquivo(const char *caminho)
{
    std::ifstream f(caminho, std::ios::binary);
    if (!f) {
        std::cerr << "Não foi possível abrir " << caminho << "\n";
        std::exit(2);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

} // namespace

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "uso: rodar_casos <casos_de_teste.txt> [-v]  |  rodar_casos -p <programa.txt>\n";
        return 2;
    }

    if (std::string(argv[1]) == "-p" && argc > 2) {
        const Resultado r = analisar(lerArquivo(argv[2]));
        std::cout << descrever(r) << "\n\nTabela de símbolos:\n";
        for (const Simbolo &s : r.simbolos)
            std::cout << "  " << s.nome << " | " << nomeTipo(s.tipo) << " | " << modalidade(s)
                      << " | " << s.escopo << " | ini=" << s.inicializado << " usado=" << s.usado
                      << " | param=" << s.posicaoParametro << " | tam=" << s.tamanho << "\n";
        return 0;
    }

    const bool detalhado = argc > 2 && std::string(argv[2]) == "-v";
    std::istringstream entrada(lerArquivo(argv[1]));
    std::string linha, nome, esperado, codigo;
    bool lendoCodigo = false;
    int ok = 0, falhas = 0;

    auto concluirCaso = [&]() {
        if (nome.empty())
            return;
        while (!codigo.empty() && (codigo.back() == '\n' || codigo.back() == '\r'))
            codigo.pop_back();

        const Resultado r = analisar(codigo);
        const bool passou = confere(esperado, r);
        (passou ? ok : falhas)++;
        if (!passou || detalhado)
            std::cout << (passou ? "ok    " : "FALHA ") << nome
                      << "\n      esperado: " << esperado
                      << "\n      obtido:   " << descrever(r) << "\n";

        nome.clear();
        codigo.clear();
        lendoCodigo = false;
    };

    while (std::getline(entrada, linha)) {
        if (!linha.empty() && linha.back() == '\r')
            linha.pop_back();

        if (linha.rfind("=== ", 0) == 0) {
            concluirCaso();
            nome = linha.substr(4);
        } else if (linha.rfind("####", 0) == 0) {
            concluirCaso();
        } else if (!nome.empty() && !lendoCodigo) {
            if (linha.rfind("ESPERADO:", 0) == 0)
                esperado = linha.substr(10);
            else if (linha == "---")
                lendoCodigo = true;
        } else if (lendoCodigo) {
            codigo += linha + "\n";
        }
    }
    concluirCaso();

    std::cout << ok << " ok, " << falhas << " falhas\n";
    return falhas == 0 ? 0 : 1;
}
