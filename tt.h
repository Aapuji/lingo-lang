#ifndef TT_H
#define TT_H

enum tt {
    ERR,
    EOF, EOL,
    // Identifiers
    IDENT,
    // Literals
    STRING,
    NUMBER,
    TRUE, FALSE,
    // Operators
    SLASH,
    DOT,
    RARROW,
    RSHIFT,
    DASH,
    COLON,
    // Separators
    LPAREN, RPAREN,
    LBRACKET, RBRACKET,
    LBRACE, RBRACE,
    // Keywords
    DEF,
    SET,
    IMPORT
};

#endif
