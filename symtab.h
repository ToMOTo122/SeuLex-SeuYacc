#ifndef SYMTAB_H
#define SYMTAB_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SYM_TAB {
    struct SYM_TAB* parent;
    SYM_INFO* function;     /* enclosing function (for return type check) */
    /* Simple linear list for symbols in this scope */
    SYM_LIST* symbols;
} SYM_TAB;

SYM_TAB* symtab_open(SYM_TAB* parent);
SYM_INFO* symtab_insert(SYM_TAB* tab, char* name, T_INFO* type);
SYM_INFO* symtab_lookup(SYM_TAB* tab, const char* name);
void symtab_print(SYM_TAB* tab, int depth);

#ifdef __cplusplus
}
#endif

#endif
