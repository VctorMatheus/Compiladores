#ifndef CONSTANTS_H
#define CONSTANTS_H

enum TokenId 
{
    EPSILON  = 0,
    DOLLAR   = 1,
    t_ID = 2,
    t_SL_COMM = 3,
    t_ML_COMM = 4,
    t_IF = 5,
    t_THEN = 6,
    t_ELSE = 7,
    t_WHILE = 8,
    t_DO = 9,
    t_FOR = 10,
    t_REPEAT = 11,
    t_UNTIL = 12,
    t_SWITCH = 13,
    t_CASE = 14,
    t_BEGIN = 15,
    t_END = 16,
    t_BREAK = 17,
    t_O_CUR_BRAC = 18,
    t_C_CUR_BRAC = 19,
    t_O_CB_EXT = 20,
    t_C_CB_EXT = 21,
    t_INT = 22,
    t_FLOAT = 23,
    t_CHAR = 24,
    t_BOOL = 25,
    t_STRING = 26,
    t_TRUE = 27,
    t_FALSE = 28,
    t_READ = 29,
    t_WRITE = 30,
    t_FUNCTION = 31,
    t_PROCEDURE = 32,
    t_RETURN = 33,
    t_ASSIGN = 34,
    t_EQUAL = 35,
    t_DIFF = 36,
    t_LESS = 37,
    t_GREATER = 38,
    t_LESS_EQ = 39,
    t_GREAT_EQ = 40,
    t_AND = 41,
    t_OR = 42,
    t_NOT = 43,
    t_AND_EXT = 44,
    t_OR_EXT = 45,
    t_NOT_EXT = 46,
    t_SUM = 47,
    t_SUB = 48,
    t_MUL = 49,
    t_DIV = 50,
    t_MOD = 51,
    t_SUM_EXT = 52,
    t_SUB_EXT = 53,
    t_MUL_EXT = 54,
    t_DIV_EXT = 55,
    t_MOD_EXT = 56,
    t_SHL = 57,
    t_SHR = 58,
    t_BIT_AND = 59,
    t_BIT_OR = 60,
    t_BIT_NOT = 61,
    t_BIT_XOR = 62,
    t_O_PARENT = 63,
    t_C_PARENT = 64,
    t_O_SQR_BRAC = 65,
    t_C_SQR_BRAC = 66,
    t_COMMA = 67,
    t_COLON = 68,
    t_SEMICOLON = 69,
    t_LIT_INT = 70,
    t_LIT_BIN = 71,
    t_LIT_HEX = 72,
    t_LIT_FLOAT = 73,
    t_LIT_CHAR = 74,
    t_LIT_STR = 75
};

const int STATES_COUNT = 52;

extern int SCANNER_TABLE[STATES_COUNT][256];

extern int TOKEN_STATE[STATES_COUNT];

extern int SPECIAL_CASES_INDEXES[77];

extern const char *SPECIAL_CASES_KEYS[35];

extern int SPECIAL_CASES_VALUES[35];

extern const char *SCANNER_ERROR[STATES_COUNT];

const int FIRST_SEMANTIC_ACTION = 130;

const int SHIFT  = 0;
const int REDUCE = 1;
const int ACTION = 2;
const int ACCEPT = 3;
const int GO_TO  = 4;
const int ERROR  = 5;

extern const int PARSER_TABLE[217][130][2];

extern const int PRODUCTIONS[120][2];

extern const char *PARSER_ERROR[217];

#endif
