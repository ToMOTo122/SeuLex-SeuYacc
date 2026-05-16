/*	minic.y(1.9)	17:46:21	97/12/10
*
*	Parser demo of simple symbol table management and type checking.
*/
#include	<stdio.h>	/* for (f)printf() */
#include	<stdlib.h>	/* for exit() */

#include	"symtab.h"
#include	"types.h"
#include	"check.h"

int		lineno	= 1;	/* number of current source line */
extern int	yylex();	/* lexical analyzer generated from lex.l */
extern char	*yytext;	/* last token, defined in lex.l  */
SYM_TAB 	*scope;		/* current symbol table, initialized in lex.l */
char		*base;		/* basename of command line argument */

void
yyerror(char *s)
{
fprintf(stderr,"Syntax error on line #%d: %s\n",lineno,s);
fprintf(stderr,"Last token was \"%s\"\n",yytext);
exit(1);
}


/* Token definitions */
#define INT 257
#define FLOAT 258
#define NAME 259
#define STRUCT 260
#define IF 261
#define ELSE 262
#define RETURN 263
#define NUMBER 264
#define LPAR 265
#define RPAR 266
#define LBRACE 267
#define RBRACE 268
#define LBRACK 269
#define RBRACK 270
#define ASSIGN 271
#define SEMICOLON 272
#define COMMA 273
#define DOT 274
#define PLUS 275
#define MINUS 276
#define TIMES 277
#define DIVIDE 278
#define EQUAL 279
#define LOW 280
#define UMINUS 281

/* Parser tables */
#define YY_NSTATES 21
#define YY_NPRODS  47

extern int yylex(void);
extern char* yytext;
extern int yylineno;

int yyparse(void);
void yyerror(const char* s);

typedef union {

	char*		name;
	int		value;
	T_LIST*		tlist;
	T_INFO*		type;
	SYM_INFO*	sym;
	SYM_LIST*	slist;
	
} YYSTYPE;

YYSTYPE yylval;

static int yy_prod_lhs[47] = {26, 27, 27, 28, 28, 29, 32, 32, 33, 33, 34, 35, 36, 30, 31, 31, 31, 31, 37, 37, 38, 39, 39, 40, 40, 40, 40, 40, 42, 42, 42, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 44, 44, 43};

static int yy_prod_len[47] = {1, 2, 0, 1, 1, 2, 1, 0, 3, 1, 2, 1, 2, 3, 1, 1, 2, 4, 2, 0, 3, 3, 0, 5, 7, 3, 2, 1, 1, 4, 3, 3, 4, 3, 3, 3, 3, 3, 3, 2, 1, 1, 3, 4, 1, 3, 1};

/* ACTION table */
#define YY_SHIFT  1
#define YY_REDUCE 2
#define YY_ACCEPT 3
#define YY_ERROR  0

typedef struct { int type; int value; } YYAction;

static YYAction yy_action[21][26] = {
    {{2,2},{1,1},{1,2},{0,0},{1,3},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{2,14},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,14},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{2,15},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,15},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,9},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{3,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{2,2},{1,1},{1,2},{0,0},{1,3},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{2,3},{2,3},{0,0},{2,3},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{2,4},{2,4},{0,0},{2,4},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{1,11},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,12},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{1,1},{1,2},{0,0},{1,3},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,19},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{2,1},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{2,5},{2,5},{0,0},{2,5},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{2,16},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,16},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{1,17},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,12},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,18},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{1,1},{1,2},{0,0},{1,3},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,19},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{2,13},{2,13},{0,0},{2,13},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{1,20},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{2,17},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,17},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{2,18},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}},
    {{0,0},{2,20},{2,20},{0,0},{2,20},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0}}
};

static int yy_goto[21][45] = {
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,4,5,6,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,10,5,6,7,8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,13,-1,-1,-1,-1,-1,14,15,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,13,-1,-1,-1,-1,-1,19,15,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
};

/* Semantic value stacks */
static YYSTYPE yy_val_stack[1024];
static int yy_val_top = 0;

static void yy_push_val(YYSTYPE v) { yy_val_stack[yy_val_top++] = v; }
static YYSTYPE yy_pop_val(void) { return yy_val_stack[--yy_val_top]; }

/* State stack */
static int yy_state_stack[1024];
static int yy_state_top = 0;

