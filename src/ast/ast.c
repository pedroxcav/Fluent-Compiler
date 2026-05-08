#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *semantic_type_name(SemanticType type) {
    switch (type) {
        case SEMANTIC_INTEGER: return "integer";
        case SEMANTIC_FLOAT: return "float";
        case SEMANTIC_STRING: return "string";
        case SEMANTIC_BOOLEAN: return "boolean";
        case SEMANTIC_VOID: return "void";
        default: return "unknown";
    }
}

static ASTNode *alloc_node(NodeType node_type, int line) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) { 
        fprintf(stderr, "ast: out of memory\n"); 
        exit(1); 
    }
    node -> node_type = node_type;
    node -> line = line;
    node -> semantic_type = SEMANTIC_UNKNOWN;
    return node;
}

ASTNode *new_program(ASTNode **children, int count, int line) {
    ASTNode *node = alloc_node(NODE_PROGRAM, line);
    node -> data.program.children = children;
    node -> data.program.count = count;
    return node;
}

ASTNode *new_declaration(SemanticType type, char *name, ASTNode *init, int line) {
    ASTNode *node = alloc_node(NODE_DECLARATION, line);
    node -> data.declaration.var_type = type;
    node -> data.declaration.name = strdup(name);
    node -> data.declaration.init_expression = init;
    return node;
}

ASTNode *new_assignment(char *name, ASTNode *expr, int line) {
    ASTNode *node = alloc_node(NODE_ASSIGNMENT, line);
    node -> data.assignment.name = strdup(name);
    node -> data.assignment.expr = expr;
    return node;
}

ASTNode *new_expression(ASTNode *call, int line) {
    ASTNode *node = alloc_node(NODE_EXPRESSION, line);
    node -> data.expression_statement.call = call;
    return node;
}

ASTNode *new_condition(ASTNode *condition, ASTNode **then_body, int then_count, ASTNode **else_body, int else_count, int line) {
    ASTNode *node = alloc_node(NODE_CONDITION, line);
    node -> data.condition_statement.condition = condition;
    node -> data.condition_statement.then_body = then_body;
    node -> data.condition_statement.then_count = then_count;
    node -> data.condition_statement.else_body = else_body;
    node -> data.condition_statement.else_count = else_count;
    return node;
}

ASTNode *new_while(ASTNode *condition, ASTNode **body, int count, int line) {
    ASTNode *node = alloc_node(NODE_WHILE, line);
    node -> data.while_statement.condition = condition;
    node -> data.while_statement.body = body;
    node -> data.while_statement.count = count;
    return node;
}

ASTNode *new_function(SemanticType ret, char *name, ASTNode **parameters, int parameter_count, ASTNode **body, int body_count, int line) {
    ASTNode *node = alloc_node(NODE_FUNCTION, line);
    node -> data.function_definition.return_type = ret;
    node -> data.function_definition.name = strdup(name);
    node -> data.function_definition.parameters = parameters;
    node -> data.function_definition.parameter_count = parameter_count;
    node -> data.function_definition.body = body;
    node -> data.function_definition.body_count = body_count;
    return node;
}

ASTNode *new_parameter(SemanticType type, char *name, int line) {
    ASTNode *node = alloc_node(NODE_PARAMETER, line);
    node -> data.parameter.type = type;
    node -> data.parameter.name = strdup(name);
    return node;
}

ASTNode *new_return(ASTNode *expr, int line) {
    ASTNode *node = alloc_node(NODE_RETURN, line);
    node -> data.return_statement.expr = expr;
    return node;
}

ASTNode *new_say(ASTNode *expr, int line) {
    ASTNode *node = alloc_node(NODE_SAY, line);
    node -> data.say.expr = expr;
    return node;
}

ASTNode *new_binop(const char *operator, ASTNode *left, ASTNode *right, int line) {
    ASTNode *node = alloc_node(NODE_BINOP, line);
    node -> data.binary_operation.operator = strdup(operator);
    node -> data.binary_operation.left = left;
    node -> data.binary_operation.right = right;
    return node;
}

ASTNode *new_unary(const char *operator, ASTNode *value, int line) {
    ASTNode *node = alloc_node(NODE_UNARY, line);
    node -> data.unary.operator = strdup(operator);
    node -> data.unary.value = value;
    return node;
}

