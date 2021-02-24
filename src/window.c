#include <string.h>

#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include "config.h"
#include "options.h"
#include "util.h"
#include "window.h"

static Cursor carrow;
//static Cursor cnone;
static GC gc;

Atom wm_delete_win; // allow window manager to close window

void win_open(win_t *win)
{
    win_env_t *e;
    XClassHint classhint;
    XColor col;
    XGCValues gcval;
    int gmask;

    if (!win) // if window not created correctly, return
        return;

    e = &win->env;
    // error handling
    if (!(e->dpy = XOpenDisplay(NULL)))
        die("could not open display");

    // Initialize window w, h, display, visual, colors
    e->scr = DefaultScreen(e->dpy);
    e->scrw = DisplayWidth(e->dpy, e->scr);
    e->scrh = DisplayHeight(e->dpy, e->scr);
    e->vis = DefaultVisual(e->dpy, e->scr);
    e->cmap = DefaultColormap(e->dpy, e->scr);
    e->depth = DefaultDepth(e->dpy, e->scr);

    // set background color
    if (XAllocNamedColor(e->dpy, DefaultColormap(e->dpy, e->scr), BG_COLOR,
                         &col, &col))
        win->bgcol = col.pixel;
    else
        die("could not allocate color: %s", BG_COLOR);

    // set selection color
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

    // Create the X window
    win->xwin = XCreateWindow(e->dpy, RootWindow(e->dpy, e->scr),
                              win->x, win->y, win->w, win->h, 0,
                              e->depth, InputOutput, e->vis, 0, None);
    if (win->xwin == None) // error handling
        die("could not create window");

    // Initialize user input
    XSelectInput(e->dpy, win->xwin, StructureNotifyMask | KeyPressMask |
                 ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    // Declare type of pointer, only arrow pointer currently
    carrow = XCreateFontCursor(e->dpy, XC_left_ptr);

    // error handling for color black
    if (!XAllocNamedColor(e->dpy, DefaultColormap(e->dpy, e->scr), "black",
                          &col, &col))
        die("could not allocate color: black");

    // setting and creating graphic content
    gcval.line_width = 2;
    gc = XCreateGC(e->dpy, win->xwin, GCLineWidth, &gcval);

    // set name for classhint
    classhint.res_name = "sxvv";
    classhint.res_class = "sxvv";
    XSetClassHint(e->dpy, win->xwin, &classhint);

    XMapWindow(e->dpy, win->xwin); // map window to display
    XFlush(e->dpy);

    // kill program in deleted by window manager (not by esc or q)
    wm_delete_win = XInternAtom(e->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(e->dpy, win->xwin, &wm_delete_win, 1);
}

void win_close(win_t *win)
{
    if (!win)
        return;

    // free cursor, graphic content, window and
    // closes the program nicely
    XFreeCursor(win->env.dpy, carrow);

    XFreeGC(win->env.dpy, gc);

    XDestroyWindow(win->env.dpy, win->xwin);
    XCloseDisplay(win->env.dpy);
}

int win_configure(win_t *win, XConfigureEvent *c)
{
    int changed;

    if (!win)
        return 0;

    // set changed if the widths or heights are not same value
    // if they are the same value, the setting of values below
    // will not matter because they are the same value
    changed = win->w != c->width || win->h != c->height;

    // change the x, y coords, width and height and
    // order width and return changed (true if needed changed)
    win->x = c->x;
    win->y = c->y;
    win->w = c->width;
    win->h = c->height;
    win->bw = c->border_width;

    return changed;
}

int win_moveresize(win_t *win, int x, int y, unsigned int w, unsigned int h) // currently not used, may fix resize issue...
{
    if (!win)
        return 0;

    // take max value for x, y. default 0
    // set width and height
    x = MAX(0, x);
    y = MAX(0, y);
    w = MIN(w, win->env.scrw - 2 * win->bw);
    h = MIN(h, win->env.scrh - 2 * win->bw);

    // if values are the same (no change) just return
    if (win->x == x && win->y == y && win->w == w && win->h == h)
        return 0;

    // set new window values
    win->x = x;
    win->y = y;
    win->w = w;
    win->h = h;

    // call X function to resize the window
    XMoveResizeWindow(win->env.dpy, win->xwin, win->x, win->y, win->w, win->h);

    return 1;
}

Pixmap win_create_pixmap(win_t *win, int w, int h)
{
    if (!win)
        return 0;
    // creates pixmap if window is created
    return XCreatePixmap(win->env.dpy, win->xwin, w, h, win->env.depth);
}

void win_free_pixmap(win_t *win, Pixmap pm)
{
    // if window and pixmap are created, free the pixmap
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

    // free the pixmap and create the new pixmap
    if (win->pm)
        XFreePixmap(e->dpy, win->pm);
    win->pm = XCreatePixmap(e->dpy, win->xwin, e->scrw, e->scrh, e->depth);

    // change graphic content with newly set window env and gc
    XChangeGC(e->dpy, gc, GCForeground, &gcval);

    XFillRectangle(e->dpy, win->pm, gc, 0, 0, e->scrw, e->scrh);
}

void win_draw_pixmap(win_t *win, Pixmap pm, int x, int y, int w, int h)
{
    if (win) // copy other "rectangle pixmap" and draw next one
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
    // update graphcis content and draw thumbnail rectangle
    XChangeGC(e->dpy, gc, GCForeground, &gcval);
    XDrawRectangle(e->dpy, win->pm, gc, x, y, w, h);
}

void win_draw(win_t *win)
{
    if (!win)
        return;

    // sets windows background pixmap and clears window
    XSetWindowBackgroundPixmap(win->env.dpy, win->xwin, win->pm);
    XClearWindow(win->env.dpy, win->xwin);
}

void win_set_cursor(win_t *win, win_cur_t cursor)
{
    if (!win)
        return;

    // defines cursor as arrow
    XDefineCursor(win->env.dpy, win->xwin, carrow);
}
