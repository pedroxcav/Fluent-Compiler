#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    // DATA TYPES
    TYPE_INTEGER,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOLEAN,

    // OPERATORS
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

    // KEY-WORDS
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FUNCTION,
    KW_RETURN,
    KW_THEN,
    KW_END,
    KW_SAY,
    KW_TRUE,
    KW_FALSE,
    IDENTIFIER,
    LPAREN,
    RPAREN,
    COMMA,

    // END OF FILE
    EOF_TOKEN,

    // UNKNOWN TOKEN
    UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;

} Token;

#endif
