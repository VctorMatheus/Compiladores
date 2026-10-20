#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "Token.h"
#include "SemanticError.h"
#include "TabelaSimbolos.h"
#include "Tipos.h"

#include <string>
#include <vector>

// ATENÇÃO: o GALS também gera Semantico.h/.cpp. Ao regenerar pelo Web-GALS,
// copie só Constants.* (e Lexico/Sintatico, se mudarem) para não perder este código.

struct Aviso
{
    std::string mensagem;
    int posicao;
};

class Semantico
{
public:
    // Erros lançam SemanticError (o primeiro erro interrompe a compilação);
    // avisos são acumulados e consultados ao final.
    void executeAction(int action, const Token *token);

    const std::vector<Aviso> &getAvisos() const { return avisos; }

    // Tabela de símbolos completa (inclusive escopos já fechados), para a IDE
    std::vector<Simbolo> getSimbolos() const { return tabela.todos(); }

private:
    struct Identificador { Simbolo *simbolo; int posicao; };
    struct Operador { std::string lexema; int posicao; };
    struct Chamada { Simbolo *funcao; size_t argumentos; int posicao; };
    struct Alvo { Simbolo *simbolo; Tipo tipo; int posicao; };

    TabelaSimbolos tabela;
    std::vector<Aviso> avisos;

    // Declaração em andamento (#1 a #4)
    Tipo tipoAtual = Tipo::Atomus;
    std::string nomePendente;
    int posicaoPendente = -1;
    bool vetorPendente = false;
    long long tamanhoPendente = 0;

    // Expressões, chamadas e atribuições
    std::vector<TipoExp> pilhaTipos;
    std::vector<Operador> pilhaOperadores;
    std::vector<Identificador> pilhaIds;
    std::vector<Chamada> pilhaChamadas;
    std::vector<Alvo> pilhaAlvos;

    // Contexto
    std::string escopoBase;     // função atual ou "principal": prefixo do rótulo dos blocos
    int contadorBlocos = 0;
    int profundidadeLaco = 0;
    Simbolo *funcaoAtual = nullptr;
    bool funcaoTeveRetorno = false;

    // Parâmetro em andamento (#72 a #74)
    std::string nomeParametro;
    int posicaoParametro = -1;
    bool parametroVetor = false;

    // Declarações e escopos
    void declararVariavel();
    void abrirEscopo(const std::string &rotulo);
    void abrirBloco();
    void fecharEscopo();

    // Identificadores
    Simbolo *buscar(const std::string &nome, int posicao);
    Identificador desempilharId();
    void usarVariavel(const Token *token);
    void usarElemento();
    void verificarIndice(const Token *token);

    // Chamadas
    void iniciarChamada();
    void argumento(const Token *token);
    void finalizarChamada(bool emExpressao, const Token *token);

    // Expressões
    void literalInteiro(const Token *token);
    void operacaoBinaria();
    void operacaoUnaria();
    TipoExp desempilharTipo();
    Operador desempilharOperador();

    // Atribuição e E/S
    void alvoVariavel();
    void alvoElemento();
    void concluirAtribuicao();
    void legere(bool elemento);
    void itemInscribe(const Token *token);

    // Controle de fluxo
    void condicao(const Token *token);

    // Funções
    void iniciarFuncao(const Token *token);
    void inserirParametro();
    void finalizarFuncao();
    void reditusComValor(const Token *token);
    void reditusSemValor(const Token *token);

    // Utilitários
    void verificarCompatibilidade(Tipo destino, const TipoExp &origem,
                                  const std::string &contexto, int posicao);
    [[noreturn]] void erro(const std::string &mensagem, int posicao);
    void aviso(const std::string &mensagem, int posicao);
};

#endif
