#ifndef SIMULADOR_BIP_H
#define SIMULADOR_BIP_H

// Simulador mínimo do processador BIP (subconjunto do BIP IV usado pelo gerador), só para os
// testes automáticos: executa o assembly gerado com uma lista de entradas e devolve o que foi
// escrito em $out_port. Não substitui o Bipide: serve para conferir a lógica do código gerado.
//
// Modelo: ACC e dados de 16 bits com sinal, memória de dados de 2048 posições (.data a partir
// do endereço 0), $indr para vetores (LDV/STOV), desvios testam o resultado da última
// operação da ULA (ADD, SUB, lógicas e deslocamentos).

#include <cctype>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct ExecucaoBIP
{
    bool ok = true;
    std::string erro;
    std::vector<long long> saidas;
};

inline ExecucaoBIP executarBIP(const std::string &assembly, const std::vector<long long> &entradas)
{
    struct Instrucao { std::string mnemonico, operando; int linha; };

    ExecucaoBIP r;
    auto falha = [&](const std::string &m, int linha) {
        r.ok = false;
        r.erro = "linha " + std::to_string(linha) + " do assembly: " + m;
        return r;
    };
    auto palavra = [](long long v) {
        return static_cast<long long>(static_cast<int16_t>(static_cast<uint16_t>(v & 0xFFFF)));
    };
    auto aparar = [](std::string s) {
        const auto i = s.find_first_not_of(" \t\r");
        const auto f = s.find_last_not_of(" \t\r");
        return i == std::string::npos ? std::string() : s.substr(i, f - i + 1);
    };
    auto maiusculas = [](std::string s) {
        for (char &c : s)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    };

    std::vector<long long> memoria(2048, 0);
    std::map<std::string, int> dados, rotulos;
    std::vector<Instrucao> programa;

    // ---- montagem ----
    std::istringstream in(assembly);
    std::string linha;
    int numero = 0, proximoDado = 0;
    enum { Nenhuma, Dados, Texto } secao = Nenhuma;
    while (std::getline(in, linha)) {
        ++numero;
        linha = aparar(linha.substr(0, linha.find(';')));
        if (linha.empty())
            continue;
        if (linha == ".data") { secao = Dados; continue; }
        if (linha == ".text") { secao = Texto; continue; }

        if (secao == Dados) {
            const auto dp = linha.find(':');
            if (dp == std::string::npos)
                return falha("declaração sem ':'", numero);
            const std::string nome = aparar(linha.substr(0, dp));
            if (dados.count(nome))
                return falha("rótulo de dado repetido: " + nome, numero);
            dados[nome] = proximoDado;
            std::stringstream valores(linha.substr(dp + 1));
            std::string v;
            while (std::getline(valores, v, ',')) {
                if (proximoDado >= 2048)
                    return falha("memória de dados cheia", numero);
                try {
                    memoria[proximoDado++] = palavra(std::stoll(aparar(v)));
                } catch (...) {
                    return falha("valor inválido: " + v, numero);
                }
            }
        } else if (secao == Texto) {
            if (linha.back() == ':') {
                rotulos[aparar(linha.substr(0, linha.size() - 1))] = static_cast<int>(programa.size());
                continue;
            }
            std::istringstream partes(linha);
            Instrucao i{ "", "", numero };
            partes >> i.mnemonico >> i.operando;
            std::string resto;
            if (i.operando.empty() || (partes >> resto))
                return falha("instrução deve ter exatamente um operando: " + linha, numero);
            i.mnemonico = maiusculas(i.mnemonico);
            programa.push_back(i);
        } else {
            return falha("conteúdo fora de .data/.text", numero);
        }
    }

    // ---- execução ----
    long long acc = 0, indr = 0, ula = 0;
    size_t proximaEntrada = 0;
    std::vector<int> pilhaRetorno;
    int pc = 0;

    auto valorImediato = [&](const Instrucao &i, long long &v) {
        try {
            size_t usados = 0;
            v = std::stoll(i.operando, &usados);
            return usados == i.operando.size();
        } catch (...) {
            return false;
        }
    };
    auto endereco = [&](const Instrucao &i, int &a) {
        long long v;
        if (valorImediato(i, v)) {
            a = static_cast<int>(v);
        } else {
            const auto it = dados.find(i.operando);
            if (it == dados.end())
                return false;
            a = it->second;
        }
        return a >= 0 && a < 2048;
    };

    for (int passos = 0; ; ++passos) {
        if (passos > 200000)
            return falha("execução não terminou (laço infinito?)", 0);
        if (pc < 0 || pc >= static_cast<int>(programa.size()))
            return falha("o programa terminou sem HLT", 0);

        const Instrucao &i = programa[pc];
        const std::string &m = i.mnemonico;
        long long imediato = 0;
        int a = 0;
        int proximo = pc + 1;

        if (m == "HLT") {
            break;
        } else if (m == "LD") {
            if (i.operando == "$in_port") {
                if (proximaEntrada >= entradas.size())
                    return falha("leitura de $in_port sem entrada disponível", i.linha);
                acc = palavra(entradas[proximaEntrada++]);
            } else if (i.operando == "$indr") {
                acc = indr;
            } else if (endereco(i, a)) {
                acc = memoria[a];
            } else {
                return falha("endereço inválido: " + i.operando, i.linha);
            }
        } else if (m == "STO") {
            if (i.operando == "$out_port")
                r.saidas.push_back(acc);
            else if (i.operando == "$indr")
                indr = acc;
            else if (endereco(i, a))
                memoria[a] = acc;
            else
                return falha("endereço inválido: " + i.operando, i.linha);
        } else if (m == "LDV" || m == "STOV") {
            if (!endereco(i, a) || a + indr < 0 || a + indr >= 2048)
                return falha("acesso a vetor fora da memória: " + i.operando + "[" +
                             std::to_string(indr) + "]", i.linha);
            if (m == "LDV")
                acc = memoria[a + indr];
            else
                memoria[a + indr] = acc;
        } else if (m == "LDI" || m == "ADDI" || m == "SUBI" || m == "ANDI" || m == "ORI" ||
                   m == "XORI" || m == "SLL" || m == "SRL" || m == "NOT") {
            if (!valorImediato(i, imediato))
                return falha(m + " exige um valor imediato: " + i.operando, i.linha);
            if (imediato < -1024 || imediato > 1023)
                return falha("imediato fora de -1024..1023: " + i.operando, i.linha);
            const uint16_t u = static_cast<uint16_t>(acc & 0xFFFF);
            if (m == "LDI")       acc = imediato;
            else if (m == "ADDI") acc = palavra(acc + imediato);
            else if (m == "SUBI") acc = palavra(acc - imediato);
            else if (m == "ANDI") acc = palavra(acc & imediato);
            else if (m == "ORI")  acc = palavra(acc | imediato);
            else if (m == "XORI") acc = palavra(acc ^ imediato);
            else if (m == "SLL")  acc = palavra(static_cast<long long>(u) << imediato);
            else if (m == "SRL")  acc = palavra(u >> imediato);
            else                  acc = palavra(~acc);
            if (m != "LDI")
                ula = acc;
        } else if (m == "ADD" || m == "SUB" || m == "AND" || m == "OR" || m == "XOR") {
            if (!endereco(i, a))
                return falha("endereço inválido: " + i.operando, i.linha);
            const long long x = memoria[a];
            if (m == "ADD")      acc = palavra(acc + x);
            else if (m == "SUB") acc = palavra(acc - x);
            else if (m == "AND") acc = palavra(acc & x);
            else if (m == "OR")  acc = palavra(acc | x);
            else                 acc = palavra(acc ^ x);
            ula = acc;
        } else if (m == "JMP" || m == "BEQ" || m == "BNE" || m == "BGT" || m == "BGE" ||
                   m == "BLT" || m == "BLE" || m == "CALL") {
            const auto it = rotulos.find(i.operando);
            if (it == rotulos.end())
                return falha("rótulo inexistente: " + i.operando, i.linha);
            const bool desvia = m == "JMP" || m == "CALL" || (m == "BEQ" && ula == 0) ||
                                (m == "BNE" && ula != 0) || (m == "BGT" && ula > 0) ||
                                (m == "BGE" && ula >= 0) || (m == "BLT" && ula < 0) ||
                                (m == "BLE" && ula <= 0);
            if (m == "CALL")
                pilhaRetorno.push_back(pc + 1);
            if (desvia)
                proximo = it->second;
        } else if (m == "RETURN") {
            if (pilhaRetorno.empty())
                return falha("RETURN sem CALL", i.linha);
            proximo = pilhaRetorno.back();
            pilhaRetorno.pop_back();
        } else {
            return falha("instrução desconhecida: " + m, i.linha);
        }
        pc = proximo;
    }
    return r;
}

#endif
