#include "parser.h"

// Both functions call each other before declared
// So we define them at the start to avoid errors
static ASTNode *parse_statement(Parser *parser);
static ASTNode *parse_expression(Parser *parser);

static int is_type_keyword(TokenType type) {
    return 
        type == TYPE_INTEGER || 
        type == TYPE_FLOAT || 
        type == TYPE_STRING  || 
        type == TYPE_BOOLEAN;
}

// Just moves our view of tokens forward
static void advance(Parser *parser) {
    parser->current_token = parser->lookahead_token;
    parser->lookahead_token = next_token(parser->lexer);
}

// Different from consume, this one returns the lexeme of the expected token
// As we may have an identifier or literal value that we want to keep in the tree
static char *expect(Parser *parser, TokenType expected_type) {
    if (parser -> current_token.type != expected_type) {
        fprintf(
            stderr,
            "Parse error on line %d: expected token type %d but got %d ('%s')\n",
            parser -> current_token.line,
            expected_type,
            parser -> current_token.type,
            parser -> current_token.lexeme ? parser -> current_token.lexeme : "NULL"
        );
        exit(1);
    }
    // Changes the lexeme from the lexer to the node
    char *lexeme = parser -> current_token.lexeme;
    parser -> current_token.lexeme = NULL;
    advance(parser);
    return lexeme;
}

// It discards the lexeme as we don't need it for operators or keywords
// Since their meanings are already represented by the token type
static void consume(Parser *parser) {
    free(parser -> current_token.lexeme);
    parser -> current_token.lexeme = NULL;
    // We also advance our view of tokens
    advance(parser);
}

static ASTNode *parse_atom(Parser *parser) {
    int line = parser -> current_token.line;

    switch (parser -> current_token.type) {
        case LIT_INTEGER: {
            ASTNode *node = new_node(NODE_LIT_INTEGER, line);
            node -> value = expect(parser, LIT_INTEGER);
            return node;
        }
        case LIT_FLOAT: {
            ASTNode *node = new_node(NODE_LIT_FLOAT, line);
            node -> value = expect(parser, LIT_FLOAT);
            return node;
        }
        case LIT_STRING: {
            ASTNode *node = new_node(NODE_LIT_STRING, line);
            node -> value = expect(parser, LIT_STRING);
            return node;
        }
        case LIT_TRUE: {
            consume(parser);
            return new_node(NODE_LIT_TRUE, line);
        }
        case LIT_FALSE: {
            consume(parser);
            return new_node(NODE_LIT_FALSE, line);
        }
        case IDENTIFIER: {
            if (parser -> lookahead_token.type == LPAREN) {
                ASTNode *call_node = new_node(NODE_FUNCTION_CALL, line);
                call_node -> value = expect(parser, IDENTIFIER);

                consume(parser); // Consume LPAREN

                ASTNode *argument_head = NULL;
                ASTNode *argument_tail = NULL;

                if (parser -> current_token.type != RPAREN) {
                    ASTNode *first_argument = parse_expression(parser);
                    argument_head = first_argument;
                    argument_tail = first_argument;

                    while (parser -> current_token.type == COMMA) {
                        consume(parser); // Consume the comma
                        ASTNode *next_argument = parse_expression(parser);
                        argument_tail -> next = next_argument;
                        argument_tail = next_argument;
                    }
                }
                expect(parser, RPAREN);
                call_node -> left = argument_head;
                return call_node;
            }
            ASTNode *identifier_node = new_node(NODE_IDENTIFIER, line);
            identifier_node -> value = expect(parser, IDENTIFIER);
            return identifier_node;
        }
        case LPAREN: {
            consume(parser); // Consume LPAREN
            ASTNode *inner = parse_expression(parser);
            expect(parser, RPAREN);
            return inner;
        }
        default:
            fprintf(
                stderr,
                "Parse error on line %d: unexpected token %d ('%s') in expression\n",
                line,
                parser -> current_token.type,
                parser -> current_token.lexeme ? parser -> current_token.lexeme : "NULL"
            );
            exit(1);
    }
}

