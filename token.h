#ifndef TOKEN_H
#define TOKEN_H

#include "tt.h"

struct token {
    enum tt tt;
    int line;
    union {
        int num;
        char *str;
    } data;
};

struct token init_token(enum tt tt, int line, char *lexeme, int len);

#endif
