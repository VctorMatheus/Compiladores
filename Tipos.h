#ifndef TIPOS_H
#define TIPOS_H

#include <string>

// Tipos da linguagem Roman++.
enum class Tipo
{
    Atomus,    // int
    Fractio,   // float
    Caracter,  // char
    Logicum,   // bool
    Catena,    // string
    Vazio      // "retorno" de um munus
};

// Tipo de uma expressão na pilha do semântico.
struct TipoExp
{
    TipoExp(Tipo b = Tipo::Atomus) : base(b) { }

    Tipo base;
    bool vetor = false;        // vetor inteiro, sem índice (só vale como argumento)
    bool constante = false;    // valor conhecido em tempo de compilação (inteiros)
    long long valor = 0;
    std::string nome;          // nome do vetor, para mensagens de erro
};

inline std::string nomeTipo(Tipo t)
{
    switch (t) {
        case Tipo::Atomus:   return "atomus";
        case Tipo::Fractio:  return "fractio";
        case Tipo::Caracter: return "caracter";
        case Tipo::Logicum:  return "logicum";
        case Tipo::Catena:   return "catena";
        case Tipo::Vazio:    return "vazio";
    }
    return "?";
}

// char e bool contam como inteiros (promoção inteira do C)
inline bool ehInteiro(Tipo t)
{
    return t == Tipo::Atomus || t == Tipo::Caracter || t == Tipo::Logicum;
}

inline bool ehNumerico(Tipo t)
{
    return ehInteiro(t) || t == Tipo::Fractio;
}

// As regras de operações e atribuições ficam na tabela de compatibilidade (SemanticTable.h)

#endif
