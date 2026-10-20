/*
 * Template to help verify type compatibility in a Semantic Analyzer.
 * Available to Computer Science course at UNIVALI.
 * Professor Eduardo Alves da Silva.
 *
 * Preenchido para a Roman++ (regras inspiradas em C):
 *  - INT = atomus, FLO = fractio, CHA = caracter, STR = catena, BOO = logicum.
 *  - caracter e logicum são promovidos a inteiro nas contas (como char e bool em C).
 *  - SUM/SUB/MUL/DIV: FLO se algum operando for FLO; senão INT (INT / INT = INT, divisão inteira).
 *  - REL (< > <= >= == !=), AND/OR_ (et && vel ||): resultado BOO; aceitam qualquer numérico.
 *  - MOD (%) e BIT (& | ^ << >> ~): só operandos inteiros (INT, CHA, BOO).
 *  - STR (catena) não tem operadores.
 *  - POT e ROO (potência e raiz) não existem na Roman++: sempre ERR.
 *  - BIT foi acrescentada ao modelo original (a linguagem tem operadores bit a bit).
 *
 * Unários usam a mesma tabela repetindo o operando:
 *   -x -> resultType(x, x, SUB)    ~x -> resultType(x, x, BIT)    !x / non x -> resultType(x, x, AND)
 */

#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H

class SemanticTable {
public:
    enum Types {INT = 0,FLO, CHA, STR,BOO};
    enum Operations {SUM = 0,SUB,MUL,DIV,REL,MOD,POT,ROO,AND,OR_,BIT};
    enum Status {ERR = -1,WAR,OK_};

    // TIPO DE RETORNO DAS EXPRESSOES ENTRE TIPOS
    // 5 x 5 X 11  = TIPO X TIPO X OPER
    static int const expTable [5][5][11];

    // atribuicoes compativeis
    // 5 x 5 = TIPO X TIPO   (linha: destino, coluna: valor atribuído)
    static int const atribTable [5][5];

    static int resultType (int TP1, int TP2, int OP) {
        if(TP1 < 0 || TP2 < 0 || OP < 0) {
            return ERR;
        }
        return (expTable[TP1][TP2][OP]);
    }

    static int atribType (int TP1, int TP2){
        if(TP1 < 0 || TP2 < 0) {
            return ERR;
        }
        return (atribTable[TP1][TP2]);
    }

//    SemanticTable();
};

// Definições dos membros estáticos: incluir este arquivo em um único .cpp (Semantico.cpp)
int const SemanticTable::expTable[5][5][11] =
            {/*                    INT                      */ /*                    FLOAT                    */ /*                    CHAR                     */ /*                   STRING                    */ /*                    BOOL                     */
       /*     SUM,SUB,MUL,DIV,REL,MOD,POT,ROO,AND,OR_,BIT , SUM,SUB,MUL,DIV,REL,MOD,POT,ROO,AND,OR_,BIT , SUM,SUB,MUL,DIV,REL,MOD,POT,ROO,AND,OR_,BIT , SUM,SUB,MUL,DIV,REL,MOD,POT,ROO,AND,OR_,BIT , SUM,SUB,MUL,DIV,REL,MOD,POT,ROO,AND,OR_,BIT */
/*INT   */  {{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT},{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR},{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT}},
/*FLOAT */  {{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR},{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR},{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR}},
/*CHAR  */  {{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT},{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR},{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT}},
/*STRING*/  {{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR}},
/*BOOL  */  {{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT},{FLO,FLO,FLO,FLO,BOO,ERR,ERR,ERR,BOO,BOO,ERR},{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT},{ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR,ERR},{INT,INT,INT,INT,BOO,INT,ERR,ERR,BOO,BOO,INT}}
            };

// WAR: fractio -> atomus/caracter descarta a parte decimal.
// logicum aceita qualquer numérico (vira 0 ou 1, como bool em C). catena só recebe catena.
int const SemanticTable::atribTable[5][5]=
    {     /* INT FLO CHA STR BOO  */
    /*INT*/ {OK_,WAR,OK_,ERR,OK_},
    /*FLO*/ {OK_,OK_,OK_,ERR,OK_},
    /*CHA*/ {OK_,WAR,OK_,ERR,OK_},
    /*STR*/ {ERR,ERR,ERR,OK_,ERR},
    /*BOO*/ {OK_,OK_,OK_,ERR,OK_}
    };

#endif // SEMANTIC_TABLE_H
