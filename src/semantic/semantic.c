#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { 
    SYMBOL_VARIABLE, 
    SYMBOL_FUNCTION 
} SymbolType;

typedef struct {
    char *name;
    SymbolType symbol_type;
    SemanticType semantic_type;
    int parameter_count;
    SemanticType *parameter_types;
} Symbol;

typedef struct Scope Scope;
struct Scope {
    Symbol *symbols;
    int length;
    int capacity;
    Scope *parent;
};

static Scope *scope_new(Scope *parent) {
    Scope *scope = calloc(1, sizeof(Scope));
    if (!scope) {
        fprintf(stderr, "semantic: out of memory\n"); 
        exit(1);
    }
    scope -> parent = parent;
    return scope;
}

static void scope_free(Scope *scope) {
    for (int index = 0; index < scope -> length; index++) {
        free(scope -> symbols[index].name);
        free(scope -> symbols[index].parameter_types);
    }
    free(scope -> symbols);
    free(scope);
}

static Symbol *scope_find_local(Scope *scope, const char *name) {
    for (int index = 0; index < scope -> length; index++)
        if (strcmp(scope -> symbols[index].name, name) == 0)
            return &scope -> symbols[index];
    return NULL;
}

static Symbol *scope_find(Scope *scope, const char *name) {
    for (Scope *current = scope; current; current = current -> parent) {
        Symbol *symbol = scope_find_local(current, name);
        if (symbol) return symbol;
    }
    return NULL;
}

static Symbol *scope_add(Scope *scope, const char *name, SymbolType symbol_type, SemanticType semantic_type) {
    if (scope -> length == scope -> capacity) {
        scope -> capacity = scope -> capacity ? scope -> capacity * 2 : 8;
        scope -> symbols = realloc(scope -> symbols, scope -> capacity * sizeof(Symbol));
        if (!scope -> symbols) {
            fprintf(stderr, "semantic: out of memory\n"); 
            exit(1);
        }
    }
    Symbol *symbol = &scope -> symbols[scope -> length++];
    memset(symbol, 0, sizeof(Symbol));
    symbol -> name = strdup(name);
    symbol -> symbol_type = symbol_type;
    symbol -> semantic_type = semantic_type;
    return symbol;
}

typedef struct {
    Scope *current;
    bool in_function;
    SemanticType return_type;
    char *function_name;
    bool has_return;
} Analyser;

static void error_line(int line, const char *msg) {
    fprintf(stderr, "semantic error on line %d: %s\n", line, msg);
    exit(1);
}

static void error(const char *msg) {
    fprintf(stderr, "semantic error: %s\n", msg);
    exit(1);
}

/* Envolve *node em um NODE_CAST se o tipo atual diferir do tipo alvo.
 * Só atua entre integer e float. Atualiza o ponteiro no lugar. */
static void coerce(ASTNode **node, SemanticType target) {
    if (!node || !*node) return;
    SemanticType current = (*node) -> semantic_type;
    if (current == target) return;
    if ((current == SEMANTIC_INTEGER || current == SEMANTIC_FLOAT) &&
        (target  == SEMANTIC_INTEGER || target  == SEMANTIC_FLOAT)) {
        *node = new_cast(target, *node, (*node) -> line);
    }
}

static SemanticType analyse_expression(Analyser *analyser, ASTNode *node);
static void analyse_statement(Analyser *analyser, ASTNode *node);

static SemanticType arith_result(SemanticType left_type, SemanticType right_type, const char *operator, int line) {
    if (left_type == SEMANTIC_INTEGER && right_type == SEMANTIC_INTEGER) return SEMANTIC_INTEGER;
    if (left_type == SEMANTIC_FLOAT && right_type == SEMANTIC_FLOAT) return SEMANTIC_FLOAT;
    if ((left_type == SEMANTIC_INTEGER && right_type == SEMANTIC_FLOAT) || (left_type == SEMANTIC_FLOAT && right_type == SEMANTIC_INTEGER)
    ) return SEMANTIC_FLOAT;
    char msg[256];
    snprintf(msg, sizeof(msg),
             "type mismatch: cannot apply '%s' to %s and %s",
             operator, semantic_type_name(left_type), semantic_type_name(right_type));
    error_line(line, msg);
    return SEMANTIC_UNKNOWN;
}

static bool is_comparison_operator(const char *operator) {
    return
        strcmp(operator, "equals") == 0 ||
        strcmp(operator, "differs from") == 0 ||
        strcmp(operator, "is less than") == 0 ||
        strcmp(operator, "is greater than") == 0 ||
        strcmp(operator, "is less than or equal to") == 0 ||
        strcmp(operator, "is greater than or equal to") == 0;
}

