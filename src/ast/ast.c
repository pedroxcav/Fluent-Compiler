#include "ast.h"

ASTNode *new_node(NodeType type, int line) {
    // With malloc we would have to set all properties manually
    // Altough with calloc we can set the default value to 0 automatically
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Out of memory allocating AST node\n");
        exit(1);
    }
    node->type = type;
    node->line = line;
    return node;
}

void del_ast(ASTNode *node) {
    if (!node) return;
    del_ast(node->left);
    del_ast(node->right);
    del_ast(node->else_branch);
    del_ast(node->next);
    free(node->value);
    free(node);
}

/* Recursively print the AST with indentation.
 * Every node is represented exclusively by token names — no lexemes.
 * depth controls the indentation level; call with depth=0 at the root.
 * Siblings linked through ->next are printed at the same depth.
 * This method was built with Artificial Intelligence */
void print_ast(ASTNode *node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; i++) printf("  ");

    switch (node->type) {
        case NODE_PROGRAM:
            printf("\nPROGRAM\n");
            print_ast(node->left, depth + 1);
            break;

        case NODE_VAR_DECL: {
            const char *dt = node->data_type == TYPE_INTEGER ? "TYPE_INTEGER" :
                             node->data_type == TYPE_FLOAT   ? "TYPE_FLOAT"   :
                             node->data_type == TYPE_STRING  ? "TYPE_STRING"  :
                             node->data_type == TYPE_BOOLEAN ? "TYPE_BOOLEAN" : "UNKNOWN";
            printf("%s IDENTIFIER KW_RECEIVES\n", dt);
            print_ast(node->left, depth + 1);
            break;
        }

        case NODE_ASSIGNMENT:
            printf("IDENTIFIER KW_RECEIVES\n");
            print_ast(node->left, depth + 1);
            break;

        case NODE_IF: {
            const char *op = "UNKNOWN";
            if (node->left && node->left->type == NODE_BINARY_OP) {
                switch (node->left->operator) {
                    case OP_EQ:  op = "OP_EQ";  break;
                    case OP_NEQ: op = "OP_NEQ"; break;
                    case OP_LT:  op = "OP_LT";  break;
                    case OP_GT:  op = "OP_GT";  break;
                    case OP_LTE: op = "OP_LTE"; break;
                    case OP_GTE: op = "OP_GTE"; break;
                    default: break;
                }
            }
            printf("KW_IF %s KW_THEN\n", op);
            print_ast(node->right, depth + 1);
            if (node->else_branch) {
                for (int i = 0; i < depth; i++) printf("  ");
                printf("KW_ELSE\n");
                print_ast(node->else_branch, depth + 1);
            }
            for (int i = 0; i < depth; i++) printf("  ");
            printf("KW_END\n");
            break;
        }

        case NODE_WHILE: {
            const char *op = "UNKNOWN";
            if (node->left && node->left->type == NODE_BINARY_OP) {
                switch (node->left->operator) {
                    case OP_EQ:  op = "OP_EQ";  break;
                    case OP_NEQ: op = "OP_NEQ"; break;
                    case OP_LT:  op = "OP_LT";  break;
                    case OP_GT:  op = "OP_GT";  break;
                    case OP_LTE: op = "OP_LTE"; break;
                    case OP_GTE: op = "OP_GTE"; break;
                    default: break;
                }
            }
            printf("KW_WHILE %s KW_THEN\n", op);
            print_ast(node->right, depth + 1);
            for (int i = 0; i < depth; i++) printf("  ");
            printf("KW_END\n");
            break;
        }

        case NODE_FUNCTION_DEF: {
            const char *dt = node->data_type == TYPE_INTEGER ? "TYPE_INTEGER" :
                             node->data_type == TYPE_FLOAT   ? "TYPE_FLOAT"   :
                             node->data_type == TYPE_STRING  ? "TYPE_STRING"  :
                             node->data_type == TYPE_BOOLEAN ? "TYPE_BOOLEAN" : "UNKNOWN";
            printf("%s KW_FUNCTION IDENTIFIER\n", dt);
            if (node->right) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("parameters:\n");
                print_ast(node->right, depth + 2);
            }
            if (node->left) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("body:\n");
                print_ast(node->left, depth + 2);
            }
            for (int i = 0; i < depth; i++) printf("  ");
            printf("KW_END\n");
            break;
        }

        case NODE_PARAMETER: {
            const char *dt = node->data_type == TYPE_INTEGER ? "TYPE_INTEGER" :
                             node->data_type == TYPE_FLOAT   ? "TYPE_FLOAT"   :
                             node->data_type == TYPE_STRING  ? "TYPE_STRING"  :
                             node->data_type == TYPE_BOOLEAN ? "TYPE_BOOLEAN" : "UNKNOWN";
            printf("%s IDENTIFIER\n", dt);
            break;
        }

        case NODE_RETURN:
            printf("KW_RETURN\n");
            print_ast(node->left, depth + 1);
            break;

        case NODE_SAY:
            printf("KW_SAY\n");
            print_ast(node->left, depth + 1);
            break;

        case NODE_BINARY_OP: {
            const char *op;
            switch (node->operator) {
                case OP_PLUS:  op = "OP_PLUS";  break;
                case OP_MINUS: op = "OP_MINUS"; break;
                case OP_TIMES: op = "OP_TIMES"; break;
                case OP_DIV:   op = "OP_DIV";   break;
                case OP_POW:   op = "OP_POW";   break;
                case OP_EQ:    op = "OP_EQ";    break;
                case OP_NEQ:   op = "OP_NEQ";   break;
                case OP_LT:    op = "OP_LT";    break;
                case OP_GT:    op = "OP_GT";    break;
                case OP_LTE:   op = "OP_LTE";   break;
                case OP_GTE:   op = "OP_GTE";   break;
                default:       op = "UNKNOWN";  break;
            }
            printf("%s\n", op);
            print_ast(node->left, depth + 1);
            print_ast(node->right, depth + 1);
            break;
        }

        case NODE_FUNCTION_CALL:
            printf("IDENTIFIER LPAREN\n");
            print_ast(node->left, depth + 1);
            for (int i = 0; i < depth; i++) printf("  ");
            printf("RPAREN\n");
            break;

        case NODE_EXPRESSION_STATEMENT:
            printf("NODE_EXPRESSION_STATEMENT\n");
            print_ast(node->left, depth + 1);
            break;

        case NODE_UNARY_OP:
            printf("KW_NEGATIVE\n");
            print_ast(node->left, depth + 1);
            break;

        case NODE_IDENTIFIER: printf("IDENTIFIER\n"); break;
        case NODE_LIT_INTEGER: printf("LIT_INTEGER\n"); break;
        case NODE_LIT_FLOAT: printf("LIT_FLOAT\n"); break;
        case NODE_LIT_STRING: printf("LIT_STRING\n"); break;
        case NODE_LIT_TRUE: printf("LIT_TRUE\n"); break;
        case NODE_LIT_FALSE: printf("LIT_FALSE\n"); break;
        default: break;
    }

    print_ast(node->next, depth);
}