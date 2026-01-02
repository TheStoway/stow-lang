#include "stow.h"

extern char* read_file(const char* filename);

void run_source(const char* source) {
    if (!source || strlen(source) == 0) return;
    
    Lexer lexer;
    lexer_init(&lexer, source);

    ASTNode* root = parse(&lexer);
    if (root) {
        interpret(root);
        free_ast(root);
    }
}

void run_repl() {
    char line[1024];
    printf("Stow Programming Language [Version 1.0]\n");
    printf("Escribe 'exit' para salir o 'clear' para limpiar pantalla.\n");

    while (1) {
        printf("stow> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;

        if (strcmp(line, "exit") == 0) break;
        if (strcmp(line, "clear") == 0) {
            #ifdef _WIN32
                system("cls");
            #else
                system("clear");
            #endif
            continue;
        }
        
        run_source(line);
    }
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        // Ejecutar archivo
        char* source = read_file(argv[1]);
        if (source) {
            run_source(source);
            free(source);
        } else {
            return 1;
        }
    } else {
        // Iniciar REPL
        run_repl();
    }

    return 0;
}