static ASTNode *parse_unary(Parser *parser) {
    int line = parser -> current_token.line;

    if (parser -> current_token.type == KW_NEGATIVE) {
        consume(parser); // Consume KW_NEGATIVE
        ASTNode *node = new_node(NODE_UNARY_OP, line);
        node -> operator = KW_NEGATIVE;
        // Recursive call allows chaining: negative negative x
        node -> left = parse_unary(parser);
        return node;
    }
    return parse_atom(parser);
}

static ASTNode *parse_exponentiation(Parser *parser) {
    ASTNode *base = parse_unary(parser);

    if (parser -> current_token.type == OP_POW) {
        int line = parser -> current_token.line;
        consume(parser); // Consume OP_POW
        ASTNode *exponent = parse_exponentiation(parser);

        ASTNode *operator_node = new_node(NODE_BINARY_OP, line);
        operator_node -> operator = OP_POW;
        operator_node -> left = base;
        operator_node -> right = exponent;
        return operator_node;
    }

    return base;
}

static ASTNode *parse_multiplication(Parser *parser) {
    ASTNode *left = parse_exponentiation(parser);

    while (
        parser -> current_token.type == OP_TIMES || 
        parser -> current_token.type == OP_DIV
    ) {
        int line = parser -> current_token.line;
        TokenType operator = parser -> current_token.type;
        consume(parser);

        ASTNode *right = parse_exponentiation(parser);
        ASTNode *operator_node = new_node(NODE_BINARY_OP, line);
        operator_node -> operator = operator;
        operator_node -> left = left;
        operator_node -> right = right;
        left = operator_node;
    }
    return left;
}

static ASTNode *parse_addition(Parser *parser) {
    ASTNode *left = parse_multiplication(parser);

    while (
        parser -> current_token.type == OP_PLUS || 
        parser -> current_token.type == OP_MINUS
    ) {
        int line = parser -> current_token.line;
        TokenType operator = parser -> current_token.type;
        consume(parser);

        ASTNode *right = parse_multiplication(parser);
        ASTNode *operator_node = new_node(NODE_BINARY_OP, line);
        operator_node -> operator = operator;
        operator_node -> left = left;
        operator_node -> right = right;
        left = operator_node;
    }
    return left;
}

static ASTNode *parse_comparison(Parser *parser) {
    ASTNode *left = parse_addition(parser);

    TokenType current = parser->current_token.type;
    if (
        current == OP_EQ || current == OP_NEQ || 
        current == OP_LT || current == OP_GT || 
        current == OP_LTE || current == OP_GTE
    ) {
        int line = parser -> current_token.line;
        TokenType operator = current;
        consume(parser);

        ASTNode *right = parse_addition(parser);
        ASTNode *operator_node = new_node(NODE_BINARY_OP, line);
        operator_node -> operator = operator;
        operator_node -> left = left;
        operator_node -> right = right;
        return operator_node;
    }
    return left;
}

// Starts a whole recursive method that goes down the precedence levels until it reaches the atoms
static ASTNode *parse_expression(Parser *parser) {
    return parse_comparison(parser);
}

static ASTNode *parse_variable_declaration(Parser *parser) {
    int line = parser->current_token.line;

    ASTNode *node = new_node(NODE_VAR_DECL, line);
    node -> data_type = parser -> current_token.type;
    consume(parser); // Consume TYPE

    node -> value = expect(parser, IDENTIFIER);
    expect(parser, KW_RECEIVES);
    node -> left = parse_expression(parser);
    expect(parser, SEMICOLON);
    return node;
}

