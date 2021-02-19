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

/* Initalizes broken image icon, image to null
and imlib contexts if window is created */
void img_init(img_t*, win_t*);

/* Sets image to broken image and free image */
void img_free(img_t*);

/* Checks if image can be accessed
if so, set image and free */
int img_check(const char*);

/* If image can be accessed and loaded, set image,
get width and height. If not accessed, warm user
and set broken image (does not get freed like
                      in img_check) */
int img_load(img_t*, const char*);

/* If image is loaded, set the context to work with,
free the image and set to NULL*/
void img_close(img_t*);

#endif /* IMAGE_H */
