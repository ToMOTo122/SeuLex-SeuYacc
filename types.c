#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

T_INFO* types_simple(TypeKind k) {
    T_INFO* t = (T_INFO*)malloc(sizeof(T_INFO));
    t->kind = k;
    t->name = NULL;
    return t;
}

T_INFO* types_array(T_INFO* elem) {
    T_INFO* t = (T_INFO*)malloc(sizeof(T_INFO));
    t->kind = array_t;
    t->info.array.target = elem;
    t->name = NULL;
    return t;
}

T_INFO* types_record(SYM_LIST* fields) {
    T_INFO* t = (T_INFO*)malloc(sizeof(T_INFO));
    t->kind = record_t;
    t->info.fields = fields;
    t->name = NULL;
    return t;
}

T_INFO* types_fun(T_INFO* ret, T_LIST* params) {
    T_INFO* t = (T_INFO*)malloc(sizeof(T_INFO));
    t->kind = func_t;
    t->info.fun.target = ret;
    t->name = NULL;
    return t;
}

T_LIST* types_list_insert(T_LIST* list, T_INFO* type) {
    T_LIST* node = (T_LIST*)malloc(sizeof(T_LIST));
    node->type = type;
    node->next = list;
    return node;
}

int types_equal(T_INFO* a, T_INFO* b) {
    if (!a || !b) return 0;
    if (a->kind != b->kind) return 0;
    switch (a->kind) {
        case int_t:
        case float_t:
            return 1;
        case array_t:
            return types_equal(a->info.array.target, b->info.array.target);
        case record_t:
            return 1; /* simplified */
        case func_t:
            return types_equal(a->info.fun.target, b->info.fun.target);
    }
    return 0;
}

void types_print(T_INFO* t) {
    if (!t) { printf("???"); return; }
    switch (t->kind) {
        case int_t:    printf("int"); break;
        case float_t:  printf("float"); break;
        case array_t:  types_print(t->info.array.target); printf("[]"); break;
        case record_t: printf("struct{...}"); break;
        case func_t:   printf("func->"); types_print(t->info.fun.target); break;
    }
}

int types_size(T_INFO* t) {
    if (!t) return 0;
    switch (t->kind) {
        case int_t:   return 4;
        case float_t: return 4;
        case array_t: return 8; /* pointer */
        case record_t: return 8;
        case func_t:  return 8;
    }
    return 4;
}

SYM_INFO* symtab_info_new(char* name, T_INFO* type) {
    SYM_INFO* s = (SYM_INFO*)malloc(sizeof(SYM_INFO));
    s->name = name ? strdup(name) : NULL;
    s->type = type;
    return s;
}

SYM_LIST* symtab_list_insert(SYM_LIST* list, SYM_INFO* info) {
    SYM_LIST* node = (SYM_LIST*)malloc(sizeof(SYM_LIST));
    node->info = info;
    node->next = list;
    return node;
}
