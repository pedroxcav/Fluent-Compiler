#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>
#include <stdio.h>
#include "token.h"

#define REGEX_FLOAT 0
#define REGEX_INTEGER 1
#define REGEX_STRING 2
#define REGEX_IDENTIFIER 3
#define REGEX_WHITESPACE 4
#define REGEX_COMMENT 5
#define REGEX_COUNT 6

typedef struct {
    char *pattern;
    TokenType type;
    bool complex;
} TokenValue;

typedef struct {
    char *source;
    int position;
    int line;
    regex_t regex[REGEX_COUNT];
    bool  has_peeked;
    Token peeked;
} Lexer;

void  init_lexer(Lexer *lexer, char *source);
void  del_lexer(Lexer *lexer);
Token next_token(Lexer *lexer);
Token peek_token(Lexer *lexer);

#endif