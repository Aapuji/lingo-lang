#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "tt.h"
#include "token.h"

// Define my own strtol using a base implementation
// Thanks to Regents of University of California
long strtol_modified(const char *nptr, char **endptr, register int base) {
    register const char *s = nptr;
    register unsigned long acc;
    register int c;
    register unsigned long cutoff;
    register int neg = 0, any, cutlim;

    do {
        c = *s++;
    } while (isspace(c));

    if (c == '-') {
        neg = 1;
        c = *s++;
    } else if (c == '+')
        c = *s++;
    
    if (c == '0') {
        switch (*s) {
            case 'b':
            case 'B':
                base = 2;
                break;
            case 't':
            case 'T':
                base = 3;
                break;
            case 'q':
            case 'Q':
                base = 4;
                break;
            case 'o':
            case 'O':
                base = 8;
                break;
            case 'd':
            case 'D':
                base = 10;
                break;
            case 'x':
            case 'X':
                base = 16;
                break;
            case 'z':
            case 'Z':
                base = 36;
                break;
            default:
                base = 10;
        }

        c = s[1];
        s += 2;
    }

    cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
    cutlim = cutoff % (unsigned long)base;
    cutoff /= (unsigned long)base;
    for (acc = 0, any = 0;; c = *s++) {
        if (isdigit(c))
            c -= '0';
        else if (isalpha(c))
            c -= isupper(c) ? 'A' - 10 : 'a' - 10;
        else
            break;
        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else {
            any = 1;
            acc *= base;
            acc += c;
        }
    }
    if (any < 0) {
        acc = neg ? LONG_MIN : LONG_MAX;
        errno = ERANGE;
    } else if (neg)
        acc = -acc;
    if (endptr != 0)
        *endptr = (char *) (any ? s - 1 : nptr);
    return (acc);
}

enum str2int_errno {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
};

enum str2int_errno str2int(int *out, const char *s) {
    char *end;
    if (s[0] == '\0' || isspace((unsigned char) s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol_modified(s, &end, 10);
    
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    printf("NUM: %li", l);
    *out = l;
    return STR2INT_SUCCESS;
}

/// Generates the string from lexeme (ie. \n goes to newline, \) goes to ) )
/// `len` does not include null byte.
/// Number of trailing backslashes must be even. This is guaranteed by the lex_string function.
void lexeme2str(char *dest, const char *src, int len) {
    dest = malloc(sizeof(char) * (len + 1));
    int n = 0;

    for (int i = 0; i <= len; i++) {
        char ch = src[i];

        if (ch == '\\') {
            switch (src[i + 1]) {
                case ')':
                    dest[n++] = ')';
                    break;
                case '\\':
                    dest[n++] = ')';
                    break;
                case 'n':
                    dest[n++] = '\n';
                    break;
                case 't':
                    dest[n++] = '\t';
                    break;
                case 's':
                    dest[n++] = ' ';
                    break;
                default:
                    dest[n++] = '\\';
                    dest[n++] = ch;
                    break;
            }
            i++;
        } else {
            dest[n++] = ch;
        }
    }

    dest[n] = '\0';
}

// int lexeme2num(const char *src, int len) {
//     int value;

//     if (len == 1) {}
//     str2int(&value, src);
// }

/// Creates token given type, line number, lexeme, and length of lexeme (not including null byte).
struct token init_token(enum tt tt, int line, int col, char *lexeme, int len) {
    struct token token;
    token.tt = tt;
    token.line = line;
    token.col = col;
    token.lexeme = (char *) malloc(sizeof(char) * (len + 1));
    strncpy(token.lexeme, lexeme, len);
    token.lexeme[len] = '\0';

    switch (tt) {
        case TT_STRING: {
            lexeme2str(token.data.str, token.lexeme, len);
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

