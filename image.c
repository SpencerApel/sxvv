#include <unistd.h>

#include "config.h"
#include "icon.h"
#include "image.h"
#include "options.h"
#include "util.h"

Imlib_Image *im_broken;

void img_init(img_t *img, win_t *win)
{

    im_broken = imlib_create_image_using_data(32, 32, icon_broken);

    if (img)
        img->im = NULL;

    if (win)
    {
        imlib_context_set_display(win->env.dpy);
        imlib_context_set_visual(win->env.vis);
        imlib_context_set_colormap(win->env.cmap);
    }
}

void img_free(img_t* img)
{
    imlib_context_set_image(im_broken);
    imlib_free_image();
}

int img_check(const char *filename)
{
    Imlib_Image *im;

    if (!filename)
        return 0;

    if (!access(filename, F_OK) && (im = imlib_load_image(filename)))
    {
        imlib_context_set_image(im);
        imlib_image_set_changes_on_disk();
        imlib_free_image();
        return 1;
    }
    else
    {
        warn("could not open file: %s", filename);
        return 0;
    }
}

int img_load(img_t *img, const char *filename)
{
    if (!img || !filename)
        return 0;

    if (!access(filename, F_OK) && (img->im = imlib_load_image(filename)))
    {
        imlib_context_set_image(img->im);
        imlib_image_set_changes_on_disk();
    }
    else
    {
        warn("could not open file: %s", filename);
        imlib_context_set_image(im_broken);
        imlib_context_set_anti_alias(0);
    }

    img->w = imlib_image_get_width();
    img->h = imlib_image_get_height();

    return 1;
}

void img_close(img_t *img)
{
    if (img && img->im)
    {
        imlib_context_set_image(img->im);
        imlib_free_image();
        img->im = NULL;
    }
}
