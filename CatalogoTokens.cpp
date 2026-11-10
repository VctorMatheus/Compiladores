#include "CatalogoTokens.h"

namespace {

const char *const RESERVADA = "Palavra reservada";
const char *const OPERADOR = "Operador";
const char *const DELIMITADOR = "Delimitador";
const char *const LITERAL = "Literal";
const char *const IGNORADO = "Ignorado";

} // namespace

const std::vector<InfoToken> &catalogoTokens()
{
    static const std::vector<InfoToken> catalogo = {
        { t_ID, "ID", "(letra | _) (letra | dígito | _)*", "contador_1", "Identificador",
          "nome de variável, vetor ou função" },

        // controle de fluxo
        { t_IF,     "IF",     "si",     nullptr, RESERVADA, "se: início do condicional" },
        { t_THEN,   "THEN",   "ergo",   nullptr, RESERVADA, "então", true },
        { t_ELSE,   "ELSE",   "aliter", nullptr, RESERVADA, "senão (aliter si encadeia condições)" },
        { t_WHILE,  "WHILE",  "dum",    nullptr, RESERVADA, "enquanto: laço com teste no início" },
        { t_DO,     "DO",     "fac",    nullptr, RESERVADA, "faça: antecede o bloco do si, do dum e do pro" },
        { t_FOR,    "FOR",    "pro",    nullptr, RESERVADA, "para: laço com inicialização, condição e passo" },
        { t_REPEAT, "REPEAT", "iterum", nullptr, RESERVADA, "repita: laço com teste no fim" },
        { t_UNTIL,  "UNTIL",  "usque",  nullptr, RESERVADA, "até: condição de parada do iterum" },
        { t_SWITCH, "SWITCH", "opta",   nullptr, RESERVADA, "escolha", true },
        { t_CASE,   "CASE",   "casus",  nullptr, RESERVADA, "caso", true },

        // blocos
        { t_BEGIN,      "BEGIN",      "principium", nullptr, RESERVADA, "início do programa" },
        { t_END,        "END",        "terminus",   nullptr, RESERVADA, "fim do programa" },
        { t_BREAK,      "BREAK",      "exi",        nullptr, RESERVADA, "sai do laço mais interno" },
        { t_O_CUR_BRAC, "O_CUR_BRAC", "{",          nullptr, DELIMITADOR, "abre bloco de comandos" },
        { t_C_CUR_BRAC, "C_CUR_BRAC", "}",          nullptr, DELIMITADOR, "fecha bloco de comandos" },
        { t_O_CB_EXT,   "O_CB_EXT",   "sic",        nullptr, RESERVADA,
          "abre bloco (forma em extenso de {; obrigatória no corpo de funções)" },
        { t_C_CB_EXT,   "C_CB_EXT",   "cis",        nullptr, RESERVADA,
          "fecha bloco (forma em extenso de })" },

        // tipos e literais lógicos
        { t_INT,    "INT",    "atomus",   nullptr, RESERVADA, "tipo inteiro" },
        { t_FLOAT,  "FLOAT",  "fractio",  nullptr, RESERVADA, "tipo real" },
        { t_CHAR,   "CHAR",   "caracter", nullptr, RESERVADA, "tipo caractere" },
        { t_BOOL,   "BOOL",   "logicum",  nullptr, RESERVADA, "tipo lógico" },
        { t_STRING, "STRING", "catena",   nullptr, RESERVADA, "tipo cadeia de caracteres" },
        { t_TRUE,   "TRUE",   "verum",    nullptr, LITERAL,   "verdadeiro (logicum)" },
        { t_FALSE,  "FALSE",  "falsus",   nullptr, LITERAL,   "falso (logicum)" },

        // entrada, saída e subprogramas
        { t_READ,      "READ",      "legere",   nullptr, RESERVADA, "lê valores para variáveis" },
        { t_WRITE,     "WRITE",     "inscribe", nullptr, RESERVADA, "escreve valores na saída" },
        { t_FUNCTION,  "FUNCTION",  "opus",     nullptr, RESERVADA, "declara função que devolve valor" },
        { t_PROCEDURE, "PROCEDURE", "munus",    nullptr, RESERVADA, "declara procedimento (sem retorno)" },
        { t_RETURN,    "RETURN",    "reditus",  nullptr, RESERVADA, "retorna da função" },

        // relacionais e atribuição
        { t_ASSIGN,   "ASSIGN",   "=",  nullptr, OPERADOR, "atribuição" },
        { t_EQUAL,    "EQUAL",    "==", nullptr, OPERADOR, "igual a" },
        { t_DIFF,     "DIFF",     "!=", nullptr, OPERADOR, "diferente de" },
        { t_LESS,     "LESS",     "<",  nullptr, OPERADOR, "menor que" },
        { t_GREATER,  "GREATER",  ">",  nullptr, OPERADOR, "maior que" },
        { t_LESS_EQ,  "LESS_EQ",  "<=", nullptr, OPERADOR, "menor ou igual a" },
        { t_GREAT_EQ, "GREAT_EQ", ">=", nullptr, OPERADOR, "maior ou igual a" },

        // lógicos
        { t_AND,     "AND",     "&&",  nullptr, OPERADOR, "e lógico" },
        { t_OR,      "OR",      "||",  nullptr, OPERADOR, "ou lógico" },
        { t_NOT,     "NOT",     "!",   nullptr, OPERADOR, "negação lógica" },
        { t_AND_EXT, "AND_EXT", "et",  nullptr, OPERADOR, "e lógico (forma em extenso de &&)" },
        { t_OR_EXT,  "OR_EXT",  "vel", nullptr, OPERADOR, "ou lógico (forma em extenso de ||)" },
        { t_NOT_EXT, "NOT_EXT", "non", nullptr, OPERADOR, "negação lógica (forma em extenso de !)" },

        // aritméticos
        { t_SUM,     "SUM",     "+",    nullptr, OPERADOR, "soma" },
        { t_SUB,     "SUB",     "-",    nullptr, OPERADOR, "subtração ou menos unário" },
        { t_MUL,     "MUL",     "*",    nullptr, OPERADOR, "multiplicação" },
        { t_DIV,     "DIV",     "/",    nullptr, OPERADOR, "divisão" },
        { t_MOD,     "MOD",     "%",    nullptr, OPERADOR, "resto da divisão" },
        { t_SUM_EXT, "SUM_EXT", "adde", nullptr, OPERADOR, "soma (forma em extenso de +)" },
        { t_SUB_EXT, "SUB_EXT", "minu", nullptr, OPERADOR, "subtração (forma em extenso de -)" },
        { t_MUL_EXT, "MUL_EXT", "mult", nullptr, OPERADOR, "multiplicação (forma em extenso de *)" },
        { t_DIV_EXT, "DIV_EXT", "divi", nullptr, OPERADOR, "divisão (forma em extenso de /)" },
        { t_MOD_EXT, "MOD_EXT", "modu", nullptr, OPERADOR, "resto da divisão (forma em extenso de %)" },

        // bit a bit
        { t_SHL,     "SHL",     "<<", nullptr, OPERADOR, "deslocamento de bits à esquerda" },
        { t_SHR,     "SHR",     ">>", nullptr, OPERADOR, "deslocamento de bits à direita" },
        { t_BIT_AND, "BIT_AND", "&",  nullptr, OPERADOR, "e bit a bit" },
        { t_BIT_OR,  "BIT_OR",  "|",  nullptr, OPERADOR, "ou bit a bit" },
        { t_BIT_NOT, "BIT_NOT", "~",  nullptr, OPERADOR, "negação bit a bit" },
        { t_BIT_XOR, "BIT_XOR", "^",  nullptr, OPERADOR, "ou exclusivo bit a bit" },

        // pontuação
        { t_O_PARENT,   "O_PARENT",   "(", nullptr, DELIMITADOR, "abre parênteses (expressões, condições e chamadas)" },
        { t_C_PARENT,   "C_PARENT",   ")", nullptr, DELIMITADOR, "fecha parênteses" },
        { t_O_SQR_BRAC, "O_SQR_BRAC", "[", nullptr, DELIMITADOR, "abre colchetes (tamanho ou índice de vetor)" },
        { t_C_SQR_BRAC, "C_SQR_BRAC", "]", nullptr, DELIMITADOR, "fecha colchetes" },
        { t_COMMA,      "COMMA",      ",", nullptr, DELIMITADOR, "separa itens de listas (nomes, parâmetros, argumentos)" },
        { t_COLON,      "COLON",      ":", nullptr, DELIMITADOR, "dois-pontos", true },
        { t_SEMICOLON,  "SEMICOLON",  ";", nullptr, DELIMITADOR, "fim de comando ou de declaração" },

        // literais
        { t_LIT_INT,   "LIT_INT",   "dígito+",           "42",           LITERAL, "inteiro decimal (atomus)" },
        { t_LIT_BIN,   "LIT_BIN",   "0b [01]+",          "0b1010",       LITERAL, "inteiro binário (atomus)" },
        { t_LIT_HEX,   "LIT_HEX",   "0x [0-9a-fA-F]+",   "0x1F",         LITERAL, "inteiro hexadecimal (atomus)" },
        { t_LIT_FLOAT, "LIT_FLOAT", "dígito+ . dígito+", "3.14",         LITERAL, "real (fractio)" },
        { t_LIT_CHAR,  "LIT_CHAR",  "' caractere '",     "'a'",          LITERAL, "um caractere (caracter)" },
        { t_LIT_STR,   "LIT_STR",   "\" texto \"",       "\"Ave, Roma!\"", LITERAL, "cadeia de caracteres (catena)" },

        // descartados pelo léxico (declarados sem nome no .vgls)
        { EPSILON, nullptr, "// texto até o fim da linha", "// nota", IGNORADO, "comentário de linha" },
        { EPSILON, nullptr, "/* texto */", "/* nota\n   longa */", IGNORADO,
          "comentário de bloco, de uma ou mais linhas (sem */ é erro léxico)" },
        { EPSILON, nullptr, "espaço, tab e quebra de linha", " \t\r\n", IGNORADO,
          "separam os tokens" },
    };
    return catalogo;
}