ASTNode *new_call(char *name, ASTNode **arguments, int count, int line) {
    ASTNode *node = alloc_node(NODE_CALL, line);
    node -> data.call.name = strdup(name);
    node -> data.call.arguments = arguments;
    node -> data.call.argument_count = count;
    return node;
}

ASTNode *new_identifier(char *name, int line) {
    ASTNode *node = alloc_node(NODE_IDENTIFIER, line);
    node -> data.identifier.name = strdup(name);
    return node;
}

ASTNode *new_integer(long long value, int line) {
    ASTNode *node = alloc_node(NODE_INTEGER, line);
    node -> data.literal_integer.value = value;
    return node;
}

ASTNode *new_float(double value, int line) {
    ASTNode *node = alloc_node(NODE_FLOAT, line);
    node -> data.literal_float.value = value;
    return node;
}

ASTNode *new_string(char *value, int line) {
    ASTNode *node = alloc_node(NODE_STRING, line);
    node -> data.literal_string.value = strdup(value);
    return node;
}

ASTNode *new_boolean(bool value, int line) {
    ASTNode *node = alloc_node(NODE_BOOLEAN, line);
    node -> data.literal_boolean.value = value;
    return node;
}

ASTNode *new_cast(SemanticType to, ASTNode *expr, int line) {
    ASTNode *node = alloc_node(NODE_CAST, line);
    node -> data.cast.to   = to;
    node -> data.cast.expr = expr;
    node -> semantic_type  = to;
    return node;
}

void del_tree(ASTNode *node) {
    if (!node) return;
    switch (node -> node_type) {
        case NODE_PROGRAM:
            for (int index = 0; index < node -> data.program.count; index++)
                del_tree(node -> data.program.children[index]);
            free(node -> data.program.children);
            break;
        case NODE_DECLARATION:
            free(node -> data.declaration.name);
            del_tree(node -> data.declaration.init_expression);
            break;
        case NODE_ASSIGNMENT:
            free(node -> data.assignment.name);
            del_tree(node -> data.assignment.expr);
            break;
        case NODE_EXPRESSION:
            del_tree(node -> data.expression_statement.call);
            break;
        case NODE_CONDITION:
            del_tree(node -> data.condition_statement.condition);
            for (int index = 0; index < node -> data.condition_statement.then_count; index++)
                del_tree(node -> data.condition_statement.then_body[index]);
            free(node -> data.condition_statement.then_body);
            for (int index = 0; index < node -> data.condition_statement.else_count; index++)
                del_tree(node -> data.condition_statement.else_body[index]);
            free(node -> data.condition_statement.else_body);
            break;
        case NODE_WHILE:
            del_tree(node -> data.while_statement.condition);
            for (int index = 0; index < node -> data.while_statement.count; index++)
                del_tree(node -> data.while_statement.body[index]);
            free(node -> data.while_statement.body);
            break;
        case NODE_FUNCTION:
            free(node -> data.function_definition.name);
            for (int index = 0; index < node -> data.function_definition.parameter_count; index++)
                del_tree(node -> data.function_definition.parameters[index]);
            free(node -> data.function_definition.parameters);
            for (int index = 0; index < node -> data.function_definition.body_count; index++)
                del_tree(node -> data.function_definition.body[index]);
            free(node -> data.function_definition.body);
            break;
        case NODE_PARAMETER:
            free(node -> data.parameter.name);
            break;
        case NODE_RETURN:
            del_tree(node -> data.return_statement.expr);
            break;
        case NODE_SAY:
            del_tree(node -> data.say.expr);
            break;
        case NODE_BINOP:
            free(node -> data.binary_operation.operator);
            del_tree(node -> data.binary_operation.left);
            del_tree(node -> data.binary_operation.right);
            break;
        case NODE_UNARY:
            free(node -> data.unary.operator);
            del_tree(node -> data.unary.value);
            break;
        case NODE_CALL:
            free(node -> data.call.name);
            for (int index = 0; index < node -> data.call.argument_count; index++)
                del_tree(node -> data.call.arguments[index]);
            free(node -> data.call.arguments);
            break;
        case NODE_IDENTIFIER:
            free(node -> data.identifier.name);
            break;
        case NODE_STRING:
            free(node -> data.literal_string.value);
            break;
        case NODE_CAST:
            del_tree(node -> data.cast.expr);
            break;
        default:
            break;
    }
    free(node);
}

