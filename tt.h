#ifndef TT_H
#define TT_H

enum tt {
    TT_ERR,
    TT_EOF, TT_EOL,
    // Identifiers
    TT_IDENT,
    // Literals
    TT_STRING,
    TT_NUMBER,
    TT_TRUE, TT_FALSE,
    // Operators
    TT_SLASH,
    TT_DOT,
    TT_RARROW,
    TT_RSHIFT,
    TT_DASH,
    TT_COLON,
    // Separators
    TT_LPAREN, TT_RPAREN,
    TT_LBRACKET, TT_RBRACKET,
    TT_LBRACE, TT_RBRACE,
    // Keywords
    TT_DEF,
    TT_SET,
    TT_IMPORT
};

#endif
