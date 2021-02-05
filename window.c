#include <string.h>

#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "config.h"
#include "options.h"
#include "util.h"
#include "window.h"

static Cursor carrow;
static Cursor cnone;
static GC gc;

Atom wm_delete_win;

void win_open(win_t *win)
{
    win_env_t *e;
    XClassHint classhint;
    XColor col;
    XGCValues gcval;
    char none_data[] = {0, 0, 0, 0, 0, 0, 0, 0};
    Pixmap none;
    int gmask;

    if (!win)
        return;

    e = &win->env;
    if (!(e->dpy = XOpenDisplay(NULL)))
        die("could not open display");

    e->scr = DefaultScreen(e->dpy);
    e->scrw = DisplayWidth(e->dpy, e->scr);
    e->scrh = DisplayHeight(e->dpy, e->scr);
    e->vis = DefaultVisual(e->dpy, e->scr);
    e->cmap = DefaultColormap(e->dpy, e->scr);
    e->depth = DefaultDepth(e->dpy, e->scr);

    if (XAllocNamedColor(e->dpy, DefaultColormap(e->dpy, e->scr), BG_COLOR,
                         &col, &col))
        win->bgcol = col.pixel;
    else
        die("could not allocate color: %s", BG_COLOR);
    if (XAllocNamedColor(e->dpy, DefaultColormap(e->dpy, e->scr), SEL_COLOR,
                         &col, &col))
        win->selcol = col.pixel;
    else
        die("could not allocate color: %s", BG_COLOR);

    win->pm = 0;
    win->fullscreen = 0;

    /* determine window offsets, width & height */
    gmask = 0;

    if (!(gmask & WidthValue))
        win->w = WIN_WIDTH;
    if (win->w > e->scrw)
        win->w = e->scrw;
    if (!(gmask & HeightValue))
        win->h = WIN_HEIGHT;
    if (win->h > e->scrh)
        win->h = e->scrh;
    if (!(gmask & XValue))
        win->x = (e->scrw - win->w) / 2;
    else if (gmask & XNegative)
        win->x += e->scrw - win->w;
    if (!(gmask & YValue))
        win->y = (e->scrh - win->h) / 2;
    else if (gmask & YNegative)
        win->y += e->scrh - win->h;

    win->xwin = XCreateWindow(e->dpy, RootWindow(e->dpy, e->scr),
                              win->x, win->y, win->w, win->h, 0,
                              e->depth, InputOutput, e->vis, 0, None);
    if (win->xwin == None)
        die("could not create window");

    XSelectInput(e->dpy, win->xwin, StructureNotifyMask | KeyPressMask |
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    carrow = XCreateFontCursor(e->dpy, XC_left_ptr);

    if (!XAllocNamedColor(e->dpy, DefaultColormap(e->dpy, e->scr), "black",
                          &col, &col))
        die("could not allocate color: black");
    none = XCreateBitmapFromData(e->dpy, win->xwin, none_data, 8, 8);
    cnone = XCreatePixmapCursor(e->dpy, none, none, &col, &col, 0, 0);

    gcval.line_width = 2;
    gc = XCreateGC(e->dpy, win->xwin, GCLineWidth, &gcval);

    classhint.res_name = "sxvv";
    classhint.res_class = "sxvv";
    XSetClassHint(e->dpy, win->xwin, &classhint);

    XMapWindow(e->dpy, win->xwin);
    XFlush(e->dpy);

    wm_delete_win = XInternAtom(e->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(e->dpy, win->xwin, &wm_delete_win, 1);
}

void win_close(win_t *win)
{
    if (!win)
        return;

    XFreeCursor(win->env.dpy, carrow);
    XFreeCursor(win->env.dpy, cnone);

    XFreeGC(win->env.dpy, gc);

    XDestroyWindow(win->env.dpy, win->xwin);
    XCloseDisplay(win->env.dpy);
}

int win_configure(win_t *win, XConfigureEvent *c)
{
    int changed;

    if (!win)
        return 0;

    changed = win->w != c->width || win->h != c->height;

    win->x = c->x;
    win->y = c->y;
    win->w = c->width;
    win->h = c->height;
    win->bw = c->border_width;

    return changed;
}

int win_moveresize(win_t *win, int x, int y, unsigned int w, unsigned int h)
{
    if (!win)
        return 0;

    x = MAX(0, x);
    y = MAX(0, y);
    w = MIN(w, win->env.scrw - 2 * win->bw);
    h = MIN(h, win->env.scrh - 2 * win->bw);

    if (win->x == x && win->y == y && win->w == w && win->h == h)
        return 0;

    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;

    XMoveResizeWindow(win->env.dpy, win->xwin, win->x, win->y, win->w, win->h);

    return 1;
}

Pixmap win_create_pixmap(win_t *win, int w, int h)
{
    if (!win)
        return 0;

    return XCreatePixmap(win->env.dpy, win->xwin, w, h, win->env.depth);
}

void win_free_pixmap(win_t *win, Pixmap pm)
{
    if (win && pm)
        XFreePixmap(win->env.dpy, pm);
}

void win_clear(win_t *win)
{
    win_env_t *e;
    XGCValues gcval;

    if (!win)
        return;

    e = &win->env;
    gcval.foreground = win->fullscreen ? BlackPixel(e->dpy, e->scr) :
                       win->bgcol;
    if (win->pm)
        XFreePixmap(e->dpy, win->pm);
    win->pm = XCreatePixmap(e->dpy, win->xwin, e->scrw, e->scrh, e->depth);

    XChangeGC(e->dpy, gc, GCForeground, &gcval);
    XFillRectangle(e->dpy, win->pm, gc, 0, 0, e->scrw, e->scrh);
}

void win_draw_pixmap(win_t *win, Pixmap pm, int x, int y, int w, int h)
{
    if (win)
        XCopyArea(win->env.dpy, pm, win->pm, gc, 0, 0, w, h, x, y);
}

void win_draw_rect(win_t *win, int x, int y, int w, int h, Bool sel)
{
    win_env_t *e;
    XGCValues gcval;

    if (!win)
        return;

    e = &win->env;

    if (sel)
        gcval.foreground = win->selcol;
    else
        gcval.foreground = win->fullscreen ? BlackPixel(e->dpy, e->scr) :
                           win->bgcol;
    XChangeGC(e->dpy, gc, GCForeground, &gcval);
    XDrawRectangle(e->dpy, win->pm, gc, x, y, w, h);
}

void win_draw(win_t *win)
{
    if (!win)
        return;

    XSetWindowBackgroundPixmap(win->env.dpy, win->xwin, win->pm);
    XClearWindow(win->env.dpy, win->xwin);
}

void win_set_cursor(win_t *win, win_cur_t cursor)
{
    if (!win)
        return;

    switch (cursor)
    {
    case CURSOR_NONE:
        XDefineCursor(win->env.dpy, win->xwin, cnone);
        break;
    case CURSOR_ARROW:
    default:
        XDefineCursor(win->env.dpy, win->xwin, carrow);
        break;
    }
}
