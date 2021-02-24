#ifndef WINDOW_H
#define WINDOW_H

#include <X11/Xlib.h>

/* Window cursor icon, only arrow, no other for now */
typedef enum win_cur_e
{
    CURSOR_ARROW = 0,
} win_cur_t;

/* Window environment struct dealing with
windows display, visuals, colors and size */
typedef struct win_env_s
{
    Display *dpy;
    int scr;
    int scrw, scrh;
    Visual *vis;
    Colormap cmap;
    int depth;
} win_env_t;

/* Struct for the window dealing with colors,
pixmap, coordinates and borders */
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

/* Creates window and everything entailing the window sets
user input, cursor, graphic content, classhint, map window to display */
void win_open(win_t*);

/* Frees necessary content and closes window nicely */
void win_close(win_t*);

/* Sets the x, y coords, width and height, order width
and return changed */
int win_configure(win_t*, XConfigureEvent*);

/* Updates x, y coordinates and width, height, then resizes window */
int win_moveresize(win_t*, int, int, unsigned int, unsigned int);

/* Creates pixmap is window is created */
Pixmap win_create_pixmap(win_t*, int, int);

/* Fress pixmap */
void win_free_pixmap(win_t*, Pixmap);

/* Deletes and creates a new pixmap and
updates the graphics content */
void win_clear(win_t*);

/* Draws the pixmap for the grid */
void win_draw_pixmap(win_t*, Pixmap, int, int, int, int);

/* Update graphcis content and draw thumbnail rectangle */
void win_draw_rect(win_t*, int, int, int, int, Bool);

/* Sets windows background pixmap and clears window */
void win_draw(win_t*);

/* Defines cursor as arrow */
void win_set_cursor(win_t*, win_cur_t);

#endif /* WINDOW_H */
