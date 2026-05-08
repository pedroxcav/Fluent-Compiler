#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    ASTNode **data;
    int length;
    int capacity;
} NodeList;

static void list_init(NodeList *nodes) {
    nodes -> data = NULL; 
    nodes -> length = 0; 
    nodes -> capacity = 0; 
}

static void list_push(NodeList *nodes, ASTNode *node) {
    if (nodes -> length == nodes -> capacity) {
        nodes -> capacity = nodes -> capacity ? nodes -> capacity * 2 : 8;
        nodes -> data = realloc(nodes -> data, nodes -> capacity * sizeof(ASTNode *));
        if (!nodes -> data) {
            fprintf(stderr, "parser: out of memory\n");
            exit(1); 
        }
    }
    nodes -> data[nodes -> length++] = node;
}

static const char *token_type_name(TokenType type) {
    switch (type) {
        case TYPE_INTEGER: return "integer";
        case TYPE_FLOAT: return "float";
        case TYPE_STRING: return "string";
        case TYPE_BOOLEAN: return "boolean";
        case LIT_INTEGER: return "integer literal";
        case LIT_FLOAT: return "float literal";
        case LIT_STRING: return "string literal";
        case LIT_TRUE: return "true";
        case LIT_FALSE: return "false";
        case OP_PLUS: return "plus";
        case OP_MINUS: return "minus";
        case OP_TIMES: return "times";
        case OP_DIV: return "divided by";
        case OP_POW: return "to the power of";
        case OP_EQ: return "equals";
        case OP_NEQ: return "differs from";
        case OP_LT: return "is less than";
        case OP_GT: return "is greater than";
        case OP_LTE: return "is less than or equal to";
        case OP_GTE: return "is greater than or equal to";
        case KW_RECEIVES: return "receives";
        case KW_IF: return "if";
        case KW_ELSE: return "else";
        case KW_WHILE: return "while";
        case KW_FUNCTION: return "function";
        case KW_RETURN: return "return";
        case KW_THEN: return "then";
        case KW_END: return "end";
        case KW_SAY: return "say";
        case KW_VOID: return "void";
        case KW_NEGATIVE: return "negative";
        case IDENTIFIER: return "identifier";
        case LPAREN: return "(";
        case RPAREN: return ")";
        case COMMA: return ",";
        case SEMICOLON: return ";";
        case EOF_TOKEN: return "end of file";
        default: return "unknown";
    }
}

void init_parser(Parser *parser, Lexer *lexer) {
    parser -> lexer = lexer;
    parser -> current = next_token(lexer);
}

static Token advance(Parser *parser) {
    Token token = parser -> current;
    parser -> current = next_token(parser -> lexer);
    return token;
}

static Token peek(Parser *parser) {
    return peek_token(parser -> lexer);
}

static bool check(Parser *parser, TokenType type) {
    return parser -> current.type == type;
}

static Token expect(Parser *parser, TokenType type) {
    if (parser -> current.type != type) {
        fprintf(
            stderr, 
            "Parse error on line %d: expected '%s', got '%s'\n",
            parser -> current.line,
            token_type_name(type),
            parser -> current.lexeme ? parser -> current.lexeme : token_type_name(parser -> current.type)
        );
        exit(1);
    }
    return advance(parser);
}

static bool is_type_token(TokenType type) {
    return 
        type == TYPE_INTEGER || type == TYPE_FLOAT ||
        type == TYPE_STRING  || type == TYPE_BOOLEAN;
}

static SemanticType get_semantic_type(TokenType type) {
    switch (type) {
        case TYPE_INTEGER: return SEMANTIC_INTEGER;
        case TYPE_FLOAT: return SEMANTIC_FLOAT;
        case TYPE_STRING: return SEMANTIC_STRING;
        case TYPE_BOOLEAN: return SEMANTIC_BOOLEAN;
        case KW_VOID: return SEMANTIC_VOID;
        default: return SEMANTIC_UNKNOWN;
    }
}

