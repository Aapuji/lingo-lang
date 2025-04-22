#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lexer.h"
#include "util.h"
#include "token.h"
#include "tt.h"
#include "util.h"

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

struct lexer init_lexer(FILE *src) {
    struct lexer lexer = (struct lexer) {
        .src = src,
        .prev = '\0',
        .ch = '\0',
        .line = 1,
        .col = 1,
        .comment_depth = 0,
        .in_string = 0
    };

    int ch = fgetc(src);
    if (ch != EOF) // If EOF, then ch should stay at \0
        lexer.ch = ch;

    return lexer;
}

int at_end(struct lexer *lexer) {
    return lexer->ch == '\0';
}

void next(struct lexer *lexer) {
    if (lexer->ch == '\n') {
        lexer->line++;
        lexer->col = 1;
    } else {
        lexer->col++;
    }

    int i = fgetc(lexer->src);
    lexer->prev = lexer->ch;
    if (i == EOF)
        lexer->ch = '\0';
    else
        lexer->ch = (char) i;
}

char peek(struct lexer *lexer) {
    if (at_end(lexer)) return '\0';
    else {
        int ch = fgetc(lexer->src);
        if (ch == EOF)
            return '\0';
        else {
            ungetc(ch, lexer->src);
            return ch;
        }
    }
}

char prev(struct lexer *lexer) {
    return lexer->prev;
}

int num_nested_comments(struct lexer *lexer) {
    return lexer->comment_depth - (lexer->comment_depth & 0x1);
}

// Should not ever be at a comment.
struct lexer_result lex_token(struct lexer *lexer) {
    switch (lexer->ch) {
        // Operators
        case '/':
            next(lexer);
            return ok(init_token(TT_SLASH, lexer->line, lexer->col, "/", 1));
        case '.':
           next(lexer);
           return ok(init_token(TT_DOT, lexer->line, lexer->col, ".", 1));
        case '-': {
            char c = peek(lexer);

            next(lexer);
            if (c == '>') {
                next(lexer);
                return ok(init_token(TT_RARROW, lexer->line, lexer->col, "->", 2));
            } else {
                return ok(init_token(TT_DASH, lexer->line, lexer->col, "-", 1));
            }
        }
        case '>': {
            char c = peek(lexer);

            next(lexer);
            if (c == '>') {
                next(lexer);
                return ok(init_token(TT_RSHIFT, lexer->line, lexer->col, ">>", 2));
            } else {
                return err(init_error(LEXER_UNKNOWN, prev(lexer), lexer->line, lexer->col - 1));
            }
        }
        case ':':
            next(lexer);
            return ok(init_token(TT_COLON, lexer->line, lexer->col, ":", 1));
        case '(': break;
        default:
            next(lexer);
            return err(init_error(LEXER_UNKNOWN, prev(lexer), lexer->line, lexer->col - 1));
    }

    // Make sure in `lex` to check if src is 0 length, because this will crash if that happens
    if (num_nested_comments(lexer) > 0) return err(init_error(LEXER_UNTERMINATED_COMMENT, prev(lexer), lexer->line, lexer->col - 1));

    else return err(init_error(LEXER_INCOMPLETE, prev(lexer), lexer->line, lexer->col - 1));
}

struct token *lex(struct lexer *lexer, struct lexer_error *errors, size_t *num_errors) {
    if (peek(lexer) == '\0') {
        num_errors = 0;
        struct token *eof = (struct token *) malloc(sizeof(struct token));
        *eof = init_token(TT_EOF, lexer->line, lexer->col, "", 0);

        return eof;
    }

    if (*num_errors == 0 && errors == NULL) {
        *num_errors = 8;
        errors = (struct lexer_error *) malloc(sizeof(struct lexer_error) * *num_errors);
        if (errors == NULL) {
            perror("Malloc failed");
            exit(1);
        } 
    }
    size_t err_cap = *num_errors;
    *num_errors = 0;
    size_t *err_len = num_errors;

    struct token *tokens = malloc(sizeof(struct token) * 16);
    size_t tok_len = 0;
    size_t tok_cap = 16;

    while (!at_end(lexer)) {
        printf("ch: %c @ %i:%i\n", lexer->ch, lexer->line, lexer->col);
        // Comment
        if (lexer->ch == '/') {
            char c = peek(lexer);

            if (c == '/' && lexer->comment_depth == 0) {
                while (!at_end(lexer) && lexer->ch != '\n') {
                    next(lexer);
                }

                if (at_end(lexer)) {
                    break;
                }

                next(lexer);
            } else if (c == '*') {
                lexer->comment_depth++;
            }
        } else if (lexer->comment_depth > 0 && lexer->ch == '*') {
            if (peek(lexer) == '/') {
                next(lexer); // To move ch: '*' -> '/'
                lexer->comment_depth--;
            }

            // If not in comment anymore, move ch: '/' -> {next}
            // Otherwise, the Skip Comments section will handle it (or otherwise it would skip the char right after the '/')
            if (lexer->comment_depth == 0) next(lexer);
        }

        // Skip Comments
        if (lexer->comment_depth > 0) {
            next(lexer);
            continue;
        }

        // Skip Whitespace
        if (isspace(lexer->ch)) {
            next(lexer);
            continue;
        }

        // Append Token or Error
        struct lexer_result result = lex_token(lexer);
        if (result.result) {
            RESIZING_APPEND(tokens, struct token, tok_len, tok_cap, result.success)
        } else {
            RESIZING_APPEND(errors, struct lexer_error, *err_len, err_cap, result.error)
        }
    }

    // Check if Comment is Unterminated at EOF
    if (lexer->comment_depth > 0) {
        RESIZING_APPEND(errors, struct lexer_error, *err_len, err_cap, init_error(LEXER_UNTERMINATED_COMMENT, prev(lexer), lexer->line, lexer->col - 1))
    }

    // Append TT_EOF Token
    if (tok_len == tok_cap) {
        tok_cap += 1;
        tokens = (struct token *) realloc(tokens, sizeof(struct token) * tok_cap);
    }

    tokens[tok_len++] = init_token(TT_EOF, lexer->line, lexer->col, "", 0);

    return tokens;
}
