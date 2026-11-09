#include "GeradorBIP.h"

#include <algorithm>
#include <cctype>
#include <cstdint>

namespace {

const int PRIMEIRO_TEMPORARIO = 1000;   // como nos slides: 1000, 1001, ... fora do .data
const long long IMEDIATO_MIN = -1024;   // o operando imediato do BIP tem 11 bits
const long long IMEDIATO_MAX = 1023;

bool ehUmDe(const std::string &s, std::initializer_list<const char *> opcoes)
{
    for (const char *o : opcoes)
        if (s == o)
            return true;
    return false;
}

std::string minusculas(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

// Nomes que o montador do Bipide entende como instrução não podem virar rótulo
bool ehMnemonico(const std::string &nome)
{
    static const std::set<std::string> mnemonicos = {
        "hlt", "sto", "ld", "ldi", "add", "addi", "sub", "subi", "beq", "bne", "bgt", "bge",
        "blt", "ble", "jmp", "not", "and", "andi", "or", "ori", "xor", "xori", "sll", "srl",
        "ldv", "stov", "call", "return" };
    return mnemonicos.count(minusculas(nome)) > 0;
}

bool cabeNoImediato(long long v)
{
    return v >= IMEDIATO_MIN && v <= IMEDIATO_MAX;
}

std::string aspas(const std::string &s)
{
    return "'" + s + "'";
}

} // namespace

long long GeradorBIP::palavra(long long valor)
{
    return static_cast<int16_t>(static_cast<uint16_t>(valor & 0xFFFF));
}

// ================= estado =================

void GeradorBIP::naoSuportado(const std::string &m, int posicao)
{
    if (falhou)
        return;
    falhou = true;
    motivo = m;
    posicaoMotivo = posicao;
}

// ================= rótulos e memória =================

std::string GeradorBIP::reservarRotulo(const std::string &base, const std::string &escopo)
{
    // Nome original sempre que possível; se repetir (mesmo nome em escopos diferentes)
    // ou coincidir com uma instrução, ganha o escopo como sufixo: x_principal1
    std::string r = base;
    if (rotulosUsados.count(minusculas(r)) || ehMnemonico(r))
        r = base + "_" + (escopo.empty() ? std::string("v") : escopo);
    const std::string semNumero = r;
    for (int n = 2; rotulosUsados.count(minusculas(r)); ++n)
        r = semNumero + std::to_string(n);
    rotulosUsados.insert(minusculas(r));
    return r;
}

std::string GeradorBIP::rotulo(const Simbolo *s)
{
    const auto it = rotulos.find(s);
    if (it != rotulos.end())
        return it->second;
    // Não deveria acontecer (toda variável passa por declarar), mas não quebra a geração
    const std::string r = reservarRotulo(s->nome, s->escopo);
    rotulos[s] = r;
    return r;
}

// Constante fora do intervalo do imediato (-1024..1023): vira uma posição do .data
std::string GeradorBIP::constante(long long valor)
{
    const auto it = constantes.find(valor);
    if (it != constantes.end())
        return it->second;
    const std::string base = valor < 0 ? "const_n" + std::to_string(-valor)
                                       : "const_" + std::to_string(valor);
    const std::string r = reservarRotulo(base, "");
    constantes[valor] = r;
    dadosConstantes.push_back(r + " : " + std::to_string(valor));
    return r;
}

std::string GeradorBIP::novoTemporario()
{
    size_t i = 0;
    while (i < temporarios.size() && temporarios[i])
        ++i;
    if (i == temporarios.size())
        temporarios.push_back(false);
    temporarios[i] = true;
    return std::to_string(PRIMEIRO_TEMPORARIO + static_cast<int>(i));
}

void GeradorBIP::liberar(const Operando &o)
{
    if (o.modo != Operando::Memoria || !o.temporario)
        return;
    const size_t i = static_cast<size_t>(std::stoi(o.endereco) - PRIMEIRO_TEMPORARIO);
    if (i < temporarios.size())
        temporarios[i] = false;
}

// ================= pilha de operandos =================

GeradorBIP::Operando GeradorBIP::desempilhar()
{
    if (pilha.empty()) {
        naoSuportado("Erro interno do gerador de código (pilha de operandos vazia)", -1);
        return Operando{};
    }
    Operando o = pilha.back();
    pilha.pop_back();
    return o;
}

void GeradorBIP::empilharAcumulador()
{
    Operando o;
    o.modo = Operando::Acumulador;
    pilha.push_back(o);
}

GeradorBIP::Operando GeradorBIP::guardarEmTemporario()
{
    Operando o;
    o.modo = Operando::Memoria;
    o.endereco = novoTemporario();
    o.temporario = true;
    emitir("STO", o.endereco);
    return o;
}

// Antes de usar o ACC para outra coisa, o valor que estiver nele vai para um temporário.
// No máximo um operando da pilha está no ACC.
void GeradorBIP::guardarAcumulador()
{
    for (Operando &o : pilha)
        if (o.modo == Operando::Acumulador) {
            o = guardarEmTemporario();
            return;
        }
}

// ================= emissão =================

void GeradorBIP::emitir(const std::string &instrucao, const std::string &operando)
{
    codigo.push_back("   " + instrucao + " " + operando);
}

void GeradorBIP::carregar(const Operando &o)
{
    switch (o.modo) {
    case Operando::Acumulador:
        break;
    case Operando::Imediato:
        if (cabeNoImediato(o.valor))
            emitir("LDI", std::to_string(o.valor));
        else
            emitir("LD", constante(o.valor));
        break;
    case Operando::Memoria:
        emitir("LD", o.endereco);
        liberar(o);
        break;
    }
}

// instrucao = forma com memória (ADD, SUB, AND, OR, XOR); a forma imediata é a mesma + "I"
void GeradorBIP::aplicar(const std::string &instrucao, const Operando &o)
{
    if (o.modo == Operando::Imediato) {
        if (cabeNoImediato(o.valor))
            emitir(instrucao + "I", std::to_string(o.valor));
        else
            emitir(instrucao, constante(o.valor));
    } else {
        emitir(instrucao, o.endereco);
        liberar(o);
    }
}

// ================= declarações =================

void GeradorBIP::declarar(const Simbolo *s, int posicao)
{
    if (falhou)
        return;
    if (s->tipo == Tipo::Fractio || s->tipo == Tipo::Catena) {
        naoSuportado("O processador BIP só trabalha com inteiros: " + aspas(s->nome) + " é " +
                     nomeTipo(s->tipo) + " (use atomus, caracter ou logicum)", posicao);
        return;
    }

    const std::string r = reservarRotulo(s->nome, s->escopo);
    rotulos[s] = r;

    std::string valores = "0";
    for (long long i = 1; s->vetor && i < s->tamanho; ++i)
        valores += ", 0";
    dados.push_back(r + " : " + valores);
}

// ================= operandos e operações =================

void GeradorBIP::imediato(long long valor)
{
    if (falhou)
        return;
    Operando o;
    o.valor = palavra(valor);
    empilhar(o);
}

void GeradorBIP::variavel(const Simbolo *s)
{
    if (falhou)
        return;
    Operando o;
    o.modo = Operando::Memoria;
    o.endereco = rotulo(s);
    empilhar(o);
}

// v[i] em expressão: índice -> $indr, LDV v
void GeradorBIP::elemento(const Simbolo *s)
{
    if (falhou)
        return;
    const Operando indice = desempilhar();
    if (indice.modo != Operando::Acumulador)
        guardarAcumulador();
    carregar(indice);
    emitir("STO", "$indr");
    emitir("LDV", rotulo(s));
    empilharAcumulador();
}

void GeradorBIP::descartar()
{
    if (falhou)
        return;
    liberar(desempilhar());
}

void GeradorBIP::binaria(const std::string &op, int posicao, const TipoExp &resultado)
{
    if (falhou)
        return;

    if (resultado.constante) {
        descartar();
        descartar();
        imediato(resultado.valor);
        return;
    }

    if (ehUmDe(op, { "*", "mult", "/", "divi", "%", "modu" })) {
        naoSuportado("O processador BIP não tem instrução de multiplicação nem de divisão: " +
                     aspas(op) + " não pode ser gerado", posicao);
        return;
    }
    if (ehUmDe(op, { "<", ">", "<=", ">=", "==", "!=", "et", "&&", "vel", "||" })) {
        naoSuportado("Operadores relacionais e lógicos (" + aspas(op) +
                     ") ainda não geram código", posicao);
        return;
    }

    const Operando b = desempilhar();
    const Operando a = desempilhar();
    const bool deslocamento = op == "<<" || op == ">>";

    // ---- deslocamentos: o BIP só desloca por uma quantidade fixa (SLL n / SRL n) ----
    if (deslocamento) {
        if (b.modo != Operando::Imediato) {
            naoSuportado("No BIP o deslocamento só pode ser por uma constante (ex.: x " + op +
                         " 2), porque SLL e SRL recebem a quantidade fixa", posicao);
            return;
        }
        const long long n = b.valor;
        if (n < 0) {
            naoSuportado("Deslocamento por quantidade negativa (" + std::to_string(n) + ")",
                         posicao);
            return;
        }
        if (a.modo == Operando::Imediato || n >= 16) {
            // constante, ou tudo deslocado para fora dos 16 bits
            const uint16_t x = static_cast<uint16_t>(a.valor & 0xFFFF);
            long long r = 0;
            if (a.modo == Operando::Imediato && n < 16)
                r = op == "<<" ? palavra(static_cast<long long>(x) << n) : palavra(x >> n);
            liberar(a);
            imediato(r);
            return;
        }
        if (a.modo != Operando::Acumulador)
            guardarAcumulador();
        carregar(a);
        if (n > 0)
            emitir(op == "<<" ? "SLL" : "SRL", std::to_string(n));
        empilharAcumulador();
        return;
    }

    // ---- soma, subtração e bit a bit ----
    std::string instrucao;
    if (ehUmDe(op, { "+", "adde" }))      instrucao = "ADD";
    else if (ehUmDe(op, { "-", "minu" })) instrucao = "SUB";
    else if (op == "&")                   instrucao = "AND";
    else if (op == "|")                   instrucao = "OR";
    else                                  instrucao = "XOR";   // ^

    // Duas constantes: o resultado é calculado aqui
    if (a.modo == Operando::Imediato && b.modo == Operando::Imediato) {
        long long r;
        if (instrucao == "ADD")      r = a.valor + b.valor;
        else if (instrucao == "SUB") r = a.valor - b.valor;
        else if (instrucao == "AND") r = a.valor & b.valor;
        else if (instrucao == "OR")  r = a.valor | b.valor;
        else                         r = a.valor ^ b.valor;
        imediato(r);
        return;
    }

    if (a.modo == Operando::Acumulador) {
        aplicar(instrucao, b);                       // ACC = a op b
    } else if (b.modo == Operando::Acumulador) {
        if (instrucao != "SUB") {
            aplicar(instrucao, a);                   // comutativa: ACC = b op a
        } else {
            const Operando t = guardarEmTemporario(); // a - ACC: guarda b, carrega a
            carregar(a);
            aplicar("SUB", t);
        }
    } else {
        guardarAcumulador();
        carregar(a);
        aplicar(instrucao, b);
    }
    empilharAcumulador();
}

void GeradorBIP::unaria(const std::string &op, int posicao, const TipoExp &resultado)
{
    if (falhou)
        return;
    if (resultado.constante) {
        descartar();
        imediato(resultado.valor);
        return;
    }
    if (op != "-" && op != "~") {
        naoSuportado("Operadores lógicos (" + aspas(op) + ") ainda não geram código", posicao);
        return;
    }

    const Operando a = desempilhar();
    if (a.modo == Operando::Imediato) {
        imediato(op == "-" ? -a.valor : ~a.valor);
        return;
    }

    if (op == "-") {
        // -x = 0 - x
        if (a.modo == Operando::Acumulador) {
            const Operando t = guardarEmTemporario();
            emitir("LDI", "0");
            aplicar("SUB", t);
        } else {
            guardarAcumulador();
            emitir("LDI", "0");
            aplicar("SUB", a);
        }
    } else {
        if (a.modo != Operando::Acumulador)
            guardarAcumulador();
        carregar(a);
        emitir("NOT", "0");
    }
    empilharAcumulador();
}

// ================= comandos =================

void GeradorBIP::alvoVariavel(const Simbolo *s)
{
    if (falhou)
        return;
    Alvo a;
    a.rotulo = rotulo(s);
    alvos.push_back(a);
}

// v[i] = ...: o índice é calculado antes do valor; se estiver no ACC, vai para um temporário
void GeradorBIP::alvoElemento(const Simbolo *s)
{
    if (falhou)
        return;
    Alvo a;
    a.rotulo = rotulo(s);
    a.vetor = true;
    a.indice = desempilhar();
    if (a.indice.modo == Operando::Acumulador)
        a.indice = guardarEmTemporario();
    alvos.push_back(a);
}

void GeradorBIP::atribuir()
{
    if (falhou)
        return;
    const Operando valor = desempilhar();
    if (alvos.empty()) {
        naoSuportado("Erro interno do gerador de código (atribuição sem destino)", -1);
        return;
    }
    const Alvo alvo = alvos.back();
    alvos.pop_back();

    if (!alvo.vetor) {
        if (valor.modo != Operando::Acumulador)
            guardarAcumulador();
        carregar(valor);
        emitir("STO", alvo.rotulo);
        return;
    }

    // Vetor: $indr <- índice, ACC <- valor, STOV
    if (valor.modo == Operando::Acumulador) {
        const Operando t = guardarEmTemporario();
        carregar(alvo.indice);
        emitir("STO", "$indr");
        carregar(t);
    } else {
        guardarAcumulador();
        carregar(alvo.indice);
        emitir("STO", "$indr");
        carregar(valor);
    }
    emitir("STOV", alvo.rotulo);
}

void GeradorBIP::lerVariavel(const Simbolo *s)
{
    if (falhou)
        return;
    emitir("LD", "$in_port");
    emitir("STO", rotulo(s));
}

void GeradorBIP::lerElemento(const Simbolo *s)
{
    if (falhou)
        return;
    const Operando indice = desempilhar();
    if (indice.modo != Operando::Acumulador)
        guardarAcumulador();
    carregar(indice);
    emitir("STO", "$indr");
    emitir("LD", "$in_port");
    emitir("STOV", rotulo(s));
}

void GeradorBIP::escrever()
{
    if (falhou)
        return;
    const Operando valor = desempilhar();
    if (valor.modo != Operando::Acumulador)
        guardarAcumulador();
    carregar(valor);
    emitir("STO", "$out_port");
}

void GeradorBIP::finalizar()
{
    if (falhou)
        return;
    emitir("HLT", "0");
}

std::string GeradorBIP::assembly() const
{
    if (falhou)
        return {};
    std::string s = ".data\n";
    for (const std::string &d : dados)
        s += "   " + d + "\n";
    for (const std::string &d : dadosConstantes)
        s += "   " + d + "\n";
    s += "\n.text\n_PRINCIPAL:\n";
    for (const std::string &c : codigo)
        s += c + "\n";
    return s;
}