static ASTNode *parse_assignment(Parser *parser) {
    int line = parser -> current_token.line;

    ASTNode *node = new_node(NODE_ASSIGNMENT, line);
    node -> value = expect(parser, IDENTIFIER);
    expect(parser, KW_RECEIVES);
    node -> left = parse_expression(parser);
    expect(parser, SEMICOLON);
    return node;
}

// It builds a whole statement as we do for the main program, but it stops when it finds the end
static ASTNode *parse_block(Parser *parser, TokenType stop_a, TokenType stop_b) {
    ASTNode *head = NULL;
    ASTNode *tail = NULL;

    while (
        parser -> current_token.type != stop_a && 
        parser -> current_token.type != stop_b && 
        parser -> current_token.type != EOF_TOKEN
    ) {
        ASTNode *statement = parse_statement(parser);
        if (!head) {
            head = statement;
            tail = statement;
        } else {
            tail -> next = statement;
            tail = statement;
        }
    }
    return head;
}

static ASTNode *parse_if_statement(Parser *parser) {
    int line = parser->current_token.line;

    expect(parser, KW_IF);
    expect(parser, LPAREN);

    ASTNode *condition = parse_expression(parser);

    expect(parser, RPAREN);
    expect(parser, KW_THEN);

    ASTNode *then_body = parse_block(parser, KW_END, KW_ELSE);

    ASTNode *node = new_node(NODE_IF, line);
    node -> left = condition;
    node -> right = then_body;

    if (parser -> current_token.type == KW_ELSE) {
        consume(parser); // Consume KW_ELSE
        node -> else_branch = parse_block(parser, KW_END, KW_END);
    }
    expect(parser, KW_END);
    return node;
}

static ASTNode *parse_while_statement(Parser *parser) {
    int line = parser -> current_token.line;

    expect(parser, KW_WHILE);
    expect(parser, LPAREN);

    ASTNode *condition = parse_expression(parser);

    expect(parser, RPAREN);
    expect(parser, KW_THEN);

    ASTNode *body = parse_block(parser, KW_END, KW_END);

    expect(parser, KW_END);

    ASTNode *node = new_node(NODE_WHILE, line);
    node -> left = condition;
    node -> right = body;
    return node;
}

static ASTNode *parse_parameter(Parser *parser) {
    int line = parser -> current_token.line;
    ASTNode *node = new_node(NODE_PARAMETER, line);
    node -> data_type = parser -> current_token.type;
    consume(parser); // consume the type
    node -> value = expect(parser, IDENTIFIER);
    return node;
}

static ASTNode *parse_function_definition(Parser *parser) {
    int line = parser -> current_token.line;

    ASTNode *node = new_node(NODE_FUNCTION_DEF, line);

    if (!is_type_keyword(parser->current_token.type) && parser->current_token.type != KW_VOID) {
        fprintf(
            stderr,
            "Parse error on line %d: expected a type or 'void' before 'function', got '%s'\n",
            parser -> current_token.line,
            parser -> current_token.lexeme ? parser -> current_token.lexeme : "NULL"
        );
        exit(1);
    }
    node -> data_type = parser -> current_token.type;
    consume(parser); // Consume the type or void

    // Through the usage of expect functions
    // We have advance that goes forward in the parser
    // So that we can keep using expect and looking for the next expected tokens
    expect(parser, KW_FUNCTION);
    node -> value = expect(parser, IDENTIFIER);
    expect(parser, LPAREN);

    ASTNode *parameter_head = NULL;
    ASTNode *parameter_tail = NULL;

    // If a type has been found between the parentheses
    // We probably have a parameter, so we can start the parameters list
    if (is_type_keyword(parser -> current_token.type)) {
        ASTNode *first_parameter = parse_parameter(parser);
        parameter_head = first_parameter;
        parameter_tail = first_parameter;

        // The first parameter was set into head and tail
        // If we have commas, we know we have more parameters
        while (parser -> current_token.type == COMMA) {
            consume(parser); // Consume the comma
            // As we set head and tail with the same node
            // When i set the next of tail i also modify the head
            ASTNode *next_parameter = parse_parameter(parser);
            parameter_tail -> next = next_parameter;
            parameter_tail = next_parameter;
        }
    }
    expect(parser, RPAREN);
    expect(parser, KW_THEN);

    // Saves the whole block code as an independent statement
    ASTNode *body = parse_block(parser, KW_END, KW_END);

    expect(parser, KW_END);

    node -> right = parameter_head;
    node -> left = body;
    return node;
}

