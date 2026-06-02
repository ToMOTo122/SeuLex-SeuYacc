#include "symtab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

SYM_TAB* symtab_open(SYM_TAB* parent) {
    SYM_TAB* tab = (SYM_TAB*)malloc(sizeof(SYM_TAB));
    tab->parent = parent;
    tab->function = parent ? parent->function : NULL;
    tab->symbols = NULL;
    return tab;
}

SYM_INFO* symtab_insert(SYM_TAB* tab, char* name, T_INFO* type) {
    SYM_INFO* info = symtab_info_new(name, type);
    SYM_LIST* node = (SYM_LIST*)malloc(sizeof(SYM_LIST));
    node->info = info;
    node->next = tab->symbols;
    tab->symbols = node;
    return info;
}

SYM_INFO* symtab_lookup(SYM_TAB* tab, const char* name) {
    for (SYM_TAB* t = tab; t != NULL; t = t->parent) {
        for (SYM_LIST* s = t->symbols; s != NULL; s = s->next) {
            if (s->info->name && strcmp(s->info->name, name) == 0) {
                return s->info;
            }
        }
    }
    return NULL;
}

void symtab_print(SYM_TAB* tab, int depth) {
    for (SYM_TAB* t = tab; t != NULL; t = t->parent) {
        printf("Scope level (parent=%p):\n", (void*)t->parent);
        for (SYM_LIST* s = t->symbols; s != NULL; s = s->next) {
            printf("  %s : ", s->info->name ? s->info->name : "(anon)");
            types_print(s->info->type);
            printf("\n");
        }
        break; /* just current scope */
    }
}
