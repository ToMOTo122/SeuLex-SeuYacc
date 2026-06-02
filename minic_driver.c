/* MiniC compiler driver */
#include <stdio.h>
#include <stdlib.h>

extern int yylex(void);
extern int yyparse(void);
extern char* yytext;
extern int yylineno;
extern int lineno;
extern FILE* yyin;      /* defined in yylex.c */

extern void names_init(void);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: minic <source.minic>\n");
        return 1;
    }

    names_init();

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror(argv[1]);
        return 1;
    }

    printf("=== MiniC Compiler ===\n");
    printf("Source: %s\n\n", argv[1]);

    int result = yyparse();

    fclose(yyin);

    if (result == 0)
        printf("\n=== Compilation Successful ===\n");
    else
        printf("\n=== Compilation Failed ===\n");

    return result;
}