static void print_indent(int indent) {
    for (int index = 0; index < indent; index++) printf("  ");
}
void print_ast(ASTNode *node, int indent) {
    if (!node) return;
    print_indent(indent);
    switch (node -> node_type) {
        case NODE_PROGRAM:
            printf("Program\n");
            for (int index = 0; index < node -> data.program.count; index++)
                print_ast(node -> data.program.children[index], indent + 1);
            break;
        case NODE_DECLARATION:
            printf("Declaration: %s %s\n",
                semantic_type_name(node -> data.declaration.var_type),
                node -> data.declaration.name);
            print_ast(node -> data.declaration.init_expression, indent + 1);
            break;
        case NODE_ASSIGNMENT:
            printf("Assignment: %s\n", node -> data.assignment.name);
            print_ast(node -> data.assignment.expr, indent + 1);
            break;
        case NODE_EXPRESSION:
            printf("ExpressionStatement\n");
            print_ast(node -> data.expression_statement.call, indent + 1);
            break;
        case NODE_CONDITION:
            printf("If\n");
            print_indent(indent + 1); printf("Cond:\n");
            print_ast(node -> data.condition_statement.condition, indent + 2);
            print_indent(indent + 1); printf("Then:\n");
            for (int index = 0; index < node -> data.condition_statement.then_count; index++)
                print_ast(node -> data.condition_statement.then_body[index], indent + 2);
            if (node -> data.condition_statement.else_count > 0) {
                print_indent(indent + 1); printf("Else:\n");
                for (int index = 0; index < node -> data.condition_statement.else_count; index++)
                    print_ast(node -> data.condition_statement.else_body[index], indent + 2);
            }
            break;
        case NODE_WHILE:
            printf("While\n");
            print_indent(indent + 1); printf("Cond:\n");
            print_ast(node -> data.while_statement.condition, indent + 2);
            print_indent(indent + 1); printf("Body:\n");
            for (int index = 0; index < node -> data.while_statement.count; index++)
                print_ast(node -> data.while_statement.body[index], indent + 2);
            break;
        case NODE_FUNCTION:
            printf("Function: %s -> %s\n",
                node -> data.function_definition.name,
                semantic_type_name(node -> data.function_definition.return_type));
            for (int index = 0; index < node -> data.function_definition.parameter_count; index++)
                print_ast(node -> data.function_definition.parameters[index], indent + 1);
            for (int index = 0; index < node -> data.function_definition.body_count; index++)
                print_ast(node -> data.function_definition.body[index], indent + 1);
            break;
        case NODE_PARAMETER:
            printf("Param: %s %s\n",
                semantic_type_name(node -> data.parameter.type),
                node -> data.parameter.name);
            break;
        case NODE_RETURN:
            printf("Return\n");
            print_ast(node -> data.return_statement.expr, indent + 1);
            break;
        case NODE_SAY:
            printf("Say\n");
            print_ast(node -> data.say.expr, indent + 1);
            break;
        case NODE_BINOP:
            printf("BinOp: %s\n", node -> data.binary_operation.operator);
            print_ast(node -> data.binary_operation.left, indent + 1);
            print_ast(node -> data.binary_operation.right, indent + 1);
            break;
        case NODE_UNARY:
            printf("Unary: %s\n", node -> data.unary.operator);
            print_ast(node -> data.unary.value, indent + 1);
            break;
        case NODE_CALL:
            printf("Call: %s\n", node -> data.call.name);
            for (int index = 0; index < node -> data.call.argument_count; index++)
                print_ast(node -> data.call.arguments[index], indent + 1);
            break;
        case NODE_IDENTIFIER:
            printf("Identifier: %s\n", node -> data.identifier.name);
            break;
        case NODE_INTEGER:
            printf("Integer: %lld\n", node -> data.literal_integer.value);
            break;
        case NODE_FLOAT:
            printf("Float: %g\n", node -> data.literal_float.value);
            break;
        case NODE_STRING:
            printf("String: \"%s\"\n", node -> data.literal_string.value);
            break;
        case NODE_BOOLEAN:
            printf("Bool: %s\n", node -> data.literal_boolean.value ? "true" : "false");
            break;
        case NODE_CAST:
            printf("Cast: %s -> %s\n",
                semantic_type_name(node -> data.cast.expr -> semantic_type),
                semantic_type_name(node -> data.cast.to));
            print_ast(node -> data.cast.expr, indent + 1);
            break;
        default:
            printf("Unknown node\n");
            break;
    }
}