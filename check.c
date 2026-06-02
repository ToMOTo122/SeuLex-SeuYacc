#include "check.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

T_INFO* check_assignment(T_INFO* lhs, T_INFO* rhs) {
    if (!lhs || !rhs) {
        fprintf(stderr, "Type error: null type in assignment\n");
        exit(1);
    }
    if (!types_equal(lhs, rhs)) {
        fprintf(stderr, "Type error: assignment type mismatch (");
        types_print(lhs);
        fprintf(stderr, " = ");
        types_print(rhs);
        fprintf(stderr, ")\n");
        exit(1);
    }
    return lhs;
}

T_INFO* check_array_access(T_INFO* arr, T_INFO* idx) {
    if (!arr || arr->kind != array_t) {
        fprintf(stderr, "Type error: array access on non-array type\n");
        exit(1);
    }
    if (!idx || idx->kind != int_t) {
        fprintf(stderr, "Type error: array index must be int\n");
        exit(1);
    }
    return arr->info.array.target;
}

T_INFO* check_record_access(T_INFO* rec, const char* field) {
    if (!rec || rec->kind != record_t) {
        fprintf(stderr, "Type error: field access on non-record type\n");
        exit(1);
    }
    for (SYM_LIST* f = rec->info.fields; f != NULL; f = f->next) {
        if (f->info->name && strcmp(f->info->name, field) == 0) {
            return f->info->type;
        }
    }
    fprintf(stderr, "Type error: record has no field '%s'\n", field);
    exit(1);
}

T_INFO* check_arith_op(int op, T_INFO* a, T_INFO* b) {
    /* Unary minus: b is NULL */
    if (b == NULL) {
        if (!a) { fprintf(stderr, "Type error in unary op\n"); exit(1); }
        return a;
    }
    if (!a || !b) {
        fprintf(stderr, "Type error: null type in arithmetic op\n");
        exit(1);
    }
    if (!types_equal(a, b)) {
        fprintf(stderr, "Type error: arithmetic op type mismatch (");
        types_print(a); fprintf(stderr, " vs "); types_print(b); fprintf(stderr, ")\n");
    }
    return a; /* result type same as operands */
}

T_INFO* check_relop(int op, T_INFO* a, T_INFO* b) {
    if (!a || !b) {
        fprintf(stderr, "Type error: null type in relational op\n");
        exit(1);
    }
    return types_simple(int_t); /* relational ops return int (bool) */
}

T_INFO* check_fun_call(SYM_TAB* scope, const char* name, T_LIST** args) {
    SYM_INFO* sym = symtab_lookup(scope, name);
    if (!sym) {
        fprintf(stderr, "Error: undefined function '%s'\n", name);
        exit(1);
    }
    if (!sym->type || sym->type->kind != func_t) {
        fprintf(stderr, "Error: '%s' is not a function\n", name);
        exit(1);
    }
    T_LIST* param = sym->type->info.fun.params;
    T_LIST* arg   = args ? *args : NULL;
    int pos = 1;
    while (param && arg) {
        if (!types_equal(param->type, arg->type)) {
            fprintf(stderr, "Type error: argument %d of '%s' type mismatch\n", pos, name);
            exit(1);
        }
        param = param->next;
        arg   = arg->next;
        pos++;
    }
    if (param || arg) {
        fprintf(stderr, "Error: wrong number of arguments to '%s'\n", name);
        exit(1);
    }
    return sym->type->info.fun.target;
}

SYM_INFO* check_symbol(SYM_TAB* scope, const char* name) {
    SYM_INFO* sym = symtab_lookup(scope, name);
    if (!sym) {
        fprintf(stderr, "Error: undefined identifier '%s'\n", name);
        exit(1);
    }
    return sym;
}
