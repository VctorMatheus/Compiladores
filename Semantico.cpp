#include "Semantico.h"
#include "Constants.h"
#include "SemanticTable.h"

#include <stdexcept>

// Os números das ações estão documentados no início da seção #Grammar do Roman++.vgls.

namespace {

bool ehUmDe(const std::string &s, std::initializer_list<const char *> opcoes)
{
    for (const char *o : opcoes)
        if (s == o)
            return true;
    return false;
}

std::string entreAspas(const std::string &s)
{
    return "'" + s + "'";
}

// ---- ponte entre os tipos da linguagem e a tabela de compatibilidade (SemanticTable.h) ----

// -1 para vazio: não existe na tabela, e resultType/atribType devolvem ERR
int tipoTabela(Tipo t)
{
    switch (t) {
        case Tipo::Atomus:   return SemanticTable::INT;
        case Tipo::Fractio:  return SemanticTable::FLO;
        case Tipo::Caracter: return SemanticTable::CHA;
        case Tipo::Catena:   return SemanticTable::STR;
        case Tipo::Logicum:  return SemanticTable::BOO;
        case Tipo::Vazio:    return -1;
    }
    return -1;
}

Tipo tipoLinguagem(int t)
{
    switch (t) {
        case SemanticTable::FLO: return Tipo::Fractio;
        case SemanticTable::CHA: return Tipo::Caracter;
        case SemanticTable::STR: return Tipo::Catena;
        case SemanticTable::BOO: return Tipo::Logicum;
        default:                 return Tipo::Atomus;
    }
}

// Lexema de um operador binário -> coluna da tabela
int operacaoTabela(const std::string &o)
{
    if (ehUmDe(o, { "+", "adde" })) return SemanticTable::SUM;
    if (ehUmDe(o, { "-", "minu" })) return SemanticTable::SUB;
    if (ehUmDe(o, { "*", "mult" })) return SemanticTable::MUL;
    if (ehUmDe(o, { "/", "divi" })) return SemanticTable::DIV;
    if (ehUmDe(o, { "%", "modu" })) return SemanticTable::MOD;
    if (ehUmDe(o, { "et", "&&" }))  return SemanticTable::AND;
    if (ehUmDe(o, { "vel", "||" })) return SemanticTable::OR_;
    if (ehUmDe(o, { "<", ">", "<=", ">=", "==", "!=" })) return SemanticTable::REL;
    return SemanticTable::BIT;   // & | ^ << >>
}

// O que a operação exige, para explicar o erro ao usuário
std::string exigencia(int operacao, bool unaria)
{
    switch (operacao) {
        case SemanticTable::MOD:
        case SemanticTable::BIT:
            return unaria ? "exige um valor inteiro" : "exige operandos inteiros";
        case SemanticTable::AND:
        case SemanticTable::OR_:
            return unaria ? "exige um valor escalar" : "exige valores escalares";
        default:
            return unaria ? "exige um valor numérico" : "exige operandos numéricos";
    }
}

// Valor de uma operação entre constantes inteiras (usado no tamanho de vetores, ex.: v[2 * 5])
long long calcular(const std::string &o, long long x, long long y)
{
    if (ehUmDe(o, { "+", "adde" })) return x + y;
    if (ehUmDe(o, { "-", "minu" })) return x - y;
    if (ehUmDe(o, { "*", "mult" })) return x * y;
    if (ehUmDe(o, { "/", "divi" })) return x / y;
    if (ehUmDe(o, { "%", "modu" })) return x % y;
    if (ehUmDe(o, { "et", "&&" }))  return x && y;
    if (ehUmDe(o, { "vel", "||" })) return x || y;
    if (o == "|")  return x | y;
    if (o == "^")  return x ^ y;
    if (o == "&")  return x & y;
    if (o == "<<") return (y >= 0 && y < 63) ? (x << y) : 0;
    if (o == ">>") return (y >= 0 && y < 63) ? (x >> y) : 0;
    if (o == "<")  return x < y;
    if (o == ">")  return x > y;
    if (o == "<=") return x <= y;
    if (o == ">=") return x >= y;
    if (o == "==") return x == y;
    return x != y;
}

} // namespace

