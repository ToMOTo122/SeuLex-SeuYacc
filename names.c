#include "names.h"
#include <string.h>
#include <stdlib.h>

#define MAX_NAMES 1024
static char* name_pool[MAX_NAMES];
static int name_count = 0;

void names_init(void) {
    name_count = 0;
    memset(name_pool, 0, sizeof(name_pool));
}

const char* names_find_or_add(const char* s) {
    for (int i = 0; i < name_count; i++) {
        if (strcmp(name_pool[i], s) == 0) return name_pool[i];
    }
    if (name_count < MAX_NAMES) {
        name_pool[name_count] = strdup(s);
        return name_pool[name_count++];
    }
    return s;
}
