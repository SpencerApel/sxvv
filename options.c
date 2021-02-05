#define _XOPEN_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "options.h"
#include "util.h"

options_t _options;
const options_t *options = (const options_t*) &_options;

void print_usage()
{
    printf("usage: sxvv [-hqrv] FILES...\n");
}

void print_version()
{
    printf("sxvv version %s - simple x video viewer\n", "0.01");
}

void parse_options(int argc, char **argv)
{
    int opt;

    _options.quiet = 0;
    _options.recursive = 0;

    while ((opt = getopt(argc, argv, "hqrv:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage();
            exit(0);
        case 'q':
            _options.quiet = 1;
            break;
        case 'r':
            _options.recursive = 1;
            break;
        case 'v':
            print_version();
            exit(0);
        }
    }

    _options.filenames = (const char**) argv + optind;
    _options.filecnt = argc - optind;
    _options.from_stdin = _options.filecnt == 1 &&
                          strcmp(_options.filenames[0], "-") == 0;
}