void Semantico::executeAction(int action, const Token *token)
{
    const std::string &lexema = token->getLexeme();
    const int pos = token->getPosition();

    switch (action) {
    // ---- declarações ----
    case 1:
        if (lexema == "atomus")        tipoAtual = Tipo::Atomus;
        else if (lexema == "fractio")  tipoAtual = Tipo::Fractio;
        else if (lexema == "caracter") tipoAtual = Tipo::Caracter;
        else if (lexema == "logicum")  tipoAtual = Tipo::Logicum;
        else                           tipoAtual = Tipo::Catena;
        break;
    case 2:
        nomePendente = lexema;
        posicaoPendente = pos;
        vetorPendente = false;
        tamanhoPendente = 0;
        break;
    case 3: declararVariavel(); break;
    case 4: {
        const TipoExp t = desempilharTipo();
        const std::string vetor = entreAspas(nomePendente);
        if (t.vetor || !ehInteiro(t.base))
            erro("O tamanho do vetor " + vetor + " deve ser um inteiro", posicaoPendente);
        if (!t.constante)
            erro("O tamanho do vetor " + vetor + " deve ser uma constante (ex.: " +
                 nomePendente + "[10])", posicaoPendente);
        if (t.valor <= 0)
            erro("O tamanho do vetor " + vetor + " deve ser maior que zero", posicaoPendente);
        vetorPendente = true;
        tamanhoPendente = t.valor;
        break;
    }

    // ---- identificadores e chamadas ----
    case 10: pilhaIds.push_back({ buscar(lexema, pos), pos }); break;
    case 11: usarVariavel(token); break;
    case 12: usarElemento(); break;
    case 13: iniciarChamada(); break;
    case 14: finalizarChamada(true, token); break;
    case 15: finalizarChamada(false, token); break;
    case 16: argumento(token); break;
    case 17: verificarIndice(token); break;

    // ---- literais ----
    case 20: literalInteiro(token); break;
    case 21: pilhaTipos.push_back({ Tipo::Fractio }); break;
    case 22: {
        TipoExp t{ Tipo::Caracter };
        t.constante = true;
        t.valor = static_cast<unsigned char>(lexema.size() > 1 ? lexema[1] : 0);
        pilhaTipos.push_back(t);
        break;
    }
    case 23: pilhaTipos.push_back({ Tipo::Catena }); break;
    case 24: {
        TipoExp t{ Tipo::Logicum };
        t.constante = true;
        t.valor = (lexema == "verum") ? 1 : 0;
        pilhaTipos.push_back(t);
        break;
    }

    // ---- operadores ----
    case 29: operacaoUnaria(); break;
    case 30: pilhaOperadores.push_back({ lexema, pos }); break;
    case 31: case 32: case 33: case 34: case 35:
    case 36: case 37: case 38: case 39:
        operacaoBinaria();
        break;

    // ---- atribuição e E/S ----
    case 40: alvoVariavel(); break;
    case 41: alvoElemento(); break;
    case 42: concluirAtribuicao(); break;
    case 43: legere(false); break;
    case 44: legere(true); break;
    case 45: itemInscribe(token); break;

    // ---- controle de fluxo ----
    case 50: condicao(token); break;
    case 51: ++profundidadeLaco; break;
    case 52: --profundidadeLaco; break;
    case 53:
        if (profundidadeLaco == 0)
            erro("exi só pode ser usado dentro de um laço (dum, pro ou iterum)", pos);
        break;

    // ---- blocos ----
    case 60: abrirBloco(); break;
    case 61: fecharEscopo(); break;

    // ---- funções ----
    case 70: iniciarFuncao(token); break;
    case 71: tipoAtual = Tipo::Vazio; break;
    case 72:
        nomeParametro = lexema;
        posicaoParametro = pos;
        parametroVetor = false;
        break;
    case 73: parametroVetor = true; break;
    case 74: inserirParametro(); break;
    case 75: finalizarFuncao(); break;
    case 76: reditusComValor(token); break;
    case 77: reditusSemValor(token); break;

    // ---- programa ----
    case 80: {
        // escopo global: o programa e as funções
        abrirEscopo("global");
        Simbolo programa;
        programa.nome = lexema;
        programa.categoria = Categoria::Programa;
        programa.tipo = Tipo::Vazio;
        programa.usado = true;
        programa.posicao = pos;
        tabela.inserir(programa);
        break;
    }
    case 81:
        escopoBase = "principal";
        contadorBlocos = 0;
        abrirEscopo("principal");
        break;
    case 82:
        fecharEscopo();
        fecharEscopo();
        break;

    default:
        break;
    }
}

// ================= declarações e escopos =================

