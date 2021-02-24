#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdarg.h>

#define ABS(a)   ((a) < 0 ? (-(a)) : (a))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define LEN(a)   (sizeof(a) / sizeof(a[0]))

/* If program could not allocate sufficient space for
the memory block pointed to by ptr, call die function */
void* s_malloc(size_t);

/* If program could not resize the memory block
pointed to by ptr, call die function */
void* s_realloc(void*, size_t);

/* Function to print detailed warning accordingly */
void warn(const char*, ...);

/* Function to drint detailed error and cleanup/kill */
void die(const char*, ...);

/* Sets size of file and the unit of szie */
void size_readable(float*, const char**);

/* Reads filenames passed from stream and allocates memory */
char* readline(FILE*);

#endif /* UTIL_H */