static void yy_execute_action(int prod_id) {
    int len = yy_prod_len[prod_id];
    YYSTYPE yy_result;
    YYSTYPE yy_v[32]; /* RHS values */

    /* Pop RHS values from stack (in reverse order) */
    for (int i = len - 1; i >= 0; i--) {
        yy_v[i] = yy_pop_val();
    }

    switch (prod_id) {
        case 0: /* no action */
        break;
        case 1: /* no action */
        break;
        case 2: /* no action */
        break;
        case 3: /* no action */
        break;
        case 4: /* no action */
        break;
        case 5: {
            	/* this is yy_v[2] */ yy_result.sym = symtab_insert(scope,yy_v[1].name,0); scope = symtab_open(scope); /* open new scope */ scope->function = yy_result.sym; /* attach to this function */ 
        }
        break;
        case 6: {
             yy_result.tlist = yy_v[0].tlist; 
        }
        break;
        case 7: {
             yy_result.tlist = 0; 
        }
        break;
        case 8: {
             yy_result.tlist = types_list_insert(yy_v[2].tlist,yy_v[0].type); 
        }
        break;
        case 9: {
             yy_result.tlist = types_list_insert(0,yy_v[0].type); 
        }
        break;
        case 10: {
             symtab_insert(scope,yy_v[1].name,yy_v[0].type); /* insert in symbol table */ yy_result.type = yy_v[0].type; /* remember type info */ 
        }
        break;
        case 11: {
              scope = symtab_open(scope); 
        }
        break;
        case 12: /* no action */
        break;
        case 13: {
             symtab_insert(scope,yy_v[1].name,yy_v[0].type); 
        }
        break;
        case 14: {
             yy_result.type = types_simple(int_t); 
        }
        break;
        case 15: {
             yy_result.type = types_simple(float_t); 
        }
        break;
        case 16: {
             yy_result.type = types_array(yy_v[0].type); 
        }
        break;
        case 17: {
             yy_result.type = types_record(yy_v[2].slist); 
        }
        break;
        case 18: {
             yy_result.slist = symtab_list_insert(yy_v[1].slist,yy_v[0].sym); 
        }
        break;
        case 19: {
             yy_result.slist = 0; 
        }
        break;
        case 20: {
             yy_result.sym = symtab_info_new(yy_v[1].name,yy_v[0].type); 
        }
        break;
        case 21: /* no action */
        break;
        case 22: /* no action */
        break;
        case 23: /* no action */
        break;
        case 24: /* no action */
        break;
        case 25: {
             check_assignment(yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 26: {
             check_assignment(scope->function->type->info.fun.target,yy_v[1].type); 
        }
        break;
        case 27: /* no action */
        break;
        case 28: {
             yy_result.type = yy_v[0].sym->type; 
        }
        break;
        case 29: {
             yy_result.type = check_array_access(yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 30: {
             yy_result.type = check_record_access(yy_v[0].type,yy_v[2].name); 
        }
        break;
        case 31: {
             yy_result.type = check_record_access(yy_v[0].type,yy_v[2].name); 
        }
        break;
        case 32: {
             yy_result.type = check_array_access(yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 33: {
             yy_result.type = check_arith_op(PLUS,yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 34: {
             yy_result.type = check_arith_op(MINUS,yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 35: {
             yy_result.type = check_arith_op(TIMES,yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 36: {
             yy_result.type = check_arith_op(DIVIDE,yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 37: {
             yy_result.type = check_relop(EQUAL,yy_v[0].type,yy_v[2].type); 
        }
        break;
        case 38: {
             yy_result.type = yy_v[1].type; 
        }
        break;
        case 39: {
             yy_result.type = check_arith_op(UMINUS,yy_v[1].type,0); 
        }
        break;
        case 40: {
             yy_result.type = yy_v[0].sym->type; 
        }
        break;
        case 41: {
             yy_result.type = types_simple(int_t); 
        }
        break;
        case 42: {
             yy_result.type = check_fun_call(scope,yy_v[0].name,0); 
        }
        break;
        case 43: {
             yy_result.type = check_fun_call(scope,yy_v[0].name,&yy_v[2].tlist); 
        }
        break;
        case 44: {
             yy_result.tlist = types_list_insert(0,yy_v[0].type); 
        }
        break;
        case 45: {
             yy_result.tlist = types_list_insert(yy_v[2].tlist,yy_v[0].type); 
        }
        break;
        case 46: {
             yy_result.sym = check_symbol(scope,yy_v[0].name); 
        }
        break;
        default: break;
    }

    /* Push result */
    yy_push_val(yy_result);
}

int yyparse(void) {
    yy_state_top = 0;
    yy_state_stack[yy_state_top++] = 0;
    yy_val_top = 0;

    int token = yylex();

    while (1) {
        int s = yy_state_stack[yy_state_top - 1];
        int t = token;

        if (t >= 26) t = 0;

        YYAction act = yy_action[s][t];

        if (act.type == YY_SHIFT) {
            yy_state_stack[yy_state_top++] = act.value;
            yy_push_val(yylval);
            token = yylex();
        } else if (act.type == YY_REDUCE) {
            int prod_id = act.value;
            int lhs = yy_prod_lhs[prod_id];

            yy_execute_action(prod_id);

            /* Pop |rhs| states */
            yy_state_top -= yy_prod_len[prod_id];
            int new_s = yy_state_stack[yy_state_top - 1];
            int next = yy_goto[new_s][lhs];
            if (next < 0) {
                yyerror("syntax error in goto");
                return 1;
            }
            yy_state_stack[yy_state_top++] = next;
        } else if (act.type == YY_ACCEPT) {
            return 0;
        } else {
            yyerror("syntax error");
            token = yylex();
            if (token == 0) return 1;
        }
    }
}


int
main(int argc,char *argv[])
{
if (argc!=2) {
	fprintf(stderr,"Usage: %s base_file_name",argv[0]);
	exit(1);
	}
base = argv[1];
return yyparse();
}
