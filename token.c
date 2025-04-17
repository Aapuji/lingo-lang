#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "tt.h"
#include "token.h"

enum str2int_errno {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
};

enum str2int_errno str2int(int *out, char *s) {
    char *end;
    if (s[0] == '\0' || isspace((unsigned char) s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, 10);
    
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    
    *out = l;
    return STR2INT_SUCCESS;
}

/// Creates token given type, line number, lexeme, and length of lexeme (not including null byte).
struct token init_token(enum tt tt, int line, int col, char *lexeme, int len) {
    struct token token;
    token.tt = tt;
    token.line = line;
    token.col = col;
    
    char str[len + 1];
    token.lexeme = str;
    strncpy(token.lexeme, lexeme, len);
    token.lexeme[len] = '\0';

    switch (tt) {
        case TT_STRING: {
            token.data.str = ""; // Actually won't be used, i know, it's pretty ugly, but oh well
            break;
        }
        case TT_NUMBER: {
            switch (str2int(&token.data.num, token.lexeme)) {
                case STR2INT_OVERFLOW:
                    perror("Error: Number too big");
                    exit(1);
                case STR2INT_UNDERFLOW:
                    perror("Error: Number too small");
                    exit(1);
                case STR2INT_INCONVERTIBLE:
                    perror("Error converting number");
                    exit(1);
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }

    return token;
}

