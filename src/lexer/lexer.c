#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../token/token.h"

typedef struct {
    char* source;
    int position;
    int line;
} Lexer;

void init_lexer(Lexer* lexer) {
    lexer -> position = 0;
    lexer -> line = 1;
    
    // gets the source.fluent file
    FILE* file = fopen("source.fluent", "r");

    // return the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);
    
    // allocates memory for that place/pointer
    lexer -> source = malloc(file_size + 1);
    // indicates where the data will be stored
    // declares the size of each element to be read
    // show the maximum size of the data to be read
    // indicates the file stream to read from
    fread(lexer -> source, sizeof(char), file_size, file);
    lexer->source[file_size] = '\0';

    fclose(file);
}

char peek(Lexer* lexer) {
    // source is the whole code/text
    // we also have position as the "index property"
    return lexer -> source[lexer -> position];
}

char advance(Lexer* lexer) {
    // gets the current character from code
    char current_char = peek(lexer);
    lexer -> position++;

    // if the line has just end, increase it
    if (current_char == '\n') 
        lexer -> line++;
    else if (current_char == '\0') 
        return '\0';

    return current_char;
}

void skip_whitespace(Lexer* lexer) {
    // stops when a character has just been found
    while(true) {
        // gets the current character
        char current_char = peek(lexer);
        // if it is not a character, go to the next one
        if(current_char == ' ' || current_char == '\t' || current_char == '\r')
            advance(lexer);
        //if it is a comment, skip the whole line
        else if (current_char == '#')
            while(peek(lexer) != '\n') {
                advance(lexer);
                current_char = peek(lexer);
                if(current_char == '\0')
                    break;
            }
        // ends here if a character was found
        else
            break;
    }
}

bool match_phrase(Lexer* lexer, const char* phrase) {
    int length = strlen(phrase);
    if (strncmp(lexer->source + lexer->position, phrase, length) == 0) {
        lexer -> position += length;
        return true;
    }
    return false;
}

char* extract_lexeme(Lexer* lexer, int start_pos) {
    int length = lexer->position - start_pos;
    char* lexeme = malloc(length + 1);
    if (lexeme) {
        memcpy(lexeme, lexer->source + start_pos, length);
        lexeme[length] = '\0';
    }
    return lexeme;
}

