#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "tt.h"
#include "token.h"

// Str should not be NULL
int my_strtol(const char *str) {
    int base = 10;
    if (str[0] == '0' && str[1]) {
        char prefix = tolower(str[1]);
        switch (prefix) {
            case 'b': base = 2;  str += 2; break;
            case 't': base = 3;  str += 2; break;
            case 'q': base = 4;  str += 2; break;
            case 'o': base = 8;  str += 2; break;
            case 'd': base = 10; str += 2; break;
            case 'x': base = 16; str += 2; break;
            case 'z': base = 36; str += 2; break;
            default: break; // unknown prefix: treat as decimal
        }
    }

    long result = 0;
    while (*str != '\0') {
        char c = *str;
        int value;

        if (isdigit(c)) {
            value = c - '0';
        } else if (isalpha(c)) {
            value = tolower(c) - 'a' + 10; // a->10, b->11, ...
        } else {
            break; // Non-digit character stops parsing
        }

        if (value >= base) {
            break; // Invalid digit for base stops parsing
        }

        // Overflow protection for long (not int!)
        if (result > (LONG_MAX - value) / base) {
            // Clamp to INT_MAX
            result = LONG_MAX;
            break;
        }

        result = result * base + value;
        str++;
    }

    return result;
}

enum str2int_errno {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
};

enum str2int_errno str2int(int *out, const char *s) {
    if (s[0] == '\0' || isspace((unsigned char) s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = my_strtol(s);

    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
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
            printf("NUM lexeme in TT_NUMBER: %s", token.lexeme);
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

