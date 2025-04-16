#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "token.h"
#include "tt.h"

struct lexer_error init_error(enum lexer_errno lerrno, char ch, int line) {
    return (struct lexer_error) { lerrno, ch, line };
}

struct lexer_result {
    int success;
    union {
        struct token result;
        struct lexer_error error;
    };
};

struct lexer_result ok(struct token result) {
    return (struct lexer_result) {
        .success = 1,
        .result = result
    };
}

struct lexer_result err(struct lexer_error error) {
    return (struct lexer_result) {
        .success = 0,
        .error = error
    };
}

struct lexer init_lexer(char *src) {
    return (struct lexer) {
        .ch = src,
        .line = 1,
        .comment_depth = 0,
        .in_string = 0
    };
}

int at_end(struct lexer *lexer) {
    return *lexer->ch == '\0';
}

void next(struct lexer *lexer) {
    lexer->ch++;
}

char peek(struct lexer *lexer) {
    if (at_end(lexer)) return '\0';
    else return lexer->ch[1];
}

int num_nested_comments(struct lexer *lexer) {
    return lexer->comment_depth - (lexer->comment_depth & 0x1);
}

/*
struct lexer_result lex_int(struct lexer *lexer) {

}

struct lexer_result lex_string(struct lexer *lexer) {
    char *c = lexer->ch;

    while (peek(lexer) != ')' || *lexer->ch == '\\' && peek(lexer) == ')') {
        if (*lexer->ch == '\0') {
            return err(init_error(LEXER_UNTERMINATED_STRING, lexer->ch[-1], lexer->line));
        }

        if (*lexer->ch == '\n') {
            return err(init_error(LEXER_MULTILINE_STRING, *lexer->ch, lexer->line));
        }

        next(lexer);
    }

    int llen = lexer->ch - c;
    char lexeme[llen + 2];
    strncpy(lexeme, c, llen + 1);
    lexeme[llen + 1] = '\0';

    char str[llen + 3];
    int slen = 0;
    
    int i = 0;
    for (char *ch = c; *ch != '\0'; ch++) {
        if (*ch == '\\') {
            switch (*(++ch--)) {
                case '\0':
                    return err(init_error(LEXER_UNTERMINATED_ESCAPE_SEQ, *ch, lexer->line));
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    struct lexer_result 
            }
        } else {

        }
    }

}
*/

// Should not ever be at a comment.
struct lexer_result lex_token(struct lexer *lexer) {
    switch (*lexer->ch) {
        // Operators
        case '/':
            next(lexer);
            return ok(init_token(TT_SLASH, lexer->line, "/", 1));
        case '.':
           next(lexer);
           return ok(init_token(TT_DOT, lexer->line, ".", 1));
        case '-':
             char c = peek(lexer);

            next(lexer);
            if (c == '>') {
                next(lexer);
                return ok(init_token(TT_RARROW, lexer->line, "->", 2));
             } else {
                return ok(init_token(TT_DASH, lexer->line, "-", 1));
        case '>':
            next(lexer);
            if (peek(lexer) == '>') {
                next(lexer);
                init_token(TT_RSHIFT, lexer->line, ">>", 2);
            } else {
                err(init_error(LEXER_UNKNOWN, *lexer->ch, lexer->line));
            }
        case ':':
            next(lexer);
            return ok(init_token(TT_COLON, lexer->line, ":", 1));
        case '(': break;
        default:
            return err(init_error(LEXER_UNKNOWN, *lexer->ch, lexer->line));
        }
    }

    // Make sure in `lex` to check if src is 0 length, because this will crash if that happens
    if (num_nested_comments(lexer) > 0) return err(init_error(LEXER_UNTERMINATED_COMMENT, lexer->ch[-1], lexer->line));

    else return err(init_error(LEXER_INCOMPLETE, *lexer->ch, lexer->line));
}

struct token *lex(struct lexer *lexer, struct lexer_error *errors, int *num_errors) {
    if (peek(lexer) == '\0') {
        num_errors = 0;
        struct token *eof = (struct token *) malloc(sizeof(struct token));
        *eof = init_token(TT_EOF, lexer->line, "", 0);

        return eof;
    }

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
            if (*lexer->ch == '\n') lexer->line++;
            next(lexer);
        }
    }
}