static const char *operator_name(TokenType type) {
    switch (type) {
        case OP_PLUS: return "plus";
        case OP_MINUS: return "minus";
        case OP_TIMES: return "times";
        case OP_DIV: return "divided by";
        case OP_POW: return "to the power of";
        case OP_EQ: return "equals";
        case OP_NEQ: return "differs from";
        case OP_LT: return "is less than";
        case OP_GT: return "is greater than";
        case OP_LTE: return "is less than or equal to";
        case OP_GTE: return "is greater than or equal to";
        default: return "?";
    }
}

static ASTNode *parse_statement(Parser *parser);
static ASTNode *parse_expression(Parser *parser);

static ASTNode *parse_atom(Parser *parser) {
    int line = parser -> current.line;
    if (check(parser, LIT_INTEGER)) {
        long long value = atoll(parser -> current.lexeme);
        advance(parser);
        return new_integer(value, line);
    }
    if (check(parser, LIT_FLOAT)) {
        double value = atof(parser -> current.lexeme);
        advance(parser);
        return new_float(value, line);
    }
    if (check(parser, LIT_STRING)) {
        char *value = strdup(parser -> current.lexeme);
        advance(parser);
        ASTNode *node = new_string(value, line);
        free(value);
        return node;
    }
    if (check(parser, LIT_TRUE)) {
        advance(parser); 
        return new_boolean(true,  line);
    }
    if (check(parser, LIT_FALSE)) {
        advance(parser); 
        return new_boolean(false, line);
    }
    if (check(parser, IDENTIFIER)) {
        char *name = strdup(parser -> current.lexeme);
        if (!name) { fprintf(stderr, "parser: out of memory\n"); exit(1); }
        advance(parser);
        if (check(parser, LPAREN)) {
            advance(parser);
            NodeList args; list_init(&args);
            if (!check(parser, RPAREN)) {
                list_push(&args, parse_expression(parser));
                while (check(parser, COMMA)) {
                    advance(parser);
                    list_push(&args, parse_expression(parser));
                }
            }
            expect(parser, RPAREN);
            ASTNode *call_node = new_call(name, args.data, args.length, line);
            free(name);
            return call_node;
        }
        ASTNode *id_node = new_identifier(name, line);
        free(name);
        return id_node;
    }
    if (check(parser, LPAREN)) {
        advance(parser);
        ASTNode *expression = parse_expression(parser);
        expect(parser, RPAREN);
        return expression;
    }
    fprintf(
        stderr, 
        "Parse error on line %d: unexpected token '%s' in expression\n",
        parser -> current.line,
        parser -> current.lexeme ? parser -> current.lexeme : token_type_name(parser -> current.type)
    );
    exit(1);
}

static ASTNode *parse_unary(Parser *parser) {
    int line = parser -> current.line;
    if (check(parser, KW_NEGATIVE)) {
        advance(parser);
        ASTNode *value = parse_unary(parser);
        return new_unary("negative", value, line);
    }
    return parse_atom(parser);
}

static ASTNode *parse_exponentiation(Parser *parser) {
    int line = parser -> current.line;
    ASTNode *left = parse_unary(parser);
    if (check(parser, OP_POW)) {
        advance(parser);
        ASTNode *right = parse_exponentiation(parser);
        return new_binop("to the power of", left, right, line);
    }
    return left;
}

static ASTNode *parse_multiplication(Parser *parser) {
    ASTNode *left = parse_exponentiation(parser);
    while (check(parser, OP_TIMES) || check(parser, OP_DIV)) {
        const char *operator = operator_name(parser -> current.type);
        int line = parser -> current.line;
        advance(parser);
        ASTNode *right = parse_exponentiation(parser);
        left = new_binop(operator, left, right, line);
    }
    return left;
}

