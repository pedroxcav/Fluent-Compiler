#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lexer/token.h"

typedef enum {
    // Statements
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_ASSIGNMENT,
    NODE_IF,
    NODE_WHILE,
    NODE_FUNCTION_DEF,
    NODE_PARAMETER,
    NODE_RETURN,
    NODE_SAY,
    NODE_EXPRESSION_STATEMENT,

    // Expressions
    NODE_BINARY_OP,
    NODE_UNARY_OP,
    NODE_FUNCTION_CALL,
    NODE_IDENTIFIER,
    NODE_LIT_INTEGER,
    NODE_LIT_FLOAT,
    NODE_LIT_STRING,
    NODE_LIT_TRUE,
    NODE_LIT_FALSE
} NodeType;

// A unique node created to be used for all the statement types
// The README.md especifies each property used by each node type
typedef struct ASTNode {
    NodeType type;
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *else_branch;
    struct ASTNode *next;
    char *value;
    TokenType operator;
    TokenType data_type;
    int line;
} ASTNode;

ASTNode *new_node(NodeType type, int line);
void del_ast(ASTNode *node);
void print_ast(ASTNode *node, int depth);

#endif