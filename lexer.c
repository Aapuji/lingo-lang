#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "token.h"
#include "tt.h"

struct lexer_error init_error(enum lexer_errno lerrno, char ch, int line, int col) {
    return (struct lexer_error) { lerrno, ch, line, col };
}

struct lexer_result {
    bool result;
    union {
        struct token success;
        struct lexer_error error;
    };
};

struct lexer_result ok(struct token result) {
    return (struct lexer_result) {
        .result = true,
        .success = result
    };
}

struct lexer_result err(struct lexer_error error) {
    return (struct lexer_result) {
        .result = false,
        .error = error
    };
}

struct lexer init_lexer(char *src) {
    return (struct lexer) {
        .ch = src,
        .line = 1,
        .col = 1,
        .comment_depth = 0,
        .in_string = 0
    };
}

int at_end(struct lexer *lexer) {
    return *lexer->ch == '\0';
}

void next(struct lexer *lexer) {
    lexer->ch++;
    lexer->col++;
}

char peek(struct lexer *lexer) {
    if (at_end(lexer)) return '\0';
    else return lexer->ch[1];
}

char prev(struct lexer *lexer) {
    return lexer->ch[-1];
}

int num_nested_comments(struct lexer *lexer) {
    return lexer->comment_depth - (lexer->comment_depth & 0x1);
}

// Should not ever be at a comment.
struct lexer_result lex_token(struct lexer *lexer) {
    switch (*lexer->ch) {
        // Operators
        case '/':
            next(lexer);
            return ok(init_token(TT_SLASH, lexer->line, lexer->col, "/", 1));
        case '.':
           next(lexer);
           return ok(init_token(TT_DOT, lexer->line, lexer->col, ".", 1));
        case '-':
            char c = peek(lexer);

            next(lexer);
            if (c == '>') {
                next(lexer);
                return ok(init_token(TT_RARROW, lexer->line, lexer->col, "->", 2));
            } else {
                return ok(init_token(TT_DASH, lexer->line, lexer->col, "-", 1));
            }
        case '>':
            next(lexer);
            if (peek(lexer) == '>') {
                next(lexer);
                init_token(TT_RSHIFT, lexer->line, lexer->col, ">>", 2);
            } else {
                err(init_error(LEXER_UNKNOWN, *lexer->ch, lexer->line, lexer->col));
            }
        case ':':
            next(lexer);
            return ok(init_token(TT_COLON, lexer->line, lexer->col, ":", 1));
        case '(': break;
        default:
            return err(init_error(LEXER_UNKNOWN, *lexer->ch, lexer->line, lexer->col));
    }

    // Make sure in `lex` to check if src is 0 length, because this will crash if that happens
    if (num_nested_comments(lexer) > 0) return err(init_error(LEXER_UNTERMINATED_COMMENT, prev(lexer), lexer->line, lexer->col));

    else return err(init_error(LEXER_INCOMPLETE, *lexer->ch, lexer->line, lexer->col));
}

struct token *lex(struct lexer *lexer, struct lexer_error *errors, size_t *num_errors) {
    if (peek(lexer) == '\0') {
        num_errors = 0;
        struct token *eof = (struct token *) malloc(sizeof(struct token));
        *eof = init_token(TT_EOF, lexer->line, lexer->col, "", 0);

        return eof;
    }

    size_t err_cap = *num_errors;
    num_errors = 0;
    size_t *err_len = num_errors;


    struct token *tokens = malloc(sizeof(struct token) * 16);
    size_t tok_len = 0;
    size_t tok_cap = 16;

    while (!at_end(lexer)){
        // Comment
        if (*lexer->ch == '/') {
            char c = peek(lexer);

            // comment_depth is odd means comments started with //
            // all others is the number of /* */
            if (c == '/' && lexer->comment_depth == 0) {
                lexer->comment_depth++; // rightmost bit signifies comment chain started with //
            } else if (c == '*') {
                lexer->comment_depth += 2; // other bits tell number of nested comments
            }
        } else if (lexer->comment_depth > 1 && *lexer->ch == '*') {
            if (peek(lexer) == '/') {
                lexer->comment_depth -= 2;
            }
        }

        // Skip Comments
        if (lexer->comment_depth > 0) {
            if (*lexer->ch == '\n') {
                lexer->line++;
                lexer->col = 1;
            };
            next(lexer);
            continue;
        }

        // Skip Whitespace
        // Skip Newline
        if (*lexer->ch == '\n') {
            lexer->line++;
            lexer->col = 1;
            next(lexer);
            continue;
        }
        // Skip Other Whitespace
        if (isspace(*lexer->ch)) {
            next(lexer);
            continue;
        }


        // Append Token
        if (tok_len == tok_cap) {
            tok_cap *= 2;
            tokens = (struct token *) realloc(tokens, sizeof(struct token) * tok_cap);

            if (tokens == NULL) {
                perror("Realloc failed");
                exit(1);
            }
        }

        struct lexer_result result = lex_token(lexer);
        if (result.result) {
            tokens[tok_len++] = result.success;
        } else {
            if (*err_len == err_cap) {
                err_cap *= 2;
                errors = (struct lexer_error *) realloc(errors, sizeof(struct lexer_error) * err_cap);

                if (errors == NULL) {
                    perror("Realloc failed");
                    exit(1);
                }
            }

            errors[(*err_len)++] = result.error;
        }
    }

    return tokens;
}