void Semantico::declararVariavel()
{
    Simbolo s;
    s.nome = nomePendente;
    s.categoria = Categoria::Variavel;
    s.tipo = tipoAtual;
    s.vetor = vetorPendente;
    s.tamanho = tamanhoPendente;
    s.posicao = posicaoPendente;

    if (!tabela.inserir(s)) {
        const Simbolo *existente = tabela.buscarNoEscopoAtual(s.nome);
        if (existente && existente->categoria == Categoria::Parametro)
            erro(entreAspas(s.nome) + " já é um parâmetro desta função", s.posicao);
        erro(entreAspas(s.nome) + " já foi declarado neste escopo", s.posicao);
    }
}

void Semantico::abrirEscopo(const std::string &rotulo)
{
    tabela.abrirEscopo(rotulo);
}

// Blocos {} / sic...cis: rótulo = função (ou principal) + número, como "principal1", "dobro2"
void Semantico::abrirBloco()
{
    abrirEscopo(escopoBase + std::to_string(++contadorBlocos));
}

void Semantico::fecharEscopo()
{
    for (const Simbolo &s : tabela.fecharEscopo()) {
        if (s.categoria == Categoria::Funcao)
            aviso(s.tipo == Tipo::Vazio
                      ? "O procedimento " + entreAspas(s.nome) + " foi declarado e nunca é chamado"
                      : "A função " + entreAspas(s.nome) + " foi declarada e nunca é chamada",
                  s.posicao);
        else if (s.inicializado)
            aviso("A variável " + entreAspas(s.nome) + " recebe valor mas nunca é lida", s.posicao);
        else
            aviso("A variável " + entreAspas(s.nome) + " foi declarada e nunca usada", s.posicao);
    }
}

// ================= identificadores =================

Simbolo *Semantico::buscar(const std::string &nome, int posicao)
{
    Simbolo *s = tabela.buscar(nome);
    if (!s)
        erro(entreAspas(nome) + " não foi declarado", posicao);
    return s;
}

Semantico::Identificador Semantico::desempilharId()
{
    if (pilhaIds.empty())
        throw std::logic_error("pilha de identificadores vazia");
    Identificador id = pilhaIds.back();
    pilhaIds.pop_back();
    return id;
}

void Semantico::usarVariavel(const Token *token)
{
    const Identificador id = desempilharId();
    Simbolo *s = id.simbolo;

    if (s->categoria == Categoria::Funcao)
        erro(entreAspas(s->nome) + " é uma função; para chamá-la use " + s->nome + "(...)",
             id.posicao);

    s->usado = true;
    TipoExp t{ s->tipo };
    if (s->vetor) {
        t.vetor = true;
        t.nome = s->nome;
    } else if (s->categoria == Categoria::Variavel && !s->inicializado && !s->avisoSemValor) {
        aviso("A variável " + entreAspas(s->nome) + " pode estar sendo usada sem valor",
              token->getPosition());
        s->avisoSemValor = true;
    }
    pilhaTipos.push_back(t);
}

void Semantico::usarElemento()
{
    const Identificador id = desempilharId();
    Simbolo *s = id.simbolo;

    if (!s->vetor || s->categoria == Categoria::Funcao)
        erro(entreAspas(s->nome) + " não é um vetor", id.posicao);

    s->usado = true;
    pilhaTipos.push_back({ s->tipo });
}

void Semantico::verificarIndice(const Token *token)
{
    const TipoExp t = desempilharTipo();
    if (t.vetor)
        erro("O índice não pode ser o vetor " + entreAspas(t.nome) + " inteiro",
             token->getPosition());
    if (!ehInteiro(t.base))
        erro("O índice de um vetor deve ser inteiro, mas é " + nomeTipo(t.base),
             token->getPosition());

    // O vetor indexado é o identificador no topo da pilha (#10 veio antes do índice)
    if (t.constante && !pilhaIds.empty()) {
        const Simbolo *s = pilhaIds.back().simbolo;
        if (s->vetor && s->tamanho > 0 && (t.valor < 0 || t.valor >= s->tamanho))
            aviso("O índice " + std::to_string(t.valor) + " está fora dos limites do vetor " +
                  entreAspas(s->nome) + " (tamanho " + std::to_string(s->tamanho) + ")",
                  token->getPosition());
    }
}

// ================= chamadas =================

