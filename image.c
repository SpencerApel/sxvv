#include <unistd.h>

#include "config.h"
#include "icon.h"
#include "image.h"
#include "options.h"
#include "util.h"

Imlib_Image *im_broken;

void img_init(img_t *img, win_t *win)
{
    // set im_broken to broken image from icon.h
    im_broken = imlib_create_image_using_data(32, 32, icon_broken);

    if (img)
        img->im = NULL; // set image var to null

    if (win) // if window is initialized, set imlib contexts
    {
        imlib_context_set_display(win->env.dpy);
        imlib_context_set_visual(win->env.vis);
        imlib_context_set_colormap(win->env.cmap);
    }
}

void img_free(img_t* img)
{
    // set context to broken image and free image
    imlib_context_set_image(im_broken);
    imlib_free_image();
}

int img_check(const char *filename)
{
    Imlib_Image *im;

    if (!filename) // return if no filename given
        return 0;

    // test for access of file, and file is loaded into im var
    if (!access(filename, F_OK) && (im = imlib_load_image(filename)))
    {
        imlib_context_set_image(im); // set the image context were working with
        imlib_free_image(); // and free the image
        return 1; // success
    }
    else
    {
        warn("could not open file: %s", filename);
        return 0;
    }
}

int img_load(img_t *img, const char *filename)
{
    if (!img || !filename) // return if no img or filename
        return 0;

    // test for access of file, and file is loaded into im var
    // if so set the image context
    if (!access(filename, F_OK) && (img->im = imlib_load_image(filename)))
        imlib_context_set_image(img->im);
    else
    {
        // If not accessed, warm user and set broken image
        warn("could not open file: %s", filename);
        imlib_context_set_image(im_broken);
        imlib_context_set_anti_alias(0);
    }
    // set image width and height
    img->w = imlib_image_get_width();
    img->h = imlib_image_get_height();

    return 1; // success
}

void img_close(img_t *img)
{
    // If image is loaded, set the context to work with,
    // free the image and set to NULL
    if (img && img->im)
    {
        imlib_context_set_image(img->im);
        imlib_free_image();
        img->im = NULL;
    }
}
