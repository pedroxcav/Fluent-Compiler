#include "lexer.h"
#include "token.h"

// Organized by length to ensure longest matches first
static TokenValue token_values[] = {
    // Operators with multiple words
    {"is greater than or equal to", OP_GTE, true},
    {"is less than or equal to", OP_LTE, true},
    {"to the power of", OP_POW, true},
    {"is greater than", OP_GT,true},
    {"is less than", OP_LT, true},
    {"differs from", OP_NEQ, true},
    {"divided by", OP_DIV, true},

    // Keywords and simple operators
    {"integer", TYPE_INTEGER, false},
    {"float", TYPE_FLOAT, false},
    {"string", TYPE_STRING, false},
    {"boolean", TYPE_BOOLEAN, false},
    {"receives", KW_RECEIVES, false},
    {"if", KW_IF, false},
    {"else", KW_ELSE, false},
    {"while", KW_WHILE, false},
    {"function", KW_FUNCTION, false},
    {"return", KW_RETURN, false},
    {"then", KW_THEN, false},
    {"end", KW_END, false},
    {"say", KW_SAY, false},
    {"true", KW_TRUE, false},
    {"false", KW_FALSE, false},
    {"plus", OP_PLUS, false},
    {"minus", OP_MINUS, false},
    {"times", OP_TIMES, false},
    {"equals", OP_EQ, false},

    // Symbols
    {"(", LPAREN, false},
    {")", RPAREN, false},
    {",", COMMA,false},

    {NULL, UNKNOWN, false}
};

static const char *patterns[REGEX_COUNT] = {
    // REGEX_FLOAT
    "^[0-9]+\\.[0-9]+",
    // REGEX_INTEGER
    "^[0-9]+",
    // REGEX_STRING
    "^\"[^\"]*\"",
    // REGEX_IDENTIFIER
    "^[a-zA-Z_][a-zA-Z0-9_]*",
    // REGEX_WHITESPACE
    "^[ \t\r\n]+",
    // REGEX_COMMENT
    "^#[^\n]*",
};

// Gets part of souce from start to current position
static char *extract_lexeme(Lexer *lexer, int start_position) {
    int length = lexer -> position - start_position;
    char *lexeme = malloc(length + 1);
    if (lexeme) {
        memcpy(lexeme, lexer -> source + start_position, length);
        lexeme[length] = '\0';
    }
    return lexeme;
}

// Matches the current position with all of the regexes
// If true, it returns the length of the match, so you know where to move the position forward
static bool match_regex(Lexer *lexer, int regex_index, int *length) {
    regmatch_t match;
    // The function regexec uses the regex, the source, the start position, the number of matches, the object with all of the results and some flags
    int status = regexec(
        &lexer -> regex[regex_index],
        lexer -> source + lexer -> position,
        1, &match, 0
    );
    // If matched, we get the whole source up to the end of the match and remove the beginning out of the match, so we have the exactly match part
    if (status == 0) {
        *length = match.rm_eo - match.rm_so;
        return true;
    }
    return false;
}

static void skip_whitespace(Lexer *lexer) {
    int length;
    bool found;
    do {
        found = false;
        // If the current character is the end of the source, we stop here
        if (lexer -> source[lexer -> position] == '\0') break;

        // When the match as a whitespace is true, we advance our position for the end of it
        // If we have an empty space than include many lines, we also update that number
        if (match_regex(lexer, REGEX_WHITESPACE, &length)) {
            for (int i = 0; i < length; i++)
                if (lexer -> source[lexer -> position + i] == '\n')
                    lexer -> line++;
            lexer -> position += length;
            found = true;
        // Doing the same thing for comments, like whitespaces
        } else if (match_regex(lexer, REGEX_COMMENT, &length)) {
            lexer -> position += length;
            found = true;
        }
    } while (found);
}

static bool complex_token(Lexer *lexer, Token *token) {
    // Runs thought all of the token values until the last one
    for (int i = 0; token_values[i].pattern != NULL; i++) {
        if (!token_values[i].complex) continue;
        
        int length = strlen(token_values[i].pattern);
        // If the current pattern doesn't match, try the next one
        if (strncmp(lexer -> source + lexer -> position, token_values[i].pattern, length) != 0)
            continue;
        
        // The pattern must be wrote correctly with spaces, otherwise it will probably be an identifier
        char next = lexer -> source[lexer->position + length];
        if (isalnum(next) || next == '_') continue;

        int start = lexer->position;
        lexer -> position += length;
        token -> type = token_values[i].type;
        token -> lexeme = extract_lexeme(lexer, start);
        return true;
    }
    return false;
}

static bool read_number(Lexer *lexer, Token *token) {
    int length;
    if (match_regex(lexer, REGEX_FLOAT, &length))
        token -> type = TYPE_FLOAT;
    else if (match_regex(lexer, REGEX_INTEGER, &length))
        token -> type = TYPE_INTEGER;
    else
        return false;

    int start = lexer -> position;
    lexer -> position += length;
    token -> lexeme = extract_lexeme(lexer, start);
    return true;
}

static bool read_string(Lexer *lexer, Token *token) {
    int length;
    if (!match_regex(lexer, REGEX_STRING, &length))
        return false;

    // Considers the size of the string without the quotes (length - 2)
    int content_length = length - 2;
    token -> lexeme = malloc(content_length + 1);
    if (token -> lexeme) {
        // Saves the value from the source inside the quotes into the token lexeme
        memcpy(token -> lexeme, lexer -> source + lexer -> position + 1, content_length);
        token -> lexeme[content_length] = '\0';
    }
    token -> type = TYPE_STRING;
    lexer -> position += length;
    return true;
}

static bool read_identifier(Lexer *lexer, Token *token) {
    int length;
    if (!match_regex(lexer, REGEX_IDENTIFIER, &length)) 
        return false;

    int start = lexer -> position;
    lexer -> position += length;
    token -> lexeme = extract_lexeme(lexer, start);

    token -> type = IDENTIFIER;
    for (int i = 0; token_values[i].pattern != NULL; i++) {
        if (!token_values[i].complex && strcmp(token -> lexeme, token_values[i].pattern) == 0) {
            token -> type = token_values[i].type;
            break;
        }
    }
    return true;
}

Token next_token(Lexer *lexer) {
    skip_whitespace(lexer);

    Token token;
    token.line = lexer->line;
    token.lexeme = NULL;

    if (lexer->source[lexer->position] == '\0') {
        token.type = EOF_TOKEN;
        return token;
    }
    if (complex_token(lexer, &token)) return token;
    if (read_number(lexer, &token)) return token;
    if (read_string(lexer, &token)) return token;
    if (read_identifier(lexer, &token)) return token;

    if (lexer->source[lexer->position] == ';') {
        lexer->position++;
        token.type = SEMICOLON;
        token.lexeme = NULL;
        return token;
    }

    int start_pos = lexer->position++;
    token.type = UNKNOWN;
    token.lexeme = extract_lexeme(lexer, start_pos);
    return token;
}

void init_lexer(Lexer *lexer, const char *source) {
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;

    for (int i = 0; i < REGEX_COUNT; i++) {
        if (regcomp(&lexer->regex[i], patterns[i], REG_EXTENDED) != 0) {
            fprintf(stderr, "Erro ao compilar regex: %s\n", patterns[i]);
            exit(1);
        }
    }
}

void del_lexer(Lexer *lexer) {
    for (int i = 0; i < REGEX_COUNT; i++)
        regfree(&lexer->regex[i]);
    free((char*) lexer->source);
}