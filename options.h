#ifndef OPTIONS_H
#define OPTIONS_H

#include "image.h"

typedef struct options_s
{
    const char **filenames;
    int filecnt;
    unsigned char from_stdin;

    unsigned char quiet;
    unsigned char recursive;
} options_t;

extern const options_t *options;

/* prints usage info if user specifies -h
or does not specify any input */
void print_usage();
void print_version(); // prints sxvv version is -v specified

/* parses standard input options, like -h, -v, plus others
also puts filenames into a list and sets a file count var from stdin */
void parse_options(int, char**);

#endif /* OPTIONS_H */
