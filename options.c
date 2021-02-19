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

    // parses arguments given from stdin and runs commands accordingly
    while ((opt = getopt(argc, argv, "hqrv:")) != -1)
    {
        switch (opt)
        {
        case 'h': // help
            print_usage();
            exit(0);
        case 'q': // quiet, no output to stdout
            _options.quiet = 1;
            break;
        case 'r': // recursion for directories
            _options.recursive = 1;
            break;
        case 'v': // version
            print_version();
            exit(0);
        }
    }

    _options.filenames = (const char**) argv + optind; // set filenames into list
    _options.filecnt = argc - optind; // get file count
    _options.from_stdin = _options.filecnt == 1 && strcmp(_options.filenames[0], "-") == 0;
}