void Semantico::iniciarChamada()
{
    const Identificador id = desempilharId();
    if (id.simbolo->categoria != Categoria::Funcao)
        erro(entreAspas(id.simbolo->nome) + " não é uma função", id.posicao);

    // Chamada recursiva (de dentro da própria função) não conta como uso
    if (id.simbolo != funcaoAtual)
        id.simbolo->usado = true;
    pilhaChamadas.push_back({ id.simbolo, 0, id.posicao });
}

void Semantico::argumento(const Token *token)
{
    const TipoExp t = desempilharTipo();
    Chamada &c = pilhaChamadas.back();
    const Simbolo *f = c.funcao;
    const size_t n = c.argumentos;

    if (n >= f->parametros.size())
        erro("Argumentos demais na chamada de " + entreAspas(f->nome) + ": ela espera " +
             std::to_string(f->parametros.size()), token->getPosition());

    const Parametro &p = f->parametros[n];
    const std::string rotulo = "argumento " + std::to_string(n + 1) + " de " + entreAspas(f->nome);

    if (p.vetor) {
        if (!t.vetor)
            erro("O " + rotulo + " deve ser um vetor de " + nomeTipo(p.tipo),
                 token->getPosition());
        if (t.base != p.tipo)
            erro("O " + rotulo + " deve ser um vetor de " + nomeTipo(p.tipo) +
                 ", mas " + entreAspas(t.nome) + " é um vetor de " + nomeTipo(t.base),
                 token->getPosition());
    } else {
        if (t.vetor)
            erro("O " + rotulo + " não pode ser o vetor " + entreAspas(t.nome) + " inteiro",
                 token->getPosition());
        verificarCompatibilidade(p.tipo, t, "no " + rotulo, token->getPosition());
    }
    ++c.argumentos;
}

void Semantico::finalizarChamada(bool emExpressao, const Token *token)
{
    const Chamada c = pilhaChamadas.back();
    pilhaChamadas.pop_back();
    const Simbolo *f = c.funcao;

    if (c.argumentos < f->parametros.size())
        erro("Argumentos de menos na chamada de " + entreAspas(f->nome) + ": ela espera " +
             std::to_string(f->parametros.size()) + " e recebeu " +
             std::to_string(c.argumentos), token->getPosition());

    if (emExpressao) {
        if (f->tipo == Tipo::Vazio)
            erro(entreAspas(f->nome) + " é um procedimento (munus) e não devolve valor",
                 c.posicao);
        pilhaTipos.push_back({ f->tipo });
    }
}

// ================= expressões =================

void Semantico::literalInteiro(const Token *token)
{
    const std::string &lex = token->getLexeme();
    TipoExp t{ Tipo::Atomus };
    t.constante = true;
    try {
        if (lex.size() > 2 && (lex[1] == 'x' || lex[1] == 'X'))
            t.valor = std::stoll(lex.substr(2), nullptr, 16);
        else if (lex.size() > 2 && (lex[1] == 'b' || lex[1] == 'B'))
            t.valor = std::stoll(lex.substr(2), nullptr, 2);
        else
            t.valor = std::stoll(lex, nullptr, 10);
    } catch (const std::out_of_range &) {
        erro("O literal inteiro " + lex + " é grande demais", token->getPosition());
    }
    pilhaTipos.push_back(t);
}

void Semantico::operacaoBinaria()
{
    const TipoExp b = desempilharTipo();
    const TipoExp a = desempilharTipo();
    const Operador op = desempilharOperador();
    const std::string &o = op.lexema;

    for (const TipoExp *x : { &a, &b })
        if (x->vetor)
            erro("O vetor " + entreAspas(x->nome) + " precisa de um índice para ser usado com " +
                 entreAspas(o), op.posicao);

    // Tipo do resultado vem da tabela de compatibilidade (SemanticTable::expTable)
    const int operacao = operacaoTabela(o);
    const int resultado =
        SemanticTable::resultType(tipoTabela(a.base), tipoTabela(b.base), operacao);
    if (resultado == SemanticTable::ERR)
        erro("O operador " + entreAspas(o) + " não se aplica a " + nomeTipo(a.base) + " e " +
             nomeTipo(b.base) + " (" + exigencia(operacao, false) + ")", op.posicao);

    TipoExp r(tipoLinguagem(resultado));
    const bool divisao = operacao == SemanticTable::DIV || operacao == SemanticTable::MOD;
    if (divisao && b.constante && b.valor == 0) {
        aviso(operacao == SemanticTable::DIV ? "Divisão por zero" : "Resto de divisão por zero",
              op.posicao);
    } else if (a.constante && b.constante && r.base != Tipo::Fractio) {
        r.constante = true;
        r.valor = calcular(o, a.valor, b.valor);
    }
    pilhaTipos.push_back(r);
}

