#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>

#include "token.h"

struct lexer {
    FILE *src;
    char ch;
    char prev;
    int line;
    int col;
    int comment_depth;
    int in_string;
    struct token current_token;
};

enum lexer_errno {
    LEXER_INCOMPLETE = 0,           // Incomplete token
    LEXER_UNKNOWN, 
    LEXER_UNTERMINATED_COMMENT,
    LEXER_UNTERMINATED_STRING,
    LEXER_MULTILINE_STRING,
    LEXER_INVALID_DIGIT
};

struct lexer_error {
    enum lexer_errno lerrno;
    char ch;
    int line;
    int col;
};

struct lexer init_lexer(FILE *src);
/// Lexes the source given the lexer, returning a `TT_EOF`-terminated vector of tokens and updates an array of errors produced and number of errors generated.
///
/// @param lexer Pointer to the lexer
/// @param num_errors Contains number of `lexer_error`s `errors` can contain initially. If it is 0 and if `errors` is `NULL`, then it will be allocated. Should not be `NULL`.
/// @param errors Contains pointer to `lexer_error` array, may be reallocated as many times as needed.
///
/// @returns An `TT-EOF`-terminated array of `token`s.
struct token *lex(struct lexer *lexer, struct lexer_error *errors, size_t *num_errors);

#endif
