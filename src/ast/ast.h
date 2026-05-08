#ifndef AST_H
#define AST_H

#include <stdbool.h>

typedef enum {
    SEMANTIC_UNKNOWN = 0,
    SEMANTIC_INTEGER,
    SEMANTIC_FLOAT,
    SEMANTIC_STRING,
    SEMANTIC_BOOLEAN,
    SEMANTIC_VOID
} SemanticType;

const char *semantic_type_name(SemanticType t);

typedef enum {
    NODE_PROGRAM,
    NODE_DECLARATION,
    NODE_ASSIGNMENT,
    NODE_EXPRESSION,
    NODE_CONDITION,
    NODE_WHILE,
    NODE_FUNCTION,
    NODE_PARAMETER,
    NODE_RETURN,
    NODE_SAY,
    NODE_BINOP,
    NODE_UNARY,
    NODE_CALL,
    NODE_IDENTIFIER,
    NODE_INTEGER,
    NODE_FLOAT,
    NODE_STRING,
    NODE_BOOLEAN,
    NODE_CAST        /* coerção implícita inserida pela análise semântica */
} NodeType;

typedef struct ASTNode ASTNode;

typedef struct {
    ASTNode **children;
    int count;
} NodeProgram;

typedef struct {
    SemanticType var_type;
    char *name;
    ASTNode *init_expression;
} NodeDeclaration;

typedef struct {
    char *name;
    ASTNode *expr;
} NodeAssignment;

typedef struct {
    ASTNode *call;
} NodeExpression;

typedef struct {
    ASTNode *condition;
    ASTNode **then_body;
    int then_count;
    ASTNode **else_body;
    int else_count;
} NodeCondition;

typedef struct {
    ASTNode *condition;
    ASTNode **body;
    int count;
} NodeWhile;

typedef struct {
    SemanticType return_type;
    char *name;
    ASTNode **parameters;
    int parameter_count;
    ASTNode **body;
    int body_count;
} NodeFunction;

typedef struct {
    SemanticType type;
    char *name;
} NodeParameter;

typedef struct {
    ASTNode *expr;
} NodeReturnStatement;

typedef struct {
    ASTNode *expr;
} NodeSay;

typedef struct {
    char *operator;
    ASTNode *left;
    ASTNode *right;
} NodeBinop;

typedef struct {
    char *operator;
    ASTNode *value;
} NodeUnary;

typedef struct {
    char *name;
    ASTNode **arguments;
    int argument_count;
} NodeCall;

typedef struct {
    char *name;
} NodeIdentifier;

typedef struct {
    long long value; 
} NodeInteger;

typedef struct { 
    double value; 
} NodeFloat;

typedef struct { 
    char *value; 
} NodeString;

typedef struct { 
    bool value; 
} NodeBoolean;

/* Coerção implícita: envolve uma expressão e indica o tipo alvo.
 * Inserido pela análise semântica, nunca pelo parser. */
typedef struct {
    SemanticType to;   /* tipo destino */
    ASTNode     *expr; /* expressão original */
} NodeCast;

struct ASTNode {
    NodeType node_type;
    int line;
    SemanticType semantic_type;
    union {
        NodeProgram program;
        NodeDeclaration declaration;
        NodeAssignment assignment;
        NodeExpression expression_statement;
        NodeCondition condition_statement;
        NodeWhile while_statement;
        NodeFunction function_definition;
        NodeParameter parameter;
        NodeReturnStatement return_statement;
        NodeSay say;
        NodeBinop binary_operation;
        NodeUnary unary;
        NodeCall call;
        NodeIdentifier identifier;
        NodeInteger literal_integer;
        NodeFloat literal_float;
        NodeString literal_string;
        NodeBoolean literal_boolean;
        NodeCast cast;
    } data;
};

ASTNode *new_program(ASTNode **children, int count, int line);
ASTNode *new_declaration(SemanticType type, char *name, ASTNode *init, int line);
ASTNode *new_assignment(char *name, ASTNode *expr, int line);
ASTNode *new_expression(ASTNode *call, int line);
ASTNode *new_condition(ASTNode *condition, ASTNode **then_body, int then_count, ASTNode **else_body, int else_count, int line);
ASTNode *new_while(ASTNode *condition, ASTNode **body, int count, int line);
ASTNode *new_function(SemanticType ret, char *name, ASTNode **parameters, int parameter_count, ASTNode **body, int body_count, int line);
ASTNode *new_parameter(SemanticType type, char *name, int line);
ASTNode *new_return(ASTNode *expr, int line);
ASTNode *new_say(ASTNode *expr, int line);
ASTNode *new_binop(const char *operator, ASTNode *left, ASTNode *right, int line);
ASTNode *new_unary(const char *operator, ASTNode *value, int line);
ASTNode *new_call(char *name, ASTNode **arguments, int count, int line);
ASTNode *new_identifier(char *name, int line);
ASTNode *new_integer(long long value, int line);
ASTNode *new_float(double value, int line);
ASTNode *new_string(char *value, int line);
ASTNode *new_boolean(bool value, int line);
ASTNode *new_cast(SemanticType to, ASTNode *expr, int line);

void print_ast(ASTNode *node, int indent);
void del_tree(ASTNode *node);

#endif
