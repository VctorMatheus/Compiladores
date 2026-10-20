#ifndef CONSTANTS_H
#define CONSTANTS_H

enum TokenId 
{
    EPSILON  = 0,
    DOLLAR   = 1,
    t_ID = 2,
    t_IF = 3,
    t_THEN = 4,
    t_ELSE = 5,
    t_WHILE = 6,
    t_DO = 7,
    t_FOR = 8,
    t_REPEAT = 9,
    t_UNTIL = 10,
    t_SWITCH = 11,
    t_CASE = 12,
    t_BEGIN = 13,
    t_END = 14,
    t_BREAK = 15,
    t_O_CUR_BRAC = 16,
    t_C_CUR_BRAC = 17,
    t_O_CB_EXT = 18,
    t_C_CB_EXT = 19,
    t_INT = 20,
    t_FLOAT = 21,
    t_CHAR = 22,
    t_BOOL = 23,
    t_STRING = 24,
    t_TRUE = 25,
    t_FALSE = 26,
    t_READ = 27,
    t_WRITE = 28,
    t_FUNCTION = 29,
    t_PROCEDURE = 30,
    t_RETURN = 31,
    t_ASSIGN = 32,
    t_EQUAL = 33,
    t_DIFF = 34,
    t_LESS = 35,
    t_GREATER = 36,
    t_LESS_EQ = 37,
    t_GREAT_EQ = 38,
    t_AND = 39,
    t_OR = 40,
    t_NOT = 41,
    t_AND_EXT = 42,
    t_OR_EXT = 43,
    t_NOT_EXT = 44,
    t_SUM = 45,
    t_SUB = 46,
    t_MUL = 47,
    t_DIV = 48,
    t_MOD = 49,
    t_SUM_EXT = 50,
    t_SUB_EXT = 51,
    t_MUL_EXT = 52,
    t_DIV_EXT = 53,
    t_MOD_EXT = 54,
    t_SHL = 55,
    t_SHR = 56,
    t_BIT_AND = 57,
    t_BIT_OR = 58,
    t_BIT_NOT = 59,
    t_BIT_XOR = 60,
    t_O_PARENT = 61,
    t_C_PARENT = 62,
    t_O_SQR_BRAC = 63,
    t_C_SQR_BRAC = 64,
    t_COMMA = 65,
    t_COLON = 66,
    t_SEMICOLON = 67,
    t_LIT_INT = 68,
    t_LIT_BIN = 69,
    t_LIT_HEX = 70,
    t_LIT_FLOAT = 71,
    t_LIT_CHAR = 72,
    t_LIT_STR = 73
};

const int STATES_COUNT = 50;

extern int SCANNER_TABLE[STATES_COUNT][256];

extern int TOKEN_STATE[STATES_COUNT];

extern int SPECIAL_CASES_INDEXES[75];

extern const char *SPECIAL_CASES_KEYS[35];

extern int SPECIAL_CASES_VALUES[35];

extern const char *SCANNER_ERROR[STATES_COUNT];

const int FIRST_SEMANTIC_ACTION = 128;

const int SHIFT  = 0;
const int REDUCE = 1;
const int ACTION = 2;
const int ACCEPT = 3;
const int GO_TO  = 4;
const int ERROR  = 5;

extern const int PARSER_TABLE[344][211][2];

extern const int PRODUCTIONS[126][2];

extern const char *PARSER_ERROR[344];

#endif
