/* Demo: Parser driver with production trace */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

extern SYM_TAB* scope;
extern int yyparse(void);
extern int yylineno;
extern int lineno;
extern FILE* yyin;      /* defined in yylex.c */

extern void names_init(void);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: demo_parser <source.minic>\n");
        return 1;
    }

    names_init();
    scope = symtab_open(NULL);  /* initialize global scope */

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror(argv[1]);
        return 1;
    }

    printf("=== SeuYacc Demo: Parse Trace ===\n");
    printf("Input: %s\n\n", argv[1]);

    int result = yyparse();

    fclose(yyin);

    if (result == 0)
        printf("\n=== Parse Successful ===\n");
    else
        printf("\n=== Parse Failed ===\n");

    return result;
}