static ASTNode *parse_return_statement(Parser *parser) {
    int line = parser -> current_token.line;
    expect(parser, KW_RETURN);
    ASTNode *node = new_node(NODE_RETURN, line);
    node -> left = parse_expression(parser);
    expect(parser, SEMICOLON);
    return node;
}

static ASTNode *parse_say_statement(Parser *parser) {
    int line = parser -> current_token.line;
    expect(parser, KW_SAY);
    expect(parser, LPAREN);
    ASTNode *node = new_node(NODE_SAY, line);
    node -> left = parse_expression(parser);
    expect(parser, RPAREN);
    expect(parser, SEMICOLON);
    return node;
}

static ASTNode *parse_expression_statement(Parser *parser) {
    int line = parser -> current_token.line;

    // Only function calls are valid as standalone statements
    // A bare identifier followed by LPAREN is the only valid case
    ASTNode *node = new_node(NODE_EXPRESSION_STATEMENT, line);
    node -> left = parse_expression(parser);
    expect(parser, SEMICOLON);
    return node;
}

static ASTNode *parse_statement(Parser *parser) {
    // It starts understanding if already have a data type
    // If so, we gonna probably have a function or variable declaration
    if (parser->current_token.type == KW_VOID) {
        return parse_function_definition(parser);
    }
    if (is_type_keyword(parser -> current_token.type)) {
        if (parser -> lookahead_token.type == KW_FUNCTION)
            return parse_function_definition(parser);
        else
            return parse_variable_declaration(parser);
    }
    // For the other cases we may have
    // We can just check the current token
    switch (parser -> current_token.type) {
        case IDENTIFIER:
            // A function call used as a statement: greet("world");
            if (parser -> lookahead_token.type == LPAREN)
                return parse_expression_statement(parser);
            return parse_assignment(parser);
        case KW_IF:
            return parse_if_statement(parser);
        case KW_WHILE:
            return parse_while_statement(parser);
        case KW_RETURN:
            return parse_return_statement(parser);
        case KW_SAY:
            return parse_say_statement(parser);
        default:
            fprintf(
                stderr,
                "Parse error on line %d: unexpected token %d ('%s') at start of statement\n",
                parser -> current_token.line,
                parser -> current_token.type,
                parser -> current_token.lexeme ? parser -> current_token.lexeme : "NULL"
            );
            exit(1);
    }
}

ASTNode *parse(Lexer *lexer) {
    Parser parser;
    parser.lexer = lexer;

    // Initiates the parser with the current and next token
    // We're going to use them as our value to identify statements and expressions
    parser.current_token = next_token(lexer);
    parser.lookahead_token = next_token(lexer);

    ASTNode *program_node = new_node(NODE_PROGRAM, 1);

    // The head will be used as out entry point for the tree
    // The tail is used to save all the nodes through the execution
    ASTNode *statement_head = NULL;
    ASTNode *statement_tail = NULL;

    while (parser.current_token.type != EOF_TOKEN) {
        // Parse statement defines which statement we have now
        ASTNode *statement = parse_statement(&parser);
        // Here we set the head, if it is the first node
        if (!statement_head) {
            statement_head = statement;
            statement_tail = statement;
        // Or we add the node as the next of the previous
        // And then also set it as the current node we are working
        } else {
            statement_tail->next = statement;
            statement_tail = statement;
        }
    }

    program_node->left = statement_head;
    return program_node;
}