#include <stdlib.h>
#include <string.h>

#include "options.h"
#include "util.h"

#define FNAME_LEN 512

void cleanup();

void* s_malloc(size_t size)
{
    void *ptr;

    // If program could not allocate sufficient space for
    // the memory block pointed to by ptr
    if (!(ptr = malloc(size)))
        die("could not allocate memory"); // kill program with error
    return ptr;
}

void* s_realloc(void *ptr, size_t size)
{
    // If program could not resize sufficient space for
    // the memory block pointed to by ptr
    if (!(ptr = realloc(ptr, size)))
        die("could not allocate memory"); // kill program with error
    return ptr;
}

void warn(const char* fmt, ...)
{
    va_list args;

    // if no warning or option '-q' was specified
    if (!fmt || options->quiet)
        return;

    // prints warning
    va_start(args, fmt);
    fprintf(stderr, "sxvv: warning: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void die(const char* fmt, ...)
{
    va_list args;

    if (!fmt) // if no warning, return
        return;

    // pring error message
    va_start(args, fmt);
    fprintf(stderr, "sxvv: error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    // cleanup and exit program
    cleanup();
    exit(1);
}

void size_readable(float *size, const char **unit)
{
    // file size units
    const char *units[] = { "", "K", "M", "G" };
    int i;

    for (i = 0; i < LEN(units) && *size > 1024; ++i)
        *size /= 1024; // cycle through units and divide by 1024 to set correct size
    *unit = units[MIN(i, LEN(units) - 1)]; // sets correct unit from file
}

char* readline(FILE *stream)
{
    size_t len;
    char *buf, *s, *end;

    if (!stream || feof(stream) || ferror(stream))
        return NULL;

    len = FNAME_LEN; // sets default length of file name
    s = buf = (char*) s_malloc(len * sizeof(char)); // allocate enough memory for file name

    // while not end of stream, do this
    do
    {
        *s = '\0';

        // get string of finite length from stream.
        fgets(s, len - (s - buf), stream);

        // when end of line, which shell reads as \n
        // set char to \0 which represents end of  string
        if ((end = strchr(s, '\n')))
        {
            *end = '\0';
        }
        // the allocated memory is less than the amount
        // of memory allocated, reallocate and calculate new length
        else if (strlen(s) + 1 == len - (s - buf))
        {
            buf = (char*) s_realloc(buf, 2 * len * sizeof(char));
            s = buf + len - 1;
            len *= 2;
        }
        else
        {
            s += strlen(s);
        }
    }
    while (!end && !feof(stream) && !ferror(stream));

    // if an error in stream set s to null
    if (ferror(stream))
    {
        s = NULL;
    }
    // allocate memory thats the size of buf for s, copy buf to s
    else
    {
        s = (char*) s_malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(s, buf);
    }

    free(buf);

    return s;
}
