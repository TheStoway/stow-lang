#ifndef STOW_H
#define STOW_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
    TOKEN_VAR, TOKEN_VAL, TOKEN_FUNC, TOKEN_IF, TOKEN_ELSE, TOKEN_WHILE,
    TOKEN_PRINT, TOKEN_INPUT, TOKEN_RETURN, TOKEN_BREAK, TOKEN_CONTINUE, TOKEN_IMPORT,
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_LBRACKET, TOKEN_RBRACKET,
    TOKEN_COLON, TOKEN_COMMA, TOKEN_EQUALS, TOKEN_SEMICOLON,
    TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
    TOKEN_EQ_EQ, TOKEN_BANG_EQ, TOKEN_LT, TOKEN_GT, TOKEN_AND, TOKEN_OR,
    TOKEN_EOF, TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
} Token;

typedef struct {
    const char* source;
    size_t pos;
    char cur;
    int line;
    Token peeked;
    bool has_peek;
} Lexer;

void lexer_init(Lexer* lexer, const char* source);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
void token_free(Token token);

typedef enum {
    TYPE_INT, TYPE_STR, TYPE_FLOAT, TYPE_BOOL, TYPE_VOID, TYPE_LIST, TYPE_UNKNOWN
} DataType;

typedef struct Symbol {
    char* name;
    DataType type;
    char* value;
    bool is_constant;
    struct Symbol* next;
} Symbol;

typedef enum {
    NODE_PRINT, NODE_INPUT, NODE_STRING, NODE_NUMBER, NODE_IDENTIFIER,
    NODE_VAR_DECL, NODE_BLOCK, NODE_FUNC_DECL, NODE_FUNC_CALL,
    NODE_IF, NODE_WHILE, NODE_BIN_OP, NODE_LIST, NODE_PARAM, NODE_ASSIGN,
    NODE_RETURN, NODE_BREAK, NODE_CONTINUE, NODE_IMPORT, NODE_INDEX
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char* value;
    char* var_name;
    DataType var_type;
    int line;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* condition;
    struct ASTNode* body;
    struct ASTNode* else_body;
    struct ASTNode* params; 
    struct ASTNode* next_param; 
    struct ASTNode* index;  
} ASTNode;

typedef struct Function {
    char* name;
    ASTNode* params;
    ASTNode* body;
    struct Function* next;
} Function;

extern bool should_return;
extern char* return_value;
extern bool should_break;
extern bool should_continue;

void report_error(const char* code, int line);
ASTNode* parse(Lexer* lexer);
void interpret(ASTNode* node);
void free_ast(ASTNode* node);
char* read_file(const char* filename);

#endif
