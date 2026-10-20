#ifndef TABELA_SIMBOLOS_H
#define TABELA_SIMBOLOS_H

#include "Tipos.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

enum class Categoria { Programa, Variavel, Parametro, Funcao };

struct Parametro
{
    Tipo tipo;
    bool vetor;
};

struct Simbolo
{
    std::string nome;
    Categoria categoria = Categoria::Variavel;
    Tipo tipo = Tipo::Atomus;           // em funções: o tipo de retorno (Vazio para munus)
    std::string escopo;                 // rótulo do escopo: global, principal, nome da função, principal1...
    bool vetor = false;
    long long tamanho = 0;              // vetores declarados; 0 em parâmetros vetor
    int posicaoParametro = 0;           // 1, 2, ... nos parâmetros; 0 nos demais
    bool inicializado = false;
    bool usado = false;
    bool avisoSemValor = false;         // evita repetir o aviso de uso sem valor
    int posicao = -1;                   // posição no código-fonte
    std::vector<Parametro> parametros;  // só em funções
};

// "variável", "vetor", "parâmetro", "função", ... (coluna Modalidade da tabela)
std::string modalidade(const Simbolo &s);

// Tabela de símbolos com pilha de escopos.
// Cada símbolo fica guardado até o fim da compilação (para a IDE exibir a tabela);
// a pilha só controla quais nomes estão visíveis. A busca vai do escopo mais
// interno para o mais externo.
class TabelaSimbolos
{
public:
    void abrirEscopo(const std::string &rotulo);
    // Fecha o escopo atual e devolve as variáveis e funções dele que nunca foram usadas
    std::vector<Simbolo> fecharEscopo();

    Simbolo *buscar(const std::string &nome);
    Simbolo *buscarNoEscopoAtual(const std::string &nome);
    // Insere no escopo atual; devolve nullptr se o nome já existe nele
    Simbolo *inserir(const Simbolo &simbolo);

    // Todos os símbolos já declarados, na ordem de declaração
    std::vector<Simbolo> todos() const;

private:
    struct Escopo
    {
        std::string rotulo;
        std::map<std::string, Simbolo *> nomes;
    };
    std::vector<Escopo> escopos;
    std::vector<std::unique_ptr<Simbolo>> simbolos;   // dono dos símbolos
};

#endif
