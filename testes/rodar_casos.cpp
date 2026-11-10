// Executa os casos de casos_de_teste.txt contra o léxico, o sintático e o semântico,
// sem a interface Qt. Uso:
//
//   rodar_casos <casos_de_teste.txt> [-v]    roda todos os casos (-v mostra também os que passam)
//   rodar_casos -p <programa.txt> [entradas]  analisa um programa e mostra o resultado, a tabela
//                                             de símbolos e o assembly; com entradas ("5 3"),
//                                             executa o assembly no simulador do BIP
//
// Formato de cada caso no arquivo:
//
//   === NOME DO CASO
//   ESPERADO: SUCESSO | AVISO (linha N) | ERRO LÉXICO/SINTÁTICO/SEMÂNTICO (linha N)
//   ENTRADA: 5 3            (opcional) valores lidos de $in_port pelo legere
//   SAIDA: 8 2              (opcional) o assembly gerado, executado no simulador do BIP
//                           (testes/simulador_bip.h), deve escrever exatamente isto em $out_port
//   GERACAO: NAO (linha N)  (opcional) o programa compila, mas o assembly não é gerado
//   ---
//   <código até o próximo "===" ou "####">
//
// Além dos casos, confere o catálogo da aba Tokens (CatalogoTokens.cpp) com o léxico e o sintático.
// Termina com código 1 se algum caso falhar.

#include "CatalogoTokens.h"
#include "Lexico.h"
#include "Sintatico.h"
#include "Semantico.h"
#include "LexicalError.h"
#include "SyntacticError.h"
#include "SemanticError.h"
#include "simulador_bip.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
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
    std::string assembly;         // vazio se não foi gerado
    std::string semAssembly;      // motivo de não ter sido gerado
    int linhaSemAssembly = 0;
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

    if (r.fase == "SUCESSO") {
        const GeradorBIP &g = semantico.getGerador();
        if (g.ativo()) {
            r.assembly = g.assembly();
        } else {
            r.semAssembly = g.getMotivo();
            r.linhaSemAssembly = linhaDe(fonte, g.getPosicaoMotivo());
        }
    }
    return r;
}

std::vector<long long> numeros(const std::string &texto)
{
    std::vector<long long> v;
    std::istringstream in(texto);
    long long x;
    while (in >> x)
        v.push_back(x);
    return v;
}

std::string juntar(const std::vector<long long> &v)
{
    std::string s;
    for (long long x : v)
        s += (s.empty() ? "" : " ") + std::to_string(x);
    return s;
}

