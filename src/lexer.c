#include "stow.h"

void lexer_advance(Lexer* lexer) {
    if (lexer->cur == '\n') lexer->line++;
    lexer->pos++;
    if (lexer->pos < strlen(lexer->source)) {
        lexer->cur = lexer->source[lexer->pos];
    } else {
        lexer->cur = '\0';
    }
}

void lexer_init(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->cur = source[0];
    lexer->line = 1;
    lexer->has_peek = false;
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->cur != '\0' && isspace(lexer->cur)) {
        lexer_advance(lexer);
    }
}

void lexer_skip_comment(Lexer* lexer) {
    if (lexer->cur == '/' && lexer->source[lexer->pos + 1] == '/') {
        while (lexer->cur != '\0' && lexer->cur != '\n') {
            lexer_advance(lexer);
        }
    } else if (lexer->cur == '/' && lexer->source[lexer->pos + 1] == '*') {
        lexer_advance(lexer); lexer_advance(lexer); // skip /*
        while (lexer->cur != '\0') {
            if (lexer->cur == '*' && lexer->source[lexer->pos + 1] == '/') {
                lexer_advance(lexer); lexer_advance(lexer); // skip */
                break;
            }
            lexer_advance(lexer);
        }
    }
}

Token lexer_collect_string(Lexer* lexer) {
    int start_line = lexer->line;
    lexer_advance(lexer); // skip "
    size_t start = lexer->pos;
    while (lexer->cur != '\0' && lexer->cur != '"') {
        lexer_advance(lexer);
    }
    size_t len = lexer->pos - start;
    char* value = malloc(len + 1);
    strncpy(value, &lexer->source[start], len);
    value[len] = '\0';
    lexer_advance(lexer); // skip "
    return (Token){TOKEN_STRING, value, start_line};
}

Token lexer_collect_id_or_keyword(Lexer* lexer) {
    int start_line = lexer->line;
    size_t start = lexer->pos;
    while (lexer->cur != '\0' && (isalnum(lexer->cur) || lexer->cur == '_')) {
        lexer_advance(lexer);
    }
    size_t len = lexer->pos - start;
    char* value = malloc(len + 1);
    strncpy(value, &lexer->source[start], len);
    value[len] = '\0';

    if (strcmp(value, "print") == 0) { free(value); return (Token){TOKEN_PRINT, NULL, start_line}; }
    if (strcmp(value, "input") == 0) { free(value); return (Token){TOKEN_INPUT, NULL, start_line}; }
    if (strcmp(value, "var") == 0) { free(value); return (Token){TOKEN_VAR, NULL, start_line}; }
    if (strcmp(value, "val") == 0) { free(value); return (Token){TOKEN_VAL, NULL, start_line}; }
    if (strcmp(value, "func") == 0) { free(value); return (Token){TOKEN_FUNC, NULL, start_line}; }
    if (strcmp(value, "if") == 0) { free(value); return (Token){TOKEN_IF, NULL, start_line}; }
    if (strcmp(value, "else") == 0) { free(value); return (Token){TOKEN_ELSE, NULL, start_line}; }
    if (strcmp(value, "while") == 0) { free(value); return (Token){TOKEN_WHILE, NULL, start_line}; }
    if (strcmp(value, "return") == 0) { free(value); return (Token){TOKEN_RETURN, NULL, start_line}; }
    if (strcmp(value, "break") == 0) { free(value); return (Token){TOKEN_BREAK, NULL, start_line}; }
    if (strcmp(value, "continue") == 0) { free(value); return (Token){TOKEN_CONTINUE, NULL, start_line}; }
    if (strcmp(value, "import") == 0) { free(value); return (Token){TOKEN_IMPORT, NULL, start_line}; }
    
    return (Token){TOKEN_IDENTIFIER, value, start_line};
}

Token lexer_collect_number(Lexer* lexer) {
    int start_line = lexer->line;
    size_t start = lexer->pos;
    while (lexer->cur != '\0' && (isdigit(lexer->cur) || lexer->cur == '.')) {
        lexer_advance(lexer);
    }
    size_t len = lexer->pos - start;
    char* value = malloc(len + 1);
    strncpy(value, &lexer->source[start], len);
    value[len] = '\0';
    return (Token){TOKEN_NUMBER, value, start_line};
}

