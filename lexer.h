#ifndef LEXER_H
#define LEXER_H

#include "token.h"

struct lexer {
    char *ch;
    int line;
    int comment_depth;
    int in_string;
    struct token current_token;
};

enum lexer_errno {
    LEXER_INCOMPLETE = 0,
    LEXER_UNKNOWN,
    LEXER_UNTERMINATED_COMMENT,
    LEXER_UNTERMINATED_STRING,
    LEXER_MULTILINE_STRING
};

struct lexer_error {
    enum lexer_errno lerrno;
    char ch;
    int line;
};

struct lexer init_lexer(char *src);
// Lexes the source given the lexer, returning a TT_EOF-terminated vector of tokens and updates an array of errors produced and number of error generated.
struct token *lex(struct lexer *lexer, struct lexer_error *errors, int *num_errors);

#endif
