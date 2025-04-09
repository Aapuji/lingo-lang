#include <errno.h>

#include "common.h"
#include "token.h"

typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

str2int_errno str2int(int *out, char *s) {
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

struct token init_token(enum tt tt, int line, char *lexeme, int len) {
    struct token token;
    token.tt = tt;
    token.line = line;

    switch (tt) {
        case STRING:
            token.data.str = strndup(lexeme, len+1);
            break;
        case NUMBER:
            char *s = strndup(lexeme, len+1);
            switch (str2int(&token.data.num, s)) {
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
        default:
            break;
    }

    return token;
}
