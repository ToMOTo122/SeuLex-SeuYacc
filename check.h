#ifndef CHECK_H
#define CHECK_H

#include "types.h"
#include "symtab.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Type checking for expressions and statements */
T_INFO* check_assignment(T_INFO* lhs, T_INFO* rhs);
T_INFO* check_array_access(T_INFO* arr, T_INFO* idx);
T_INFO* check_record_access(T_INFO* rec, const char* field);
T_INFO* check_arith_op(int op, T_INFO* a, T_INFO* b);
T_INFO* check_relop(int op, T_INFO* a, T_INFO* b);
T_INFO* check_fun_call(SYM_TAB* scope, const char* name, T_LIST** args);
SYM_INFO* check_symbol(SYM_TAB* scope, const char* name);

#ifdef __cplusplus
}
#endif

#endif
