#ifndef TOKEN_H
#define TOKEN_H

#include "tt.h"

struct token {
    enum tt tt;
    int line;
    int col;
    union {
        int num;
        char *str;
    } data;
    char *lexeme;
};

struct token init_token(enum tt tt, int line, int col, char *lexeme, int len);

#endif
