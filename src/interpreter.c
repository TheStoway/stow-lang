#include "stow.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo '%s'\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
        buffer[length] = '\0';
    }
    fclose(file);
    return buffer;
}

Symbol* symbol_table = NULL;
Function* function_table = NULL;

bool should_return = false;
char* return_value = NULL;
bool should_break = false;
bool should_continue = false;

void set_variable(const char* name, DataType type, const char* value, bool is_const) {
    Symbol* current = symbol_table;
    while (current) {
        if (strcmp(current->name, name) == 0) {
            free(current->value);
            current->value = strdup(value);
            return;
        }
        current = current->next;
    }
    Symbol* new_sym = malloc(sizeof(Symbol));
    new_sym->name = strdup(name);
    new_sym->type = type;
    new_sym->value = strdup(value);
    new_sym->is_constant = is_const;
    new_sym->next = symbol_table;
    symbol_table = new_sym;
}

char* get_variable_value(const char* name) {
    Symbol* current = symbol_table;
    while (current) {
        if (strcmp(current->name, name) == 0) return current->value;
        current = current->next;
    }
    return NULL;
}

char* evaluate_node(ASTNode* node);

void interpret_node(ASTNode* node) {
    if (!node || should_return || should_break || should_continue) return;

    if (node->type == NODE_BLOCK) {
        interpret_node(node->body);
    } else if (node->type == NODE_PRINT) {
        char* val = evaluate_node(node->left);
        printf("%s\n", val); free(val);
    } else if (node->type == NODE_VAR_DECL) {
        char* val = evaluate_node(node->left);
        set_variable(node->var_name, node->var_type, val, strcmp(node->value, "val") == 0);
        free(val);
    } else if (node->type == NODE_FUNC_DECL) {
        Function* nf = malloc(sizeof(Function));
        nf->name = strdup(node->value);
        nf->params = node->params;
        nf->body = node->body;
        nf->next = function_table;
        function_table = nf;
    } else if (node->type == NODE_FUNC_CALL) {
        Function* f = function_table;
        while (f) {
            if (strcmp(f->name, node->value) == 0) {
                // Set parameters as global variables (simple version)
                ASTNode* p = f->params;
                ASTNode* arg = node->params;
                while (p && arg) {
                    char* val = evaluate_node(arg);
                    set_variable(p->value, p->var_type, val, false);
                    free(val);
                    p = p->next_param;
                    arg = arg->next_param;
                }
                interpret_node(f->body);
                should_return = false; // Reset for next call
                break;
            }
            f = f->next;
        }
    } else if (node->type == NODE_RETURN) {
        if (node->left) return_value = evaluate_node(node->left);
        else return_value = strdup("void");
        should_return = true;
    } else if (node->type == NODE_BREAK) {
        should_break = true;
    } else if (node->type == NODE_CONTINUE) {
        should_continue = true;
    } else if (node->type == NODE_IF) {
        char* cond = evaluate_node(node->condition);
        if (strcmp(cond, "true") == 0 || (atof(cond) != 0)) {
            interpret_node(node->body);
        } else if (node->else_body) {
            interpret_node(node->else_body);
        }
        free(cond);
    } else if (node->type == NODE_WHILE) {
        while (true) {
            char* cond = evaluate_node(node->condition);
            if (strcmp(cond, "true") == 0 || (atof(cond) != 0)) {
                free(cond);
                interpret_node(node->body);
                if (should_break) { should_break = false; break; }
                if (should_continue) { should_continue = false; continue; }
                if (should_return) break;
            } else {
                free(cond); break;
            }
        }
    } else if (node->type == NODE_ASSIGN) {
        char* val = evaluate_node(node->left);
        if (node->index) {
            // Indexing assignment: mi_lista[i] = val
            // Not implemented in this simple string-storage version
        } else {
            set_variable(node->value, TYPE_UNKNOWN, val, false);
        }
        free(val);
    } else if (node->type == NODE_IMPORT) {
        char* src = read_file(node->value);
        if (src) {
            Lexer l; lexer_init(&l, src);
            ASTNode* root = parse(&l);
            if (root) {
                interpret(root);
                free_ast(root);
            }
            free(src);
        } else {
            fprintf(stderr, "Error: No se pudo importar '%s'\n", node->value);
        }
    }

    if (node->right) interpret_node(node->right);
}

