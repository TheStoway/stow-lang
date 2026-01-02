#include "stow.h"

void report_error(const char* errorCode, int line) {
    FILE* f = fopen("errors.json", "r");
    if (!f) { printf("Error [%s] en linea %d\n", errorCode, line); return; }
    char buf[256];
    bool found = false;
    while (fgets(buf, sizeof(buf), f)) {
        if (strstr(buf, errorCode)) {
            while (fgets(buf, sizeof(buf), f)) {
                if (strstr(buf, "message")) {
                    char* start = strchr(buf, ':') + 3;
                    char* end = strrchr(buf, '"');
                    if (start && end) { *end = '\0'; printf("Error [%s] en linea %d: %s\n", errorCode, line, start); }
                    found = true; break;
                }
            }
        }
        if (found) break;
    }
    fclose(f);
}

ASTNode* create_node(NodeType type, char* value, int line) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->var_name = NULL;
    node->var_type = TYPE_UNKNOWN;
    node->line = line;
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    node->body = NULL;
    node->else_body = NULL;
    node->params = NULL;
    node->next_param = NULL;
    node->index = NULL;
    return node;
}

void free_ast(ASTNode* node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free_ast(node->condition);
    free_ast(node->body);
    free_ast(node->else_body);
    free_ast(node->params);
    free_ast(node->next_param);
    free_ast(node->index);
    if (node->value) free(node->value);
    if (node->var_name) free(node->var_name);
    free(node);
}

DataType string_to_type(const char* type_str) {
    if (!type_str) return TYPE_UNKNOWN;
    if (strcmp(type_str, "Int") == 0) return TYPE_INT;
    if (strcmp(type_str, "Str") == 0) return TYPE_STR;
    if (strcmp(type_str, "Float") == 0) return TYPE_FLOAT;
    if (strcmp(type_str, "Bool") == 0) return TYPE_BOOL;
    if (strcmp(type_str, "Void") == 0) return TYPE_VOID;
    if (strcmp(type_str, "List") == 0) return TYPE_LIST;
    return TYPE_UNKNOWN;
}

ASTNode* parse_expression(Lexer* lexer);

ASTNode* parse_atom(Lexer* lexer) {
    Token token = lexer_next_token(lexer);
    if (token.type == TOKEN_STRING) {
        ASTNode* n = create_node(NODE_STRING, token.value, token.line);
        token_free(token); return n;
    } else if (token.type == TOKEN_NUMBER) {
        ASTNode* n = create_node(NODE_NUMBER, token.value, token.line);
        token_free(token); return n;
    } else if (token.type == TOKEN_INPUT) {
        int l = token.line;
        lexer_next_token(lexer); // (
        ASTNode* prompt = parse_expression(lexer);
        lexer_next_token(lexer); // )
        ASTNode* n = create_node(NODE_INPUT, NULL, l);
        n->left = prompt;
        token_free(token); return n;
    } else if (token.type == TOKEN_IDENTIFIER) {
        char* name = strdup(token.value);
        int l = token.line;
        token_free(token);
        if (lexer_peek_token(lexer).type == TOKEN_LPAREN) {
            lexer_next_token(lexer); // (
            ASTNode* call = create_node(NODE_FUNC_CALL, name, l);
            if (lexer_peek_token(lexer).type != TOKEN_RPAREN) {
                ASTNode* last = NULL;
                while (1) {
                    ASTNode* arg = parse_expression(lexer);
                    if (!call->params) call->params = arg;
                    else last->next_param = arg;
                    last = arg;
                    if (lexer_peek_token(lexer).type == TOKEN_COMMA) {
                        lexer_next_token(lexer); // ,
                    } else break;
                }
            }
            lexer_next_token(lexer); // )
            free(name); return call;
        } else if (lexer_peek_token(lexer).type == TOKEN_LBRACKET) {
            lexer_next_token(lexer); // [
            ASTNode* idx = parse_expression(lexer);
            lexer_next_token(lexer); // ]
            ASTNode* node = create_node(NODE_INDEX, name, l);
            node->index = idx;
            free(name); return node;
        }
        ASTNode* n = create_node(NODE_IDENTIFIER, name, l);
        free(name); return n;
    } else if (token.type == TOKEN_LBRACKET) {
        int l = token.line;
        ASTNode* list = create_node(NODE_LIST, NULL, l);
        if (lexer_peek_token(lexer).type != TOKEN_RBRACKET) {
            ASTNode* last = NULL;
            while (1) {
                ASTNode* item = parse_expression(lexer);
                if (!list->params) list->params = item;
                else last->next_param = item;
                last = item;
                if (lexer_peek_token(lexer).type == TOKEN_COMMA) lexer_next_token(lexer);
                else break;
            }
        }
        lexer_next_token(lexer); // ]
        token_free(token); return list;
    }
    token_free(token); return NULL;
}