static ASTNode *parse_addition(Parser *parser) {
    ASTNode *left = parse_multiplication(parser);
    while (check(parser, OP_PLUS) || check(parser, OP_MINUS)) {
        const char *operator = operator_name(parser -> current.type);
        int line = parser -> current.line;
        advance(parser);
        ASTNode *right = parse_multiplication(parser);
        left = new_binop(operator, left, right, line);
    }
    return left;
}

static ASTNode *parse_comparison(Parser *parser) {
    ASTNode *left = parse_addition(parser);
    TokenType token_type = parser -> current.type;
    if (
        token_type == OP_EQ  || token_type == OP_NEQ || token_type == OP_LT ||
        token_type == OP_GT  || token_type == OP_LTE || token_type == OP_GTE
    ) {
        const char *operator = operator_name(token_type);
        int line = parser -> current.line;
        advance(parser);
        ASTNode *right = parse_addition(parser);
        return new_binop(operator, left, right, line);
    }
    return left;
}

static ASTNode *parse_expression(Parser *parser) {
    return parse_comparison(parser);
}

static ASTNode **parse_body(Parser *parser, int *out_count) {
    NodeList nodes; list_init(&nodes);
    while (!check(parser, KW_END) && !check(parser, KW_ELSE) && !check(parser, EOF_TOKEN))
        list_push(&nodes, parse_statement(parser));
    *out_count = nodes.length;
    return nodes.data;
}

static ASTNode *parse_condition(Parser *parser) {
    int line = parser -> current.line;
    expect(parser, KW_IF);
    expect(parser, LPAREN);
    ASTNode *condition = parse_expression(parser);
    expect(parser, RPAREN);
    expect(parser, KW_THEN);
    int then_count = 0;
    ASTNode **then_body = parse_body(parser, &then_count);
    int else_count = 0;
    ASTNode **else_body = NULL;
    if (check(parser, KW_ELSE)) {
        advance(parser);
        else_body = parse_body(parser, &else_count);
    }
    expect(parser, KW_END);
    return new_condition(condition, then_body, then_count, else_body, else_count, line);
}

static ASTNode *parse_while(Parser *parser) {
    int line = parser -> current.line;
    expect(parser, KW_WHILE);
    expect(parser, LPAREN);
    ASTNode *condition = parse_expression(parser);
    expect(parser, RPAREN);
    expect(parser, KW_THEN);
    int count = 0;
    ASTNode **body = parse_body(parser, &count);
    expect(parser, KW_END);
    return new_while(condition, body, count, line);
}

static ASTNode *parse_return(Parser *parser) {
    int line = parser -> current.line;
    expect(parser, KW_RETURN);
    ASTNode *expression = NULL;
    if (!check(parser, SEMICOLON))
        expression = parse_expression(parser);
    expect(parser, SEMICOLON);
    return new_return(expression, line);
}

static ASTNode *parse_say(Parser *parser) {
    int line = parser -> current.line;
    expect(parser, KW_SAY);
    expect(parser, LPAREN);
    ASTNode *expression = parse_expression(parser);
    expect(parser, RPAREN);
    expect(parser, SEMICOLON);
    return new_say(expression, line);
}

static ASTNode *parse_declaration(Parser *parser) {
    int line = parser -> current.line;
    SemanticType type = get_semantic_type(parser -> current.type);
    advance(parser);
    Token name_token = expect(parser, IDENTIFIER);
    expect(parser, KW_RECEIVES);
    ASTNode *init_expression = parse_expression(parser);
    expect(parser, SEMICOLON);
    return new_declaration(type, name_token.lexeme, init_expression, line);
}

static ASTNode *parse_assignment(Parser *parser) {
    int line = parser -> current.line;
    Token name_token = expect(parser, IDENTIFIER);
    expect(parser, KW_RECEIVES);
    ASTNode *expression = parse_expression(parser);
    expect(parser, SEMICOLON);
    return new_assignment(name_token.lexeme, expression, line);
}

