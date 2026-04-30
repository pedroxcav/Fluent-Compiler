#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lexer/lexer.h"
#include "../ast/ast.h"

typedef struct {
    Lexer *lexer;
    Token current_token;
    Token lookahead_token;
} Parser;

ASTNode *parse(Lexer *lexer);

#endif