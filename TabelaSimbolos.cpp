#include "TabelaSimbolos.h"

#include <algorithm>

std::string modalidade(const Simbolo &s)
{
    switch (s.categoria) {
        case Categoria::Programa:  return "programa";
        case Categoria::Funcao:    return s.tipo == Tipo::Vazio ? "procedimento" : "função";
        case Categoria::Parametro: return s.vetor ? "parâmetro vetor" : "parâmetro";
        case Categoria::Variavel:  return s.vetor ? "vetor" : "variável";
    }
    return "?";
}

void TabelaSimbolos::abrirEscopo(const std::string &rotulo)
{
    escopos.push_back({ rotulo, {} });
}

std::vector<Simbolo> TabelaSimbolos::fecharEscopo()
{
    std::vector<Simbolo> naoUsados;
    if (escopos.empty())
        return naoUsados;

    for (const auto &par : escopos.back().nomes) {
        const Simbolo &s = *par.second;
        const bool verificar = s.categoria == Categoria::Variavel || s.categoria == Categoria::Funcao;
        if (verificar && !s.usado)
            naoUsados.push_back(s);
    }
    std::sort(naoUsados.begin(), naoUsados.end(),
              [](const Simbolo &a, const Simbolo &b) { return a.posicao < b.posicao; });

    escopos.pop_back();
    return naoUsados;
}

Simbolo *TabelaSimbolos::buscar(const std::string &nome)
{
    for (auto it = escopos.rbegin(); it != escopos.rend(); ++it) {
        auto achado = it->nomes.find(nome);
        if (achado != it->nomes.end())
            return achado->second;
    }
    return nullptr;
}

Simbolo *TabelaSimbolos::buscarNoEscopoAtual(const std::string &nome)
{
    if (escopos.empty())
        return nullptr;
    auto achado = escopos.back().nomes.find(nome);
    return achado != escopos.back().nomes.end() ? achado->second : nullptr;
}

Simbolo *TabelaSimbolos::inserir(const Simbolo &simbolo)
{
    if (escopos.empty() || buscarNoEscopoAtual(simbolo.nome))
        return nullptr;

    simbolos.push_back(std::make_unique<Simbolo>(simbolo));
    Simbolo *s = simbolos.back().get();
    s->escopo = escopos.back().rotulo;
    escopos.back().nomes[s->nome] = s;
    return s;
}

std::vector<Simbolo> TabelaSimbolos::todos() const
{
    std::vector<Simbolo> lista;
    lista.reserve(simbolos.size());
    for (const auto &s : simbolos)
        lista.push_back(*s);
    return lista;
}
