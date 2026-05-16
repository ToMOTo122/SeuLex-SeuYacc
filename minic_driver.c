/* MiniC compiler driver */
#include <stdio.h>
#include <stdlib.h>

extern int yylex(void);
extern int yyparse(void);
extern char* yytext;
extern int yylineno;

extern void lex_init(void);
extern void names_init(void);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: minic <source.minic>\n");
        return 1;
    }

    names_init();

    if (!freopen(argv[1], "r", stdin)) {
        perror(argv[1]);
        return 1;
    }

    printf("=== MiniC Compiler ===\n");
    printf("Source: %s\n\n", argv[1]);

    int result = yyparse();

    if (result == 0)
        printf("\n=== Compilation Successful ===\n");
    else
        printf("\n=== Compilation Failed ===\n");

    return result;
}
