#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../token/token.h"

typedef struct {
    char* source;
    int position;
    int line;
} Lexer;

void init_lexer(Lexer* lexer, const char* filename);
Token next_token(Lexer* lexer);

#endif