Token lexer_get_raw_token(Lexer* lexer) {
    while (lexer->cur != '\0') {
        lexer_skip_whitespace(lexer);
        if (lexer->cur == '/' && (lexer->source[lexer->pos + 1] == '/' || lexer->source[lexer->pos + 1] == '*')) {
            lexer_skip_comment(lexer);
            continue;
        }
        if (lexer->cur == '\0') break;
        int current_line = lexer->line;
        if (lexer->cur == '"') return lexer_collect_string(lexer);
        if (isdigit(lexer->cur)) return lexer_collect_number(lexer);
        if (isalpha(lexer->cur)) return lexer_collect_id_or_keyword(lexer);
        
        if (lexer->cur == '(') { lexer_advance(lexer); return (Token){TOKEN_LPAREN, NULL, current_line}; }
        if (lexer->cur == ')') { lexer_advance(lexer); return (Token){TOKEN_RPAREN, NULL, current_line}; }
        if (lexer->cur == '{') { lexer_advance(lexer); return (Token){TOKEN_LBRACE, NULL, current_line}; }
        if (lexer->cur == '}') { lexer_advance(lexer); return (Token){TOKEN_RBRACE, NULL, current_line}; }
        if (lexer->cur == '[') { lexer_advance(lexer); return (Token){TOKEN_LBRACKET, NULL, current_line}; }
        if (lexer->cur == ']') { lexer_advance(lexer); return (Token){TOKEN_RBRACKET, NULL, current_line}; }
        if (lexer->cur == ':') { lexer_advance(lexer); return (Token){TOKEN_COLON, NULL, current_line}; }
        if (lexer->cur == ',') { lexer_advance(lexer); return (Token){TOKEN_COMMA, NULL, current_line}; }
        if (lexer->cur == ';') { lexer_advance(lexer); return (Token){TOKEN_SEMICOLON, NULL, current_line}; }
        if (lexer->cur == '+') { lexer_advance(lexer); return (Token){TOKEN_PLUS, NULL, current_line}; }
        if (lexer->cur == '-') { lexer_advance(lexer); return (Token){TOKEN_MINUS, NULL, current_line}; }
        if (lexer->cur == '*') { lexer_advance(lexer); return (Token){TOKEN_STAR, NULL, current_line}; }
        if (lexer->cur == '/') { lexer_advance(lexer); return (Token){TOKEN_SLASH, NULL, current_line}; }

        if (lexer->cur == '=') {
            lexer_advance(lexer);
            if (lexer->cur == '=') { lexer_advance(lexer); return (Token){TOKEN_EQ_EQ, NULL, current_line}; }
            return (Token){TOKEN_EQUALS, NULL, current_line};
        }
        if (lexer->cur == '!') {
            lexer_advance(lexer);
            if (lexer->cur == '=') { lexer_advance(lexer); return (Token){TOKEN_BANG_EQ, NULL, current_line}; }
            return (Token){TOKEN_UNKNOWN, NULL, current_line};
        }
        if (lexer->cur == '<') { lexer_advance(lexer); return (Token){TOKEN_LT, NULL, current_line}; }
        if (lexer->cur == '>') { lexer_advance(lexer); return (Token){TOKEN_GT, NULL, current_line}; }
        if (lexer->cur == '&' && lexer->source[lexer->pos + 1] == '&') {
            lexer_advance(lexer); lexer_advance(lexer); return (Token){TOKEN_AND, NULL, current_line};
        }
        if (lexer->cur == '|' && lexer->source[lexer->pos + 1] == '|') {
            lexer_advance(lexer); lexer_advance(lexer); return (Token){TOKEN_OR, NULL, current_line};
        }

        lexer_advance(lexer);
        return (Token){TOKEN_UNKNOWN, NULL, current_line};
    }
    return (Token){TOKEN_EOF, NULL, lexer->line};
}

Token lexer_next_token(Lexer* lexer) {
    if (lexer->has_peek) { lexer->has_peek = false; return lexer->peeked; }
    return lexer_get_raw_token(lexer);
}

Token lexer_peek_token(Lexer* lexer) {
    if (!lexer->has_peek) { lexer->peeked = lexer_get_raw_token(lexer); lexer->has_peek = true; }
    return lexer->peeked;
}

void token_free(Token token) {
    if (token.value) free(token.value);
}
