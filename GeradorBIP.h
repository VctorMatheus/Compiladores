#ifndef GERADOR_BIP_H
#define GERADOR_BIP_H

#include "TabelaSimbolos.h"

#include <map>
#include <set>
#include <string>
#include <vector>

// Geração de código assembly para o processador BIP (executável no Bipide 3.0).
//
// É dirigida pelas mesmas ações semânticas do Semantico, que chama os métodos abaixo depois de
// validar cada construção. Como a análise é bottom-up, o gerador mantém uma pilha de operandos
// paralela à pilha de tipos do Semantico:
//
//  - literais e variáveis entram "preguiçosos" (sem gerar código): o código só sai quando o
//    operador é aplicado, o que permite usar as formas imediatas (x + 1 -> LD x / ADDI 1);
//  - o resultado de uma subexpressão fica no acumulador (ACC) e só é guardado num temporário
//    (endereços 1000, 1001, ..., fora do .data, como nos slides) quando outra conta precisa do ACC.
//
// O que o BIP (ou esta etapa do trabalho) não suporta não interrompe a compilação: o gerador
// guarda o primeiro motivo, para de gerar, e a IDE mostra o motivo no lugar do assembly.
class GeradorBIP
{
public:
    // ---- estado ----
    void naoSuportado(const std::string &motivo, int posicao);
    bool ativo() const { return !falhou; }
    const std::string &getMotivo() const { return motivo; }
    int getPosicaoMotivo() const { return posicaoMotivo; }

    // ---- declarações (.data) ----
    void declarar(const Simbolo *s, int posicao);

    // ---- operandos e operações ----
    void imediato(long long valor);
    void variavel(const Simbolo *s);        // variável simples (ou vetor inteiro, como argumento)
    void elemento(const Simbolo *s);        // v[i]: o índice está no topo da pilha
    void descartar();                       // operando que não vira código (tamanho de vetor)
    // Se o Semantico já calculou o valor (operandos constantes, ex.: v[2 * 5]), o resultado entra
    // como constante, mesmo para operações que o BIP não tem (*, /, relacionais...)
    void binaria(const std::string &op, int posicao, const TipoExp &resultado);
    void unaria(const std::string &op, int posicao, const TipoExp &resultado);

    // ---- comandos ----
    void alvoVariavel(const Simbolo *s);
    void alvoElemento(const Simbolo *s);    // índice no topo da pilha
    void atribuir();
    void lerVariavel(const Simbolo *s);
    void lerElemento(const Simbolo *s);     // índice no topo da pilha
    void escrever();
    void finalizar();

    // Programa completo (.data + .text); vazio se a geração não foi possível
    std::string assembly() const;

    // Valor como palavra de 16 bits com sinal (o BIP trabalha com 16 bits)
    static long long palavra(long long valor);

private:
    struct Operando
    {
        enum Modo { Imediato, Memoria, Acumulador } modo = Imediato;
        long long valor = 0;       // Imediato
        std::string endereco;      // Memoria: rótulo do .data ou endereço de temporário
        bool temporario = false;   // Memoria: libera o temporário ao ser consumido
    };
    struct Alvo
    {
        std::string rotulo;
        bool vetor = false;
        Operando indice;           // vetor: nunca fica no ACC (é guardado ao entrar)
    };

    bool falhou = false;
    std::string motivo;
    int posicaoMotivo = -1;

    std::vector<std::string> dados;           // linhas do .data (variáveis e vetores)
    std::vector<std::string> dadosConstantes; // constantes que não cabem no imediato
    std::vector<std::string> codigo;          // linhas do .text
    std::map<const Simbolo *, std::string> rotulos;
    std::set<std::string> rotulosUsados;      // em minúsculas
    std::map<long long, std::string> constantes;
    std::vector<bool> temporarios;            // true = ocupado

    std::vector<Operando> pilha;
    std::vector<Alvo> alvos;

    // rótulos e memória
    std::string reservarRotulo(const std::string &base, const std::string &escopo);
    std::string rotulo(const Simbolo *s);
    std::string constante(long long valor);
    std::string novoTemporario();
    void liberar(const Operando &o);

    // pilha de operandos
    void empilhar(const Operando &o) { pilha.push_back(o); }
    Operando desempilhar();
    void empilharAcumulador();
    void guardarAcumulador();               // tira da pilha qualquer valor que esteja no ACC

    // emissão
    void emitir(const std::string &instrucao, const std::string &operando);
    void carregar(const Operando &o);       // ACC <- o
    void aplicar(const std::string &instrucao, const Operando &o);   // ACC <- ACC op o
    Operando guardarEmTemporario();         // ACC -> temporário
};

#endif
