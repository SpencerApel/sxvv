#include <stdlib.h>
#include <string.h>

#include <Imlib2.h>

#include "config.h"
#include "thumbs.h"
#include "util.h"

extern Imlib_Image *im_broken;
const int thumb_dim = THUMB_SIZE + 10;

void tns_init(tns_t *tns, int cnt)
{
    if (!tns)
        return;

    tns->cnt = tns->first = tns->sel = 0; // init cnt, first, sel to 0

    // allocate and set memory
    tns->thumbs = (thumb_t*) s_malloc(cnt * sizeof(thumb_t));
    memset(tns->thumbs, 0, cnt * sizeof(thumb_t));

    tns->dirty = 0; // set to 0, meaning no refresh will be needed
}

void tns_free(tns_t *tns, win_t *win)
{
    if (!tns || !tns->thumbs)
        return;

    // for each thumbnail in thumbnail mode, cycle
    // through the list and free the pixmap
    for (int i = 0; i < tns->cnt; ++i)
        win_free_pixmap(win, tns->thumbs[i].pm);

    // free thumbnail images and set to null
    free(tns->thumbs);
    tns->thumbs = NULL;
}

void tns_load(tns_t *tns, win_t *win, const char *filename)
{
    int w, h;
    float z, zw, zh;
    thumb_t *t;
    Imlib_Image *im;

    // return if no thumbnails, window, or files
    if (!tns || !win || !filename)
        return;

    // if file loaded as image, set image context with image
    // else set image context with broken image
    if ((im = imlib_load_image(filename)))
        imlib_context_set_image(im);
    else
        imlib_context_set_image(im_broken);

    // set w, h using imlib functions
    w = imlib_image_get_width();
    h = imlib_image_get_height();

    zw = (float) THUMB_SIZE / (float) w;
    zh = (float) THUMB_SIZE / (float) h;
    z = MIN(zw, zh);
    if (!im && z > 1.0)
        z = 1.0;

    t = &tns->thumbs[tns->cnt++]; // add thumbnail image list
    t->w = z * w; // set new width
    t->h = z * h; // set new height

    // create pixmap w, h of thumbnail, and set drawable
    t->pm = win_create_pixmap(win, t->w, t->h);
    imlib_context_set_drawable(t->pm);
    imlib_context_set_anti_alias(1);

    // render image in grid in thumbnail mode
    imlib_render_image_part_on_drawable_at_size(0, 0, w, h, 0, 0, t->w, t->h);
    tns->dirty = 1; // set to one to cause window to refresh

    if (im) // after image placed on screen, free image
        imlib_free_image();
}

void tns_check_view(tns_t *tns, Bool scrolled)
{
    int r;

    if (!tns) // if no thumbnails, return
        return;

    tns->first -= tns->first % tns->cols;
    r = tns->sel % tns->cols;

    /* if the user scrolls with the mouse wheel,
    first move the selection (border around the thumbnal)
    then refresh the screen and update the thumbnails...
    simulating a scrolling function */
    if (scrolled)
    {
        /* move selection into visible area */
        // if selection is past the last photo in the bottom right corner
        if (tns->sel >= tns->first + tns->cols * tns->rows)
            // move selection border down a row, all the way to the first col
            tns->sel = tns->first + r + tns->cols * (tns->rows - 1);
        else if (tns->sel < tns->first) // if selection is beofre the first thumbnail on screen
            tns->sel = tns->first + r; // move sel border up a row, to the last col
    }
    else
    {
        /* scroll to selection */
        // if selection is past the last photo in the bottom right corner
        if (tns->first + tns->cols * tns->rows <= tns->sel)
        {
            // move down a row, and remove the top row
            tns->first = tns->sel - r - tns->cols * (tns->rows - 1);
            tns->dirty = 1; // set to one to refresh
        }
        else if (tns->first > tns->sel) // if first thumbnail is greater than selection
        {                               // aka selection is on image above what is shown
            tns->first = tns->sel - r; // move to row that selection is on
            tns->dirty = 1; // set to one to refresh
        }
    }
}

