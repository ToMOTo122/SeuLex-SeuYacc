#ifndef TYPES_H
#define TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Type kinds */
typedef enum { int_t, float_t, array_t, record_t, func_t } TypeKind;

/* Forward declarations */
typedef struct T_INFO T_INFO;
typedef struct T_LIST T_LIST;
typedef struct SYM_INFO SYM_INFO;
typedef struct SYM_LIST SYM_LIST;

struct T_INFO {
    TypeKind kind;
    union {
        struct { T_INFO* target; } array;    /* array of target type */
        struct { T_INFO* target; T_LIST* params; } fun; /* function: return type + param types */
        SYM_LIST* fields;                     /* record fields */
    } info;
    char* name;  /* for debugging */
};

struct T_LIST {
    T_INFO* type;
    T_LIST* next;
};

/* Type constructors */
T_INFO* types_simple(TypeKind k);
T_INFO* types_array(T_INFO* elem);
T_INFO* types_record(SYM_LIST* fields);
T_INFO* types_fun(T_INFO* ret, T_LIST* params);
T_LIST* types_list_insert(T_LIST* list, T_INFO* type);
int types_equal(T_INFO* a, T_INFO* b);
void types_print(T_INFO* t);
int types_size(T_INFO* t);

/* Symbol info (a symbol table entry with type) */
struct SYM_INFO {
    char* name;
    T_INFO* type;
};

struct SYM_LIST {
    SYM_INFO* info;
    SYM_LIST* next;
};

SYM_INFO* symtab_info_new(char* name, T_INFO* type);
SYM_LIST* symtab_list_insert(SYM_LIST* list, SYM_INFO* info);

#ifdef __cplusplus
}
#endif

#endif
