#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdarg.h>

#define ABS(a)   ((a) < 0 ? (-(a)) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LEN(a)   (sizeof(a) / sizeof(a[0]))

#define TV_TO_DOUBLE(x) ((double) ((x).tv_sec) + 0.000001 * \
                         (double) ((x).tv_usec))

void* s_malloc(size_t);
void* s_realloc(void*, size_t);

void warn(const char*, ...);
void die(const char*, ...);

void size_readable(float*, const char**);

char* readline(FILE*);

#endif /* UTIL_H */
