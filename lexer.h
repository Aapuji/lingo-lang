#ifndef LEXER_H
#define LEXER_H

#include <stdlib.h>

#include "token.h"

struct lexer {
    char *ch;
    int line;
    int col;
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
    int col;
};

struct lexer init_lexer(char *src);
/// Lexes the source given the lexer, returning a `TT_EOF`-terminated vector of tokens and updates an array of errors produced and number of error generated.
///
/// Notes;
/// - `num_errors` should contain number of `lexer_error`s `errors` can contain initially.
/// - `errors` may be reallocated.
/// - They should not be `NULL`.
struct token *lex(struct lexer *lexer, struct lexer_error *errors, size_t *num_errors);

#endif