static SemanticType analyse_expression(Analyser *analyser, ASTNode *node) {
    SemanticType semantic_type = SEMANTIC_UNKNOWN;
    switch (node -> node_type) {
        case NODE_INTEGER: semantic_type = SEMANTIC_INTEGER; break;
        case NODE_FLOAT: semantic_type = SEMANTIC_FLOAT; break;
        case NODE_STRING: semantic_type = SEMANTIC_STRING; break;
        case NODE_BOOLEAN: semantic_type = SEMANTIC_BOOLEAN; break;
        case NODE_IDENTIFIER: {
            Symbol *symbol = scope_find(analyser -> current, node -> data.identifier.name);
            if (!symbol || symbol -> symbol_type != SYMBOL_VARIABLE) {
                char msg[256];
                snprintf(msg, sizeof(msg), "undefined variable '%s'", node -> data.identifier.name);
                error_line(node -> line, msg);
            }
            semantic_type = symbol -> semantic_type;
            break;
        }
        case NODE_CALL: {
            const char *function_name = node -> data.call.name;
            Symbol *symbol = scope_find(analyser -> current, function_name);
            if (!symbol || symbol -> symbol_type != SYMBOL_FUNCTION) {
                char msg[256];
                snprintf(msg, sizeof(msg), "undefined function '%s'", function_name);
                error_line(node -> line, msg);
            }
            int expected = symbol -> parameter_count;
            int got = node -> data.call.argument_count;
            if (expected != got) {
                char msg[256];
                snprintf(msg, sizeof(msg), "function '%s' expects %d arguments, got %d", function_name, expected, got);
                error_line(node -> line, msg);
            }
            for (int index = 0; index < got; index++) {
                SemanticType argument_type = analyse_expression(analyser, node -> data.call.arguments[index]);
                SemanticType parameter_type = symbol -> parameter_types[index];
                if (argument_type != parameter_type) {
                    if (!(
                        (argument_type == SEMANTIC_INTEGER || argument_type == SEMANTIC_FLOAT) && 
                        (parameter_type == SEMANTIC_INTEGER || parameter_type == SEMANTIC_FLOAT)
                    )) {
                        char msg[256];
                        snprintf(
                            msg, 
                            sizeof(msg), 
                            "argument %d of '%s': expected %s, got %s", 
                            index + 1, 
                            function_name, 
                            semantic_type_name(parameter_type), 
                            semantic_type_name(argument_type)
                        );
                        error_line(node -> line, msg);
                    }
                    coerce(&node -> data.call.arguments[index], parameter_type);
                }
            }
            semantic_type = symbol -> semantic_type;
            break;
        }
        case NODE_BINOP: {
            SemanticType left_type = analyse_expression(analyser, node -> data.binary_operation.left);
            SemanticType right_type = analyse_expression(analyser, node -> data.binary_operation.right);
            const char *operator = node -> data.binary_operation.operator;
            if (is_comparison_operator(operator)) {
                bool is_equality = strcmp(operator, "equals") == 0 || strcmp(operator, "differs from") == 0;
                if (is_equality) {
                    if (left_type != right_type) {
                        if (!((left_type == SEMANTIC_INTEGER || left_type == SEMANTIC_FLOAT) &&
                              (right_type == SEMANTIC_INTEGER || right_type == SEMANTIC_FLOAT))) {
                            char msg[256];
                            snprintf(
                                msg, sizeof(msg),
                                "type mismatch: cannot apply '%s' to %s and %s",
                                operator, semantic_type_name(left_type), 
                                semantic_type_name(right_type)
                            );
                            error_line(node -> line, msg);
                        }
                    }
                } else {
                    arith_result(left_type, right_type, operator, node -> line);
                }
                semantic_type = SEMANTIC_BOOLEAN;
            } else {
                semantic_type = arith_result(left_type, right_type, operator, node -> line);
            }
            break;
        }
        case NODE_UNARY: {
            SemanticType operand_type = analyse_expression(analyser, node -> data.unary.value);
            if (operand_type != SEMANTIC_INTEGER && operand_type != SEMANTIC_FLOAT) {
                char msg[256];
                snprintf(
                    msg, sizeof(msg),
                    "type mismatch: cannot apply 'negative' to %s",
                    semantic_type_name(operand_type)
                );
                error_line(node -> line, msg);
            }
            semantic_type = operand_type;
            break;
        }
        default:
            fprintf(stderr, "semantic: unexpected expr node kind %d\n", node -> node_type);
            exit(1);
    }
    node -> semantic_type = semantic_type;
    return semantic_type;
}

