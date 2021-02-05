#include <stdlib.h>
#include <string.h>

#include "options.h"
#include "util.h"

#define FNAME_LEN 512

void cleanup();

void* s_malloc(size_t size)
{
    void *ptr;

    if (!(ptr = malloc(size)))
        die("could not allocate memory");
    return ptr;
}

void* s_realloc(void *ptr, size_t size)
{
    if (!(ptr = realloc(ptr, size)))
        die("could not allocate memory");
    return ptr;
}

void warn(const char* fmt, ...)
{
    va_list args;

    if (!fmt || options->quiet)
        return;

    va_start(args, fmt);
    fprintf(stderr, "sxvv: warning: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void die(const char* fmt, ...)
{
    va_list args;

    if (!fmt)
        return;

    va_start(args, fmt);
    fprintf(stderr, "sxvv: error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    cleanup();
    exit(1);
}

void size_readable(float *size, const char **unit)
{
    const char *units[] = { "", "K", "M", "G" };
    int i;

    for (i = 0; i < LEN(units) && *size > 1024; ++i)
        *size /= 1024;
    *unit = units[MIN(i, LEN(units) - 1)];
}

char* readline(FILE *stream)
{
    size_t len;
    char *buf, *s, *end;

    if (!stream || feof(stream) || ferror(stream))
        return NULL;

    len = FNAME_LEN;
    s = buf = (char*) s_malloc(len * sizeof(char));

    do
    {
        *s = '\0';
        fgets(s, len - (s - buf), stream);
        if ((end = strchr(s, '\n')))
        {
            *end = '\0';
        }
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

    if (ferror(stream))
    {
        s = NULL;
    }
    else
    {
        s = (char*) s_malloc((strlen(buf) + 1) * sizeof(char));
        strcpy(s, buf);
    }

    free(buf);

    return s;
}
