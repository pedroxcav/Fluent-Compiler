#include <stdio.h>
#include "lexer.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo.fluent>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Erro ao abrir arquivo");
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

    printf("--- Analise Lexica iniciada ---\n");

    Token token;
    do {
        token = next_token(&lexer);
        printf("[Linha %02d] Tipo: %-5d | Lexema: '%s'\n", token.line, token.type, token.lexeme ? token.lexeme : token.type == SEMICOLON ? ";" : "EOF");
        if (token.lexeme) 
            free(token.lexeme);
    } while (token.type != EOF_TOKEN);

    printf("--- Analise Lexica concluida ---\n");

    del_lexer(&lexer);
    return 0;
}
