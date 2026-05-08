#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "semantic/semantic.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file.fluent>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file"); 
        return 1; 
    }
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *source = malloc(size + 1);
    if (!source) { 
        fprintf(stderr, "Out of memory\n"); 
        return 1; 
    }
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);

    Lexer lexer;
    init_lexer(&lexer, source);

    Parser parser;
    init_parser(&parser, &lexer);

    ASTNode *program = parse_program(&parser);

    semantic_analyse(program);

    printf("\n");
    print_ast(program, 0);

    del_tree(program);
    del_lexer(&lexer);

    printf("\n");
    return 0;
}