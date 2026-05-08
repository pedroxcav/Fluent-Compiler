#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include "../lexer/lexer.h"
#include "../ast/ast.h"

typedef struct {
    Lexer *lexer;
    Token  current;
} Parser;

void init_parser(Parser *parser, Lexer *lexer);
ASTNode *parse_program(Parser *parser);

#endif
