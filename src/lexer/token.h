#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // Data types
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOLEAN,

    // Literals
    LIT_INTEGER,
    LIT_FLOAT,
    LIT_STRING,
    LIT_TRUE,
    LIT_FALSE,

    // Operators
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIV,
    OP_POW,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
    KW_RECEIVES,

    // Keywords
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FUNCTION,
    KW_RETURN,
    KW_THEN,
    KW_END,
    KW_SAY,
    IDENTIFIER,
    LPAREN,
    RPAREN,
    COMMA,

    // Delimiter
    SEMICOLON,

    // End of file
    EOF_TOKEN,

    // Unknown token
    UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
} Token;

#endif