void tns_render(tns_t *tns, win_t *win)
{
    int i, cnt, r, x, y;
    thumb_t *t;

    // no thumbnail, window or doesnt need refreshed
    if (!tns || !tns->dirty || !win)
        return;

    win_clear(win); // clear the thumbnail grid

    // calculate number of rows, cols
    tns->cols = MAX(1, win->w / thumb_dim);
    tns->rows = MAX(1, win->h / thumb_dim);

    /* if amount of thumbnails < number of spaces made by rows, cols
    then space was correctly made and set vars
    else call tns_check_view to fix/reconfigure the window and grid */
    if (tns->cnt < tns->cols * tns->rows)
    {
        tns->first = 0;
        cnt = tns->cnt;
    }
    else
    {
        tns_check_view(tns, False);
        cnt = tns->cols * tns->rows;
        if ((r = tns->first + cnt - tns->cnt) >= tns->cols)
            tns->first -= r - r % tns->cols;
        if (r > 0)
            cnt -= r % tns->cols;
    }

    // set x, y coords
    r = cnt % tns->cols ? 1 : 0;
    tns->x = x = (win->w - MIN(cnt, tns->cols) * thumb_dim) / 2 + 5;
    tns->y = y = (win->h - (cnt / tns->cols + r) * thumb_dim) / 2 + 5;

    // for each thumbnail, calculate their x,y coord for each grid
    // draw the pixmap with those coords
    for (i = 0; i < cnt; ++i)
    {
        t = &tns->thumbs[tns->first + i];
        t->x = x + (THUMB_SIZE - t->w) / 2;
        t->y = y + (THUMB_SIZE - t->h) / 2;
        win_draw_pixmap(win, t->pm, t->x, t->y, t->w, t->h);
        // calculations for end of cols, placing in the next row, first col
        if ((i + 1) % tns->cols == 0)
        {
            x = tns->x;
            y += thumb_dim;
        }
        else
            x += thumb_dim;
    }

    tns->dirty = 0; // set 0, no refresh needed
    tns_highlight(tns, win, tns->sel, True); // highlight the first or previous (before re-render) thumbnail
}

void tns_highlight(tns_t *tns, win_t *win, int n, Bool hl)
{
    thumb_t *t;

    if (!tns || !win)
        return;

    // draws rectangle around selected thumbnail
    if (n >= 0 && n < tns->cnt)
    {
        t = &tns->thumbs[n];
        win_draw_rect(win, t->x - 2, t->y - 2, t->w + 4, t->h + 4, hl);
    }

    win_draw(win); // call function from window.c
}

int tns_move_selection(tns_t *tns, win_t *win, tnsdir_t dir)
{
    int old;

    if (!tns || !win)
        return 0;

    old = tns->sel; // set old selection

    // calculations to move the new selection to
    // the correct thumbnail
    switch (dir)
    {
    case TNS_LEFT:
        if (tns->sel > 0)
            --tns->sel;
        break;
    case TNS_RIGHT:
        if (tns->sel < tns->cnt - 1)
            ++tns->sel;
        break;
    case TNS_UP:
        if (tns->sel >= tns->cols)
            tns->sel -= tns->cols;
        break;
    case TNS_DOWN:
        if (tns->sel + tns->cols < tns->cnt)
            tns->sel += tns->cols;
        break;
    }

    // once selection has changed, call highlight function
    // to draw border and refresh window
    if (tns->sel != old)
    {
        tns_highlight(tns, win, old, False);
        tns_check_view(tns, False);
        if (!tns->dirty)
            tns_highlight(tns, win, tns->sel, True);
    }

    return tns->sel != old;
}

int tns_scroll(tns_t *tns, tnsdir_t dir)
{
    int old;

    if (!tns)
        return 0;

    old = tns->first;

    // if scrolled down
    if (dir == TNS_DOWN && tns->first + tns->cols * tns->rows < tns->cnt)
    {
        // update cols, and re-calculate view
        tns->first += tns->cols;
        tns_check_view(tns, True);
        tns->dirty = 1; // calls for refresh
    }
    // if scrolled up
    else if (dir == TNS_UP && tns->first >= tns->cols)
    {
        // update cols, and re-calculate view
        tns->first -= tns->cols;
        tns_check_view(tns, True);
        tns->dirty = 1; // calls for refresh
    }

    return tns->first != old;
}
