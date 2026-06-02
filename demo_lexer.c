/* Demo: Lexer driver that prints token stream */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minic.tab.h"

static const char* token_name(int tok) {
    static char buf[64];
    switch (tok) {
        case INT:    return "INT";
        case FLOAT:  return "FLOAT";
        case NAME:   return "NAME";
        case STRUCT: return "STRUCT";
        case IF:     return "IF";
        case ELSE:   return "ELSE";
        case RETURN: return "RETURN";
        case NUMBER: return "NUMBER";
        case LPAR:   return "LPAR";
        case RPAR:   return "RPAR";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case LBRACK: return "LBRACK";
        case RBRACK: return "RBRACK";
        case ASSIGN: return "ASSIGN";
        case EQUAL:  return "EQUAL";
        case SEMICOLON: return "SEMICOLON";
        case COMMA:  return "COMMA";
        case DOT:    return "DOT";
        case PLUS:   return "PLUS";
        case MINUS:  return "MINUS";
        case TIMES:  return "TIMES";
        case DIVIDE: return "DIVIDE";
        case 0:      return "EOF";
        default:
            if (tok >= 32 && tok < 127)
                snprintf(buf, sizeof(buf), "'%c'", tok);
            else
                snprintf(buf, sizeof(buf), "token(%d)", tok);
            return buf;
    }
}

/* lineno is defined in yylex.c (the lexer tracks line numbers) */
extern int lineno;
YYSTYPE yylval;
void* scope = NULL;

extern int yylex(void);
extern char* yytext;
extern int yylineno;
extern FILE* yyin;   /* defined in yylex.c */

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: demo_lexer <source.minic>\n");
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (!yyin) {
        perror(argv[1]);
        return 1;
    }

    printf("=== SeuLex Demo: Token Stream ===\n");
    printf("Input: %s\n\n", argv[1]);
    printf("%-6s %-12s %s\n", "Line", "Token", "Lexeme");
    printf("%-6s %-12s %s\n", "------", "------------", "------");

    int token;
    while ((token = yylex()) != 0) {
        printf("%-6d %-12s '%s'\n", lineno, token_name(token),
               yytext ? yytext : "");
    }
    printf("%-6d %-12s\n", lineno, "EOF");

    printf("\n=== Tokenization Complete ===\n");
    fclose(yyin);
    return 0;
}