static void analyse_body(Analyser *analyser, ASTNode **body, int count) {
    for (int index = 0; index < count; index++)
        analyse_statement(analyser, body[index]);
}

static void analyse_statement(Analyser *analyser, ASTNode *node) {
    switch (node -> node_type) {
        case NODE_DECLARATION: {
            const char *name = node -> data.declaration.name;
            if (scope_find_local(analyser -> current, name)) {
                char msg[256];
                snprintf(msg, sizeof(msg), "variable '%s' already declared", name);
                error_line(node -> line, msg);
            }
            SemanticType init_type = analyse_expression(analyser, node -> data.declaration.init_expression);
            SemanticType decl_type = node -> data.declaration.var_type;
            if (init_type != decl_type) {
                if (!(
                    (init_type == SEMANTIC_INTEGER || init_type == SEMANTIC_FLOAT) &&
                    (decl_type == SEMANTIC_INTEGER || decl_type == SEMANTIC_FLOAT)
                )) {
                    char msg[256];
                    snprintf(
                        msg, 
                        sizeof(msg),
                        "type mismatch in declaration of '%s': expected %s, got %s",
                        name, 
                        semantic_type_name(decl_type), 
                        semantic_type_name(init_type)
                    );
                    error_line(node -> line, msg);
                }
                coerce(&node -> data.declaration.init_expression, decl_type);
            }
            scope_add(analyser -> current, name, SYMBOL_VARIABLE, decl_type);
            break;
        }
        case NODE_ASSIGNMENT: {
            const char *name = node -> data.assignment.name;
            Symbol *symbol = scope_find(analyser -> current, name);
            if (!symbol || symbol -> symbol_type != SYMBOL_VARIABLE) {
                char msg[256];
                snprintf(msg, sizeof(msg), "undefined variable '%s'", name);
                error_line(node -> line, msg);
            }
            SemanticType expression_type = analyse_expression(analyser, node -> data.assignment.expr);
            if (expression_type != symbol -> semantic_type) {
                if (!((expression_type == SEMANTIC_INTEGER || expression_type == SEMANTIC_FLOAT) &&
                      (symbol -> semantic_type == SEMANTIC_INTEGER || symbol -> semantic_type == SEMANTIC_FLOAT))) {
                    char msg[256];
                    snprintf(
                        msg, 
                        sizeof(msg),
                        "type mismatch in assignment to '%s': expected %s, got %s",
                        name, 
                        semantic_type_name(symbol -> semantic_type), 
                        semantic_type_name(expression_type)
                    );
                    error_line(node -> line, msg);
                }
                coerce(&node -> data.assignment.expr, symbol -> semantic_type);
            }
            break;
        }
        case NODE_EXPRESSION:
            analyse_expression(analyser, node -> data.expression_statement.call);
            break;
        case NODE_CONDITION: {
            analyse_expression(analyser, node -> data.condition_statement.condition);
            bool saved = analyser -> has_return;

            Scope *then_scope = scope_new(analyser -> current);
            Scope *previous_scope = analyser -> current;
            analyser -> current = then_scope;
            analyser -> has_return = false;
            analyse_body(analyser, node -> data.condition_statement.then_body, node -> data.condition_statement.then_count);
            bool then_returns = analyser -> has_return;
            analyser -> current = previous_scope;
            scope_free(then_scope);

            bool else_returns = false;
            if (node -> data.condition_statement.else_count > 0) {
                Scope *else_scope = scope_new(analyser -> current);
                analyser -> current = else_scope;
                analyser -> has_return = false;
                analyse_body(analyser, node -> data.condition_statement.else_body, node -> data.condition_statement.else_count);
                else_returns = analyser -> has_return;
                analyser -> current = previous_scope;
                scope_free(else_scope);
            }
            analyser -> has_return = saved || (then_returns && else_returns);
            break;
        }
        case NODE_WHILE: {
            analyse_expression(analyser, node -> data.while_statement.condition);
            bool saved_while = analyser -> has_return;
            Scope *loop_scope = scope_new(analyser -> current);
            Scope *previous_scope = analyser -> current;
            analyser -> current = loop_scope;
            analyser -> has_return = false;
            analyse_body(analyser, node -> data.while_statement.body, node -> data.while_statement.count);
            analyser -> current = previous_scope;
            scope_free(loop_scope);
            analyser -> has_return = saved_while;
            break;
        }
        case NODE_RETURN: {
            if (!analyser -> in_function)
                error_line(node -> line, "'return' outside function");
            ASTNode *expression = node -> data.return_statement.expr;
            SemanticType actual = expression ? analyse_expression(analyser, expression) : SEMANTIC_VOID;
            SemanticType expected = analyser -> return_type;
            if (actual != expected) {
                if (!(
                    (actual == SEMANTIC_INTEGER || actual == SEMANTIC_FLOAT) &&
                    (expected == SEMANTIC_INTEGER || expected == SEMANTIC_FLOAT)
                )) {
                    char msg[256];
                    snprintf(
                        msg, 
                        sizeof(msg),
                        "return type mismatch in '%s': expected %s, got %s",
                        analyser -> function_name, 
                        semantic_type_name(expected), 
                        semantic_type_name(actual)
                    );
                    error_line(node -> line, msg);
                }
                coerce(&node -> data.return_statement.expr, expected);
            }
            analyser -> has_return = true;
            break;
        }
        case NODE_SAY:
            analyse_expression(analyser, node -> data.say.expr);
            break;
        case NODE_FUNCTION:
            fprintf(stderr, "semantic error: nested function definition not supported\n");
            exit(1);
        default:
            fprintf(stderr, "semantic: unexpected stmt node kind %d\n", node -> node_type);
            exit(1);
    }
}

