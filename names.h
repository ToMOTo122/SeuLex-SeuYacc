#ifndef NAMES_H
#define NAMES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Simple string pool for identifier names */
const char* names_find_or_add(const char* s);
void names_init(void);

#ifdef __cplusplus
}
#endif

#endif
