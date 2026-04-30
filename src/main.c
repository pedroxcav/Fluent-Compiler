#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"

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
    fread(source, 1, size, file);
    source[size] = '\0';
    fclose(file);

    Lexer lexer;
    init_lexer(&lexer, source);

    ASTNode *ast = parse(&lexer);

    print_ast(ast, 0);

    del_ast(ast);
    del_lexer(&lexer);
    return 0;
}