static void register_function(Scope *global, ASTNode *node) {
    const char *name = node -> data.function_definition.name;
    if (scope_find_local(global, name)) {
        char msg[256];
        snprintf(msg, sizeof(msg), "function '%s' already declared", name);
        error_line(node -> line, msg);
    }
    Symbol *symbol = scope_add(global, name, SYMBOL_FUNCTION, node -> data.function_definition.return_type);
    int parameter_count = node -> data.function_definition.parameter_count;
    symbol -> parameter_count = parameter_count;
    if (parameter_count > 0) {
        symbol -> parameter_types = malloc(parameter_count * sizeof(SemanticType));
        if (!symbol -> parameter_types) {
            fprintf(stderr, "semantic: out of memory\n");
            exit(1);
        }
        for (int index = 0; index < parameter_count; index++)
            symbol -> parameter_types[index] = node -> data.function_definition.parameters[index] -> data.parameter.type;
    }
}

void semantic_analyse(ASTNode *program) {
    Scope *global = scope_new(NULL);
    Analyser analyser;
    analyser.current = global;
    analyser.in_function = false;
    analyser.return_type = SEMANTIC_VOID;
    analyser.function_name = NULL;
    analyser.has_return = false;
    for (int index = 0; index < program -> data.program.count; index++) {
        ASTNode *node = program -> data.program.children[index];
        if (node -> node_type == NODE_FUNCTION)
            register_function(global, node);
    }
    for (int index = 0; index < program -> data.program.count; index++) {
        ASTNode *node = program -> data.program.children[index];
        if (node -> node_type == NODE_FUNCTION) {
            Scope *func_scope = scope_new(global);
            Scope *previous_scope = analyser.current;
            analyser.current = func_scope;
            bool outer_in_function = analyser.in_function;
            SemanticType outer_return_type = analyser.return_type;
            char *outer_name = analyser.function_name;
            bool outer_has_return = analyser.has_return;
            analyser.in_function = true;
            analyser.return_type = node -> data.function_definition.return_type;
            analyser.function_name = node -> data.function_definition.name;
            analyser.has_return = false;
            for (int j = 0; j < node -> data.function_definition.parameter_count; j++) {
                ASTNode *param = node -> data.function_definition.parameters[j];
                if (scope_find_local(func_scope, param -> data.parameter.name)) {
                    char msg[256];
                    snprintf(msg, sizeof(msg), "duplicate parameter '%s' in function '%s'",
                             param -> data.parameter.name,
                             node -> data.function_definition.name);
                    error_line(param -> line, msg);
                }
                scope_add(func_scope, param -> data.parameter.name, SYMBOL_VARIABLE, param -> data.parameter.type);
            }
            analyse_body(&analyser, node -> data.function_definition.body, node -> data.function_definition.body_count);
            if (analyser.return_type != SEMANTIC_VOID && !analyser.has_return) {
                char msg[256];
                snprintf(msg, sizeof(msg), "missing return in function '%s'", analyser.function_name);
                error(msg);
            }
            analyser.current = previous_scope;
            analyser.in_function = outer_in_function;
            analyser.return_type = outer_return_type;
            analyser.function_name = outer_name;
            analyser.has_return = outer_has_return;
            scope_free(func_scope);
        } else {
            analyse_statement(&analyser, node);
        }
    }
    scope_free(global);
}