ASTNode* parse_expression(Lexer* lexer) {
    ASTNode* left = parse_atom(lexer);
    Token peek = lexer_peek_token(lexer);
    if (peek.type >= TOKEN_PLUS && peek.type <= TOKEN_OR) {
        lexer_next_token(lexer);
        char* op = NULL;
        switch(peek.type) {
            case TOKEN_PLUS: op = "+"; break;
            case TOKEN_MINUS: op = "-"; break;
            case TOKEN_STAR: op = "*"; break;
            case TOKEN_SLASH: op = "/"; break;
            case TOKEN_EQ_EQ: op = "=="; break;
            case TOKEN_BANG_EQ: op = "!="; break;
            case TOKEN_LT: op = "<"; break;
            case TOKEN_GT: op = ">"; break;
            case TOKEN_AND: op = "&&"; break;
            case TOKEN_OR: op = "||"; break;
            default: op = "?";
        }
        ASTNode* bin = create_node(NODE_BIN_OP, op, peek.line);
        bin->left = left;
        bin->right = parse_expression(lexer);
        return bin;
    }
    return left;
}

ASTNode* parse_statement(Lexer* lexer);

ASTNode* parse_block(Lexer* lexer) {
    Token lbrace = lexer_next_token(lexer); // {
    ASTNode* root = NULL; ASTNode* current = NULL;
    while (lexer_peek_token(lexer).type != TOKEN_RBRACE && lexer_peek_token(lexer).type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(lexer);
        if (!stmt) continue;
        if (!root) { root = stmt; current = root; }
        else { current->right = stmt; current = stmt; }
    }
    lexer_next_token(lexer); // }
    ASTNode* block = create_node(NODE_BLOCK, NULL, lbrace.line);
    block->body = root;
    return block;
}