void Semantico::operacaoUnaria()
{
    const TipoExp a = desempilharTipo();
    const Operador op = desempilharOperador();
    const std::string &o = op.lexema;

    if (a.vetor)
        erro("O vetor " + entreAspas(a.nome) + " precisa de um índice para ser usado com " +
             entreAspas(o), op.posicao);

    // Unários consultam a mesma tabela repetindo o operando: -x ~ SUB, ~x ~ BIT, !x / non x ~ AND
    const int operacao = (o == "-") ? SemanticTable::SUB
                       : (o == "~") ? SemanticTable::BIT
                                    : SemanticTable::AND;
    const int t = tipoTabela(a.base);
    const int resultado = SemanticTable::resultType(t, t, operacao);
    if (resultado == SemanticTable::ERR)
        erro("O operador " + entreAspas(o) + " não se aplica a " + nomeTipo(a.base) + " (" +
             exigencia(operacao, true) + ")", op.posicao);

    TipoExp r(tipoLinguagem(resultado));
    if (a.constante && r.base != Tipo::Fractio) {
        r.constante = true;
        r.valor = (o == "-") ? -a.valor : (o == "~") ? ~a.valor : !a.valor;
    }
    pilhaTipos.push_back(r);
}

TipoExp Semantico::desempilharTipo()
{
    if (pilhaTipos.empty())
        throw std::logic_error("pilha de tipos vazia");
    TipoExp t = pilhaTipos.back();
    pilhaTipos.pop_back();
    return t;
}

Semantico::Operador Semantico::desempilharOperador()
{
    if (pilhaOperadores.empty())
        throw std::logic_error("pilha de operadores vazia");
    Operador o = pilhaOperadores.back();
    pilhaOperadores.pop_back();
    return o;
}

// ================= atribuição e E/S =================

void Semantico::alvoVariavel()
{
    const Identificador id = desempilharId();
    const Simbolo *s = id.simbolo;

    if (s->categoria == Categoria::Funcao)
        erro("Não é possível atribuir valor à função " + entreAspas(s->nome), id.posicao);
    if (s->vetor)
        erro("Não é possível atribuir ao vetor " + entreAspas(s->nome) +
             " inteiro; use um índice, como " + s->nome + "[0]", id.posicao);

    pilhaAlvos.push_back({ id.simbolo, s->tipo, id.posicao });
}

void Semantico::alvoElemento()
{
    const Identificador id = desempilharId();
    const Simbolo *s = id.simbolo;

    if (!s->vetor || s->categoria == Categoria::Funcao)
        erro(entreAspas(s->nome) + " não é um vetor", id.posicao);

    pilhaAlvos.push_back({ id.simbolo, s->tipo, id.posicao });
}

void Semantico::concluirAtribuicao()
{
    const TipoExp t = desempilharTipo();
    const Alvo alvo = pilhaAlvos.back();
    pilhaAlvos.pop_back();

    if (t.vetor)
        erro("Não é possível atribuir o vetor " + entreAspas(t.nome) + " inteiro", alvo.posicao);

    verificarCompatibilidade(alvo.tipo, t, "na atribuição a " + entreAspas(alvo.simbolo->nome),
                             alvo.posicao);
    alvo.simbolo->inicializado = true;
}

void Semantico::legere(bool elemento)
{
    const Identificador id = desempilharId();
    Simbolo *s = id.simbolo;

    if (s->categoria == Categoria::Funcao)
        erro("legere não pode ler um valor para a função " + entreAspas(s->nome), id.posicao);
    if (!elemento && s->vetor)
        erro("legere precisa de um índice para o vetor " + entreAspas(s->nome), id.posicao);
    if (elemento && !s->vetor)
        erro(entreAspas(s->nome) + " não é um vetor", id.posicao);

    s->inicializado = true;
}

void Semantico::itemInscribe(const Token *token)
{
    const TipoExp t = desempilharTipo();
    if (t.vetor)
        erro("inscribe não pode escrever o vetor " + entreAspas(t.nome) +
             " inteiro; use um índice", token->getPosition());
}

// ================= controle de fluxo =================

