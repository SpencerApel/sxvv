#ifndef IMAGE_H
#define IMAGE_H

#include <Imlib2.h>

#include "window.h"

typedef struct img_s
{
    Imlib_Image *im;

    int x;
    int y;
    int w;
    int h;
} img_t;

void img_init(img_t*, win_t*);
void img_free(img_t*);

int img_check(const char*);
int img_load(img_t*, const char*);
void img_close(img_t*);

#endif /* IMAGE_H */
