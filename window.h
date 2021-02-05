#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>

#define CLEANMASK(mask) ((mask) & ~LockMask)

typedef enum win_cur_e
{
    CURSOR_ARROW = 0,
    CURSOR_NONE,
    CURSOR_HAND,
    CURSOR_WATCH
} win_cur_t;

typedef struct win_env_s
{
    Display *dpy;
    int scr;
    int scrw, scrh;
    Visual *vis;
    Colormap cmap;
    int depth;
} win_env_t;

typedef struct win_s
{
    Window xwin;
    win_env_t env;

    unsigned long bgcol;
    unsigned long selcol;
    Pixmap pm;

    int x;
    int y;
    unsigned int w;
    unsigned int h;

    unsigned int bw;
    unsigned char fullscreen;
} win_t;

extern Atom wm_delete_win; //allows program to be closed/exited by WM

void win_open(win_t*);
void win_close(win_t*);

int win_configure(win_t*, XConfigureEvent*);
int win_moveresize(win_t*, int, int, unsigned int, unsigned int);

Pixmap win_create_pixmap(win_t*, int, int);
void win_free_pixmap(win_t*, Pixmap);

void win_clear(win_t*);
void win_draw_pixmap(win_t*, Pixmap, int, int, int, int);
void win_draw_rect(win_t*, int, int, int, int, Bool);
void win_draw(win_t*);

void win_set_cursor(win_t*, win_cur_t);

#endif /* WINDOW_H */