void Semantico::condicao(const Token *token)
{
    const TipoExp t = desempilharTipo();
    if (t.vetor)
        erro("A condição não pode ser o vetor " + entreAspas(t.nome) + " inteiro",
             token->getPosition());
    if (!ehNumerico(t.base))
        erro("A condição deve ser um valor escalar (atomus, fractio, caracter ou logicum), não " +
             nomeTipo(t.base), token->getPosition());
}

// ================= funções =================

void Semantico::iniciarFuncao(const Token *token)
{
    Simbolo f;
    f.nome = token->getLexeme();
    f.categoria = Categoria::Funcao;
    f.tipo = tipoAtual;   // definido por #1 (opus) ou #71 (munus)
    f.posicao = token->getPosition();

    funcaoAtual = tabela.inserir(f);
    if (!funcaoAtual)
        erro(entreAspas(f.nome) + " já foi declarado", f.posicao);

    funcaoTeveRetorno = false;
    escopoBase = f.nome;
    contadorBlocos = 0;
    abrirEscopo(f.nome);   // parâmetros e corpo ficam no mesmo escopo, como em C
}

void Semantico::inserirParametro()
{
    Simbolo p;
    p.nome = nomeParametro;
    p.categoria = Categoria::Parametro;
    p.tipo = tipoAtual;
    p.vetor = parametroVetor;
    p.inicializado = true;
    p.posicao = posicaoParametro;
    p.posicaoParametro = static_cast<int>(funcaoAtual->parametros.size()) + 1;

    if (!tabela.inserir(p))
        erro("O parâmetro " + entreAspas(p.nome) + " aparece mais de uma vez em " +
             entreAspas(funcaoAtual->nome), p.posicao);

    funcaoAtual->parametros.push_back({ p.tipo, p.vetor });
}

void Semantico::finalizarFuncao()
{
    if (funcaoAtual->tipo != Tipo::Vazio && !funcaoTeveRetorno)
        aviso("A função " + entreAspas(funcaoAtual->nome) + " pode terminar sem reditus",
              funcaoAtual->posicao);

    fecharEscopo();
    funcaoAtual = nullptr;
}

void Semantico::reditusComValor(const Token *token)
{
    const TipoExp t = desempilharTipo();
    const int pos = token->getPosition();

    if (!funcaoAtual)
        erro("reditus só pode ser usado dentro de uma função (opus ou munus)", pos);
    if (funcaoAtual->tipo == Tipo::Vazio)
        erro("O procedimento " + entreAspas(funcaoAtual->nome) +
             " (munus) não devolve valor; use apenas reditus;", pos);
    if (t.vetor)
        erro("reditus não pode devolver o vetor " + entreAspas(t.nome) + " inteiro", pos);

    verificarCompatibilidade(funcaoAtual->tipo, t, "no reditus de " + entreAspas(funcaoAtual->nome),
                             pos);
    funcaoTeveRetorno = true;
}

void Semantico::reditusSemValor(const Token *token)
{
    const int pos = token->getPosition();
    if (!funcaoAtual)
        erro("reditus só pode ser usado dentro de uma função (opus ou munus)", pos);
    if (funcaoAtual->tipo != Tipo::Vazio)
        erro("A função " + entreAspas(funcaoAtual->nome) + " deve devolver um valor do tipo " +
             nomeTipo(funcaoAtual->tipo), pos);
}

// ================= utilitários =================

void Semantico::verificarCompatibilidade(Tipo destino, const TipoExp &origem,
                                         const std::string &contexto, int posicao)
{
    // Consulta a tabela de atribuições (SemanticTable::atribTable): OK_, WAR ou ERR
    const int status = SemanticTable::atribType(tipoTabela(destino), tipoTabela(origem.base));

    if (status == SemanticTable::WAR)
        aviso("Conversão de " + nomeTipo(origem.base) + " para " + nomeTipo(destino) + " " +
              contexto + " descarta a parte decimal", posicao);
    else if (status == SemanticTable::ERR)
        erro("Tipos incompatíveis " + contexto + ": esperado " + nomeTipo(destino) +
             ", encontrado " + nomeTipo(origem.base), posicao);
}

void Semantico::erro(const std::string &mensagem, int posicao)
{
    throw SemanticError(mensagem, posicao);
}

void Semantico::aviso(const std::string &mensagem, int posicao)
{
    avisos.push_back({ mensagem, posicao });
}
