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

void print_usage();
void print_version();

void parse_options(int, char**);

#endif /* OPTIONS_H */