static ASTNode *parse_function_usage(Parser *parser) {
    int line = parser -> current.line;
    char *name = strdup(parser -> current.lexeme);
    if (!name) { fprintf(stderr, "parser: out of memory\n"); exit(1); }
    advance(parser);
    expect(parser, LPAREN);
    NodeList args; list_init(&args);
    if (!check(parser, RPAREN)) {
        list_push(&args, parse_expression(parser));
        while (check(parser, COMMA)) {
            advance(parser);
            list_push(&args, parse_expression(parser));
        }
    }
    expect(parser, RPAREN);
    expect(parser, SEMICOLON);
    ASTNode *call = new_call(name, args.data, args.length, line);
    free(name);
    return new_expression(call, line);
}

static ASTNode *parse_statement(Parser *parser) {
    if (check(parser, KW_IF)) return parse_condition(parser);
    if (check(parser, KW_WHILE)) return parse_while(parser);
    if (check(parser, KW_RETURN)) return parse_return(parser);
    if (check(parser, KW_SAY)) return parse_say(parser);
    if (is_type_token(parser -> current.type)) return parse_declaration(parser);
    if (check(parser, IDENTIFIER)) {
        Token next = peek(parser);
        if (next.type == KW_RECEIVES)return parse_assignment(parser);
        if (next.type == LPAREN) return parse_function_usage(parser);
    }
    fprintf(
        stderr, "Parse error on line %d: unexpected token '%s' in statement\n",
        parser -> current.line,
        parser -> current.lexeme ? parser -> current.lexeme : token_type_name(parser -> current.type)
    );
    exit(1);
}

static ASTNode *parse_function_definition(Parser *parser) {
    int line = parser -> current.line;
    SemanticType return_type = get_semantic_type(parser -> current.type);
    advance(parser);
    expect(parser, KW_FUNCTION);
    Token name_token = expect(parser, IDENTIFIER);
    expect(parser, LPAREN);
    NodeList parameters; list_init(&parameters);
    if (!check(parser, RPAREN)) {
        SemanticType parameter_type = get_semantic_type(parser -> current.type);
        if (!is_type_token(parser -> current.type)) {
            fprintf(stderr, "Parse error on line %d: expected parameter type\n", parser -> current.line);
            exit(1);
        }
        int parameter_line = parser -> current.line;
        advance(parser);
        Token parameter_name = expect(parser, IDENTIFIER);
        list_push(&parameters, new_parameter(parameter_type, parameter_name.lexeme, parameter_line));
        while (check(parser, COMMA)) {
            advance(parser);
            parameter_type = get_semantic_type(parser -> current.type);
            if (!is_type_token(parser -> current.type)) {
                fprintf(stderr, "Parse error on line %d: expected parameter type\n", parser -> current.line);
                exit(1);
            }
            parameter_line = parser -> current.line;
            advance(parser);
            parameter_name = expect(parser, IDENTIFIER);
            list_push(&parameters, new_parameter(parameter_type, parameter_name.lexeme, parameter_line));
        }
    }
    expect(parser, RPAREN);
    expect(parser, KW_THEN);
    int body_count = 0;
    ASTNode **body = parse_body(parser, &body_count);
    expect(parser, KW_END);
    return new_function(
        return_type, name_token.lexeme,
        parameters.data, parameters.length,
        body, body_count, line
    );
}

static ASTNode *parse_top_level(Parser *parser) {
    if (is_type_token(parser -> current.type) || check(parser, KW_VOID)) {
        Token next = peek(parser);
        if (next.type == KW_FUNCTION) 
            return parse_function_definition(parser);
        if (is_type_token(parser -> current.type))
            return parse_declaration(parser);
    }
    return parse_statement(parser);
}

ASTNode *parse_program(Parser *parser) {
    NodeList statements; list_init(&statements);
    while (!check(parser, EOF_TOKEN))
        list_push(&statements, parse_top_level(parser));
    return new_program(statements.data, statements.length, 1);
}