// Confere ENTRADA/SAIDA/GERACAO; devolve "" se está tudo certo, senão o problema
std::string confereGeracao(const Resultado &r, const std::string &entrada,
                           const std::string &saida, const std::string &geracao)
{
    if (!geracao.empty()) {
        const auto p = geracao.find("linha ");
        const std::string linha = p == std::string::npos ? ""
            : geracao.substr(p + 6, geracao.find(')', p) - p - 6);
        if (!r.assembly.empty())
            return "o assembly foi gerado, mas não deveria";
        if (!linha.empty() && std::to_string(r.linhaSemAssembly) != linha)
            return "assembly não gerado na linha " + std::to_string(r.linhaSemAssembly) +
                   " (esperado na linha " + linha + "): " + r.semAssembly;
        return "";
    }
    if (saida.empty())
        return "";
    if (r.assembly.empty())
        return "assembly não gerado (linha " + std::to_string(r.linhaSemAssembly) + "): " +
               r.semAssembly;

    const ExecucaoBIP e = executarBIP(r.assembly, numeros(entrada));
    if (!e.ok)
        return "simulador do BIP: " + e.erro;
    if (juntar(e.saidas) != juntar(numeros(saida)))
        return "saída do simulador: " + juntar(e.saidas) + " (esperado " +
               juntar(numeros(saida)) + ")";
    return "";
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

// Confere o catálogo de tokens com as tabelas geradas pelo GALS: cada token aparece uma vez, o
// léxico reconhece o exemplo (ou o lexema fixo) como aquele token, os descartados não viram token
// e 'semUso' bate com a PARSER_TABLE. Devolve os problemas encontrados (vazio = ok).
std::vector<std::string> conferirCatalogoTokens()
{
    std::vector<std::string> problemas;
    const std::vector<InfoToken> &catalogo = catalogoTokens();

    // No GALS os não terminais são numerados logo depois do último terminal
    int ultimoToken = PRODUCTIONS[0][0];
    for (const auto &producao : PRODUCTIONS)
        ultimoToken = std::min(ultimoToken, producao[0]);
    --ultimoToken;

    for (int id = t_ID; id <= ultimoToken; ++id) {
        const auto vezes = std::count_if(catalogo.begin(), catalogo.end(),
                                         [&](const InfoToken &t) { return t.id == id; });
        if (vezes != 1)
            problemas.push_back("o token " + std::to_string(id) + " aparece " +
                                std::to_string(vezes) + " vez(es) no catálogo");
    }

    const int estados = sizeof(PARSER_TABLE) / sizeof(PARSER_TABLE[0]);
    for (const InfoToken &t : catalogo) {
        const std::string nome = t.nome ? t.nome : t.significado;
        const std::string texto = t.exemplo ? t.exemplo : t.lexema;
        if (t.id != EPSILON && (t.id < t_ID || t.id > ultimoToken)) {
            problemas.push_back(nome + ": código " + std::to_string(t.id) + " não é de token");
            continue;
        }

        try {
            Lexico lexico(texto.c_str());
            std::unique_ptr<Token> token(lexico.nextToken());
            std::unique_ptr<Token> resto(token ? lexico.nextToken() : nullptr);
            const bool reconhecido = t.id == EPSILON
                ? !token
                : token && token->getId() == t.id && token->getLexeme() == texto && !resto;
            if (!reconhecido)
                problemas.push_back(nome + ": o léxico não reconhece \"" + texto + "\" como esse token");
        } catch (const LexicalError &) {
            problemas.push_back(nome + ": erro léxico em \"" + texto + "\"");
        }

        if (t.id != EPSILON) {
            bool aceito = false;
            for (int estado = 0; estado < estados && !aceito; ++estado)
                aceito = PARSER_TABLE[estado][t.id - 1][0] != ERROR;
            if (aceito == t.semUso)
                problemas.push_back(nome + (t.semUso ? ": marcado como semUso, mas a gramática o aceita"
                                                     : ": nenhuma regra da gramática o aceita; marque semUso"));
        }
    }
    return problemas;
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
        if (!r.assembly.empty()) {
            std::cout << "\nAssembly:\n" << r.assembly;
            if (argc > 3) {
                const ExecucaoBIP e = executarBIP(r.assembly, numeros(argv[3]));
                std::cout << "\nSimulador: " << (e.ok ? "saída = " + juntar(e.saidas) : e.erro)
                          << "\n";
            }
        } else if (!r.semAssembly.empty()) {
            std::cout << "\nAssembly não gerado (linha " << r.linhaSemAssembly << "): "
                      << r.semAssembly << "\n";
        }
        return 0;
    }

    const bool detalhado = argc > 2 && std::string(argv[2]) == "-v";
    std::istringstream entrada(lerArquivo(argv[1]));
    std::string linha, nome, esperado, codigo, entradaBip, saidaBip, geracao;
    bool lendoCodigo = false;
    int ok = 0, falhas = 0;

    auto concluirCaso = [&]() {
        if (nome.empty())
            return;
        while (!codigo.empty() && (codigo.back() == '\n' || codigo.back() == '\r'))
            codigo.pop_back();

        const Resultado r = analisar(codigo);
        const std::string problemaGeracao = confereGeracao(r, entradaBip, saidaBip, geracao);
        const bool passou = confere(esperado, r) && problemaGeracao.empty();
        (passou ? ok : falhas)++;
        if (!passou || detalhado) {
            std::cout << (passou ? "ok    " : "FALHA ") << nome
                      << "\n      esperado: " << esperado
                      << "\n      obtido:   " << descrever(r) << "\n";
            if (!problemaGeracao.empty())
                std::cout << "      geração:  " << problemaGeracao << "\n";
            if (!problemaGeracao.empty() && !r.assembly.empty())
                std::cout << r.assembly << "\n";
        }

        nome.clear();
        codigo.clear();
        entradaBip.clear();
        saidaBip.clear();
        geracao.clear();
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
            else if (linha.rfind("ENTRADA:", 0) == 0)
                entradaBip = linha.substr(8);
            else if (linha.rfind("SAIDA:", 0) == 0)
                saidaBip = linha.substr(6);
            else if (linha.rfind("GERACAO:", 0) == 0)
                geracao = linha.substr(8);
            else if (linha == "---")
                lendoCodigo = true;
        } else if (lendoCodigo) {
            codigo += linha + "\n";
        }
    }
    concluirCaso();

    const std::vector<std::string> problemas = conferirCatalogoTokens();
    (problemas.empty() ? ok : falhas)++;
    if (!problemas.empty() || detalhado)
        std::cout << (problemas.empty() ? "ok    " : "FALHA ") << "catálogo de tokens (CatalogoTokens.cpp)\n";
    for (const std::string &p : problemas)
        std::cout << "      " << p << "\n";

    std::cout << ok << " ok, " << falhas << " falhas\n";
    return falhas == 0 ? 0 : 1;
}
