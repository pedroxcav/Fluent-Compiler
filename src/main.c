#include <stdio.h>
#include "../lexer/lexer.h"

int main(int argc, char* argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    Lexer lexer;
    init_lexer(&lexer, argv[1]);

    printf("--- Iniciando Lexical Analysis ---\n");

    Token t;
    while (true) {
        t = next_token(&lexer);
        
        if (t.type == EOF_TOKEN) break;

        if (t.lexeme != NULL) {
            printf("Linha %d | Tipo: %d | Lexema: [%s]\n", t.line, t.type, t.lexeme);
            free(t.lexeme);
        } else
            printf("Linha %d | Tipo: %d | Lexema: [NULL]\n", t.line, t.type);
    }

    printf("--- Fim da Analise ---\n");
    free(lexer.source);

    return 0;
}