ASTNode* parse_statement(Lexer* lexer) {
    Token peek = lexer_peek_token(lexer);
    if (peek.type == TOKEN_EOF) return NULL;

    if (peek.type == TOKEN_PRINT) {
        int l = peek.line;
        lexer_next_token(lexer); lexer_next_token(lexer); // print, (
        ASTNode* expr = parse_expression(lexer);
        lexer_next_token(lexer); lexer_next_token(lexer); // ), ;
        ASTNode* n = create_node(NODE_PRINT, NULL, l);
        n->left = expr; return n;
    }

    if (peek.type == TOKEN_VAR || peek.type == TOKEN_VAL) {
        bool is_const = (peek.type == TOKEN_VAL);
        int l = peek.line;
        lexer_next_token(lexer);
        Token id = lexer_next_token(lexer);
        lexer_next_token(lexer); // :
        Token type_tok = lexer_next_token(lexer);
        lexer_next_token(lexer); // =
        ASTNode* expr = parse_expression(lexer);
        lexer_next_token(lexer); // ;
        ASTNode* n = create_node(NODE_VAR_DECL, NULL, l);
        n->var_name = strdup(id.value);
        n->var_type = string_to_type(type_tok.value);
        n->value = is_const ? strdup("val") : strdup("var");
        n->left = expr;
        token_free(id); token_free(type_tok); return n;
    }

    if (peek.type == TOKEN_FUNC) {
        int l = peek.line;
        lexer_next_token(lexer);
        Token id = lexer_next_token(lexer);
        lexer_next_token(lexer); // (
        ASTNode* params = NULL;
        if (lexer_peek_token(lexer).type != TOKEN_RPAREN) {
            ASTNode* last = NULL;
            while(1) {
                Token p_id = lexer_next_token(lexer);
                lexer_next_token(lexer); // :
                Token p_type = lexer_next_token(lexer);
                ASTNode* p = create_node(NODE_PARAM, p_id.value, p_id.line);
                p->var_type = string_to_type(p_type.value);
                if (!params) params = p; else last->next_param = p;
                last = p;
                token_free(p_id); token_free(p_type);
                if (lexer_peek_token(lexer).type == TOKEN_COMMA) lexer_next_token(lexer); else break;
            }
        }
        lexer_next_token(lexer); // )
        lexer_next_token(lexer); // :
        Token ret_type = lexer_next_token(lexer);
        ASTNode* body = parse_block(lexer);
        ASTNode* n = create_node(NODE_FUNC_DECL, id.value, l);
        n->params = params;
        n->body = body;
        n->var_type = string_to_type(ret_type.value);
        token_free(id); token_free(ret_type); return n;
    }

    if (peek.type == TOKEN_RETURN) {
        int l = peek.line;
        lexer_next_token(lexer);
        ASTNode* expr = parse_expression(lexer);
        lexer_next_token(lexer); // ;
        ASTNode* n = create_node(NODE_RETURN, NULL, l);
        n->left = expr; return n;
    }

    if (peek.type == TOKEN_BREAK) {
        int l = peek.line;
        lexer_next_token(lexer); lexer_next_token(lexer); // break, ;
        return create_node(NODE_BREAK, NULL, l);
    }
    if (peek.type == TOKEN_CONTINUE) {
        int l = peek.line;
        lexer_next_token(lexer); lexer_next_token(lexer); // continue, ;
        return create_node(NODE_CONTINUE, NULL, l);
    }

    if (peek.type == TOKEN_IF) {
        int l = peek.line;
        lexer_next_token(lexer); lexer_next_token(lexer); // if, (
        ASTNode* cond = parse_expression(lexer);
        lexer_next_token(lexer); // )
        ASTNode* body = parse_block(lexer);
        ASTNode* n = create_node(NODE_IF, NULL, l);
        n->condition = cond; n->body = body;
        if (lexer_peek_token(lexer).type == TOKEN_ELSE) {
            lexer_next_token(lexer);
            if (lexer_peek_token(lexer).type == TOKEN_IF) n->else_body = parse_statement(lexer);
            else n->else_body = parse_block(lexer);
        }
        return n;
    }

    if (peek.type == TOKEN_WHILE) {
        int l = peek.line;
        lexer_next_token(lexer); lexer_next_token(lexer); // while, (
        ASTNode* cond = parse_expression(lexer);
        lexer_next_token(lexer); // )
        ASTNode* body = parse_block(lexer);
        ASTNode* n = create_node(NODE_WHILE, NULL, l);
        n->condition = cond; n->body = body;
        return n;
    }

    if (peek.type == TOKEN_IMPORT) {
        int l = peek.line;
        lexer_next_token(lexer);
        Token file = lexer_next_token(lexer);
        lexer_next_token(lexer); // ;
        ASTNode* n = create_node(NODE_IMPORT, file.value, l);
        token_free(file); return n;
    }

    if (peek.type == TOKEN_IDENTIFIER) {
        Token id = lexer_next_token(lexer);
        if (lexer_peek_token(lexer).type == TOKEN_EQUALS) {
            lexer_next_token(lexer); // =
            ASTNode* expr = parse_expression(lexer);
            lexer_next_token(lexer); // ;
            ASTNode* n = create_node(NODE_ASSIGN, id.value, id.line);
            n->left = expr; token_free(id); return n;
        } else if (lexer_peek_token(lexer).type == TOKEN_LBRACKET) {
            lexer_next_token(lexer); // [
            ASTNode* idx = parse_expression(lexer);
            lexer_next_token(lexer); // ]
            lexer_next_token(lexer); // =
            ASTNode* expr = parse_expression(lexer);
            lexer_next_token(lexer); // ;
            ASTNode* n = create_node(NODE_ASSIGN, id.value, id.line);
            n->index = idx; n->left = expr;
            token_free(id); return n;
        } else if (lexer_peek_token(lexer).type == TOKEN_LPAREN) {
            // Standalone function call
            lexer_next_token(lexer); // (
            ASTNode* call = create_node(NODE_FUNC_CALL, id.value, id.line);
            if (lexer_peek_token(lexer).type != TOKEN_RPAREN) {
                ASTNode* last = NULL;
                while (1) {
                    ASTNode* arg = parse_expression(lexer);
                    if (!call->params) call->params = arg;
                    else last->next_param = arg;
                    last = arg;
                    if (lexer_peek_token(lexer).type == TOKEN_COMMA) {
                        lexer_next_token(lexer); // ,
                    } else break;
                }
            }
            lexer_next_token(lexer); // )
            lexer_next_token(lexer); // ;
            token_free(id);
            return call;
        } else {
            // Unknown statement, skip it
            token_free(id);
            return NULL;
        }
    }

    lexer_next_token(lexer); return NULL;
}

ASTNode* parse(Lexer* lexer) {
    ASTNode* root = NULL; ASTNode* current = NULL;
    while (lexer_peek_token(lexer).type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(lexer);
        if (!stmt) continue;
        if (!root) { root = stmt; current = root; }
        else { current->right = stmt; current = stmt; }
    }
    return root;
}