char* evaluate_node(ASTNode* node) {
    if (!node) return strdup("");
    if (node->type == NODE_STRING || node->type == NODE_NUMBER) return strdup(node->value);
    if (node->type == NODE_IDENTIFIER) {
        char* v = get_variable_value(node->value);
        if (!v) { report_error("E007", node->line); return strdup(""); }
        return strdup(v);
    }
    if (node->type == NODE_FUNC_CALL) {
        Function* f = function_table;
        while (f) {
            if (strcmp(f->name, node->value) == 0) {
                // Set parameters as global variables (simple version)
                ASTNode* p = f->params;
                ASTNode* arg = node->params;
                while (p && arg) {
                    char* val = evaluate_node(arg);
                    set_variable(p->value, p->var_type, val, false);
                    free(val);
                    p = p->next_param;
                    arg = arg->next_param;
                }
                interpret_node(f->body);
                char* res = return_value ? strdup(return_value) : strdup("void");
                if (return_value) { free(return_value); return_value = NULL; }
                should_return = false; // Reset for next call
                return res;
            }
            f = f->next;
        }
        return strdup("void");
    }
    if (node->type == NODE_INPUT) {
        char* prompt = evaluate_node(node->left);
        printf("%s", prompt); free(prompt);
        char buf[1024];
        if (fgets(buf, sizeof(buf), stdin)) {
            buf[strcspn(buf, "\n")] = 0;
            return strdup(buf);
        }
        return strdup("");
    }
    if (node->type == NODE_BIN_OP) {
        char* l = evaluate_node(node->left);
        char* r = evaluate_node(node->right);
        double lv = atof(l), rv = atof(r);
        char* res = NULL;
        if (strcmp(node->value, "+") == 0) {
            if (isdigit(l[0]) && isdigit(r[0])) {
                char b[64]; sprintf(b, "%g", lv + rv); res = strdup(b);
            } else {
                res = malloc(strlen(l) + strlen(r) + 1);
                strcpy(res, l); strcat(res, r);
            }
        } else if (strcmp(node->value, "-") == 0) { char b[64]; sprintf(b, "%g", lv - rv); res = strdup(b); }
        else if (strcmp(node->value, "*") == 0) { char b[64]; sprintf(b, "%g", lv * rv); res = strdup(b); }
        else if (strcmp(node->value, "/") == 0) { char b[64]; sprintf(b, "%g", rv != 0 ? lv / rv : 0); res = strdup(b); }
        else if (strcmp(node->value, "==") == 0) res = strdup(strcmp(l, r) == 0 ? "true" : "false");
        else if (strcmp(node->value, "!=") == 0) res = strdup(strcmp(l, r) != 0 ? "true" : "false");
        else if (strcmp(node->value, "<") == 0) res = strdup(lv < rv ? "true" : "false");
        else if (strcmp(node->value, ">") == 0) res = strdup(lv > rv ? "true" : "false");
        else if (strcmp(node->value, "&&") == 0) res = strdup((lv && rv) ? "true" : "false");
        else if (strcmp(node->value, "||") == 0) res = strdup((lv || rv) ? "true" : "false");
        free(l); free(r);
        return res ? res : strdup("");
    }
    if (node->type == NODE_LIST) return strdup("[Lista]");
    if (node->type == NODE_INDEX) {
        // Return dummy value for indexing
        return strdup("Item");
    }
    return strdup("");
}

void interpret(ASTNode* node) {
    interpret_node(node);
}