Token next_token(Lexer* lexer) {
    while(true) {
        // skip all the spaces, tabs, and comments
        skip_whitespace(lexer);

        char current_char = peek(lexer);
        if(current_char == '\0') {
            Token token;
            token.type = EOF_TOKEN;
            token.lexeme = NULL;
            token.line = lexer -> line;
            return token;
        }

        if (current_char == '\n' || current_char == '\r') {
            advance(lexer);
            continue; 
        }
        
        Token token;
        token.line = lexer->line;

        if (isalpha(current_char)) {
            int start = lexer->position;
            while (isalnum(peek(lexer)) || peek(lexer) == '_')
                advance(lexer);

            char* lexeme = extract_lexeme(lexer, start);
            token.lexeme = lexeme;

            // depending on the word, we return a different token
            if (strcmp(lexeme, "is") == 0) {
                if (match_phrase(lexer, " greater than or equal to")) {
                    free(lexeme);
                    token.type = OP_GTE;
                    token.lexeme = strdup("is greater than or equal to");
                } else if (match_phrase(lexer, " greater than")) {
                    free(lexeme);
                    token.type = OP_GT;
                    token.lexeme = strdup("is greater than");
                } else if (match_phrase(lexer, " less than or equal to")) {
                    free(lexeme);
                    token.type = OP_LTE;
                    token.lexeme = strdup("is less than or equal to");
                } else if (match_phrase(lexer, " less than")) {
                    free(lexeme);
                    token.type = OP_LT;
                    token.lexeme = strdup("is less than");
                } else {
                    token.type = IDENTIFIER;
                }
            } else if (strcmp(lexeme, "differs") == 0) {
                if (match_phrase(lexer, " from")) {
                    free(lexeme);
                    token.type = OP_NEQ;
                    token.lexeme = strdup("differs from");
                } else {
                    token.type = IDENTIFIER;
                }
            } else if (strcmp(lexeme, "divided") == 0) {
                if (match_phrase(lexer, " by")) {
                    free(lexeme);
                    token.type = OP_DIV;
                    token.lexeme = strdup("divided by");
                } else {
                    token.type = IDENTIFIER;
                }
            } else if (strcmp(lexeme, "to") == 0) {
                if (match_phrase(lexer, " the power of")) {
                    free(lexeme);
                    token.type = OP_POW;
                    token.lexeme = strdup("to the power of");
                } else {
                    token.type = IDENTIFIER;
                }
            } else if (strcmp(lexeme, "plus") == 0)
                token.type = OP_PLUS;
            else if (strcmp(lexeme, "minus") == 0) 
                token.type = OP_MINUS;
            else if (strcmp(lexeme, "times") == 0) 
                token.type = OP_TIMES;
            else if (strcmp(lexeme, "equals") == 0) 
                token.type = OP_EQ;
            else if (strcmp(lexeme, "receives") == 0) 
                token.type = KW_RECEIVES;
            else if (strcmp(lexeme, "if") == 0) 
                token.type = KW_IF;
            else if (strcmp(lexeme, "else") == 0) 
                token.type = KW_ELSE;
            else if (strcmp(lexeme, "while") == 0) 
                token.type = KW_WHILE;
            else if (strcmp(lexeme, "function") == 0) 
                token.type = KW_FUNCTION;
            else if (strcmp(lexeme, "return") == 0) 
                token.type = KW_RETURN;
            else if (strcmp(lexeme, "then") == 0) 
                token.type = KW_THEN;
            else if (strcmp(lexeme, "end") == 0) 
                token.type = KW_END;
            else if (strcmp(lexeme, "say") == 0) 
                token.type = KW_SAY;
            else if (strcmp(lexeme, "true") == 0) 
                token.type = KW_TRUE;
            else if (strcmp(lexeme, "false") == 0) 
                token.type = KW_FALSE;
            else if (strcmp(lexeme, "integer") == 0) 
                token.type = TYPE_INTEGER;
            else if (strcmp(lexeme, "float") == 0) 
                token.type = TYPE_FLOAT;
            else if (strcmp(lexeme, "string") == 0) 
                token.type = TYPE_STRING;
            else if (strcmp(lexeme, "boolean") == 0) 
                token.type = TYPE_BOOLEAN;
            else 
                token.type = IDENTIFIER;
            return token;
        } else if (isdigit(current_char)) {
            int start = lexer->position;
            bool is_float = false;

            while (isdigit(peek(lexer)) || peek(lexer) == '.') {
                if (peek(lexer) == '.') {
                    if (is_float) break;
                    is_float = true;
                }
                advance(lexer);
            }
            token.lexeme = extract_lexeme(lexer, start);
            token.type = is_float ? TYPE_FLOAT : TYPE_INTEGER;
            return token;
        } else if (current_char == '"') {
            advance(lexer);
            int start = lexer->position;
            while (peek(lexer) != '"' && peek(lexer) != '\0')
                advance(lexer);
            int len = lexer->position - start;
            char* str_content = malloc(len + 1);
            memcpy(str_content, lexer->source + start, len);
            str_content[len] = '\0';
            
            advance(lexer);
            token.type = TYPE_STRING;
            token.lexeme = str_content;
            return token;
        } else if (current_char == '(') {
            advance(lexer);
            token.type = LPAREN;
            token.lexeme = strdup("(");
            return token;
        } else if (current_char == ')') {
            advance(lexer);
            token.type = RPAREN;
            token.lexeme = strdup(")");
            return token;
        } else if (current_char == ',') {
            advance(lexer);
            token.type = COMMA;
            token.lexeme = strdup(",");
            return token;
        }
        advance(lexer);
    }
}