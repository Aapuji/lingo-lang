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

struct lexer_result lex_string(struct lexer *lexer) {
    int line = lexer->line;
    int col = lexer->col;

    size_t len = 0;
    size_t cap = 8;
    char *str = malloc(sizeof(char) * cap);

    int num_trailing_bs;

    next(lexer);

    // While not )
    while (lexer->ch != ')' || num_trailing_bs % 2 != 0) {
        if (at_end(lexer)) {
            return err(init_error(LEXER_UNTERMINATED_STRING, prev(lexer), lexer->line, lexer->col));
        }

        if (lexer->ch == '\\') num_trailing_bs++;
        else num_trailing_bs = 0;

        RESIZING_APPEND(str, char, len, cap, lexer->ch)

        next(lexer);
    }
    next(lexer);

    if (len == cap) {
        str = (char *) realloc(str, sizeof(char) * ++cap);
        if (str == NULL) {
            perror("Realloc failed");
            exit(1);
        }
    }
    str[len++] = '\0';

    printf("STR: %s\n", str);

    // Since no error, it will store the line:col position of the start of the string.
    return ok(init_token(TT_STRING, line, col, str, len));
}

/// Argument `base` must either be 2, 3, 4, 8, 10, 16, or 36.
///
/// Must be called only when `lexer->ch` is an alphanumeric character.
///
/// Argument `base_char` is a char representing the base prefix (eg. b/B for binary, etc.), or is '\0' if there was no base string.
struct lexer_result lex_number(struct lexer *lexer, int base, char base_char) {
    int cap = 8;
    int len = base_char == '\0' ? 1 : 2;
    char *lexeme = malloc(sizeof(char) * cap);
    lexeme[0] = '0';
    lexeme[1] = base_char;

    while (isalnum(lexer->ch)) {
        if (isdigit(lexer->ch)) {
            if (lexer->ch >= '0' + base) err(init_error(LEXER_INVALID_DIGIT, lexer->ch, lexer->line, lexer->col));
        } else if (isalpha(lexer->ch)) {
            if (base <= 10) return err(init_error(LEXER_INVALID_DIGIT, lexer->ch, lexer->line, lexer->col));

            base -= 10;
            char ch = tolower(lexer->ch);

            if (ch >= 'a' + base) err(init_error(LEXER_INVALID_DIGIT, lexer->ch, lexer->line, lexer->col));
        }

        RESIZING_APPEND(lexeme, char, len, cap, lexer->ch)
        next(lexer);
    }

    // Add null byte
    if (len == cap) {
        lexeme = (char *) realloc(lexeme, sizeof(char) * ++cap);
        if (lexeme == NULL) {
            perror("Realloc failed");
            exit(1);
        }
    }
    lexeme[len++] = '\0';

    printf("NUM: %s", lexeme);

    return ok(init_token(TT_NUMBER, lexer->line, lexer->col, lexeme, len));
}

//struct lexer_result lex_ident(struct lexer *lexer) {

//}

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
        case '0': {
            char ch = peek(lexer);

            next(lexer);
            if (ch == 'b' || ch == 'B') {
                next(lexer);
                return lex_number(lexer, 2, ch);
            } else if (ch == 't' || ch == 'T') {
                next(lexer);
                return lex_number(lexer, 3, ch);
            } else if (ch == 'q' || ch == 'Q') {
                next(lexer);
                return lex_number(lexer, 4, ch);
            } else if (ch == 'o' || ch == 'O') {
                next(lexer);
                return lex_number(lexer, 8, ch);
            } else if (ch == 'd' || ch == 'D') {
                next(lexer);
                return lex_number(lexer, 10, ch);
            } else if (ch == 'x' || ch == 'd') {
                next(lexer);
                return lex_number(lexer, 16, ch);
            } else if (ch == 'z' || ch == 'Z') {
                next(lexer);
                return lex_number(lexer, 36, ch);
            } else if (isdigit(ch)) {
                return lex_number(lexer, 10, '\0');
            } else {
                return ok(init_token(TT_NUMBER, lexer->line, lexer->col, "0", 0));
            }
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return lex_number(lexer, 10, '\0');
        case '(':
            return lex_string(lexer);
        case '[':
        case '{':
        default:
            if (isalpha(lexer->ch)) {
                //return lex_ident(lexer);
            } else {
                next(lexer);
                return err(init_error(LEXER_UNKNOWN, prev(lexer), lexer->line, lexer->col - 1));
            }
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
        RESIZING_APPEND(
            errors, 
            struct lexer_error, 
            *err_len, 
            err_cap, 
            init_error(LEXER_UNTERMINATED_COMMENT, prev(lexer), lexer->line, lexer->col - 1))
    }

    // Append TT_EOF Token
    if (tok_len == tok_cap) {
        tok_cap += 1;
        tokens = (struct token *) realloc(tokens, sizeof(struct token) * tok_cap);
    }

    tokens[tok_len++] = init_token(TT_EOF, lexer->line, lexer->col, "", 0);

    return tokens;
}
