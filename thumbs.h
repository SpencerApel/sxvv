#ifndef THUMBS_H
#define THUMBS_H

#include "window.h"

typedef enum tnsdir_e
{
    TNS_LEFT = 0,
    TNS_RIGHT,
    TNS_UP,
    TNS_DOWN
} tnsdir_t;

/* struct for each individual thumbnail */
typedef struct thumb_s
{
    Pixmap pm;
    int x;
    int y;
    int w;
    int h;
} thumb_t;

/* struct for the thumbnail grid */
typedef struct tns_s
{
    thumb_t *thumbs;
    int cnt;
    int x;
    int y;
    int cols;
    int rows;
    int first;
    int sel;
    unsigned char dirty;
} tns_t;

/* Initialize variables, allocate and set memory */
void tns_init(tns_t*, int);

/* Free thumbnail pixmap and thumbnail images
also set thumbnails to null */
void tns_free(tns_t*, win_t*);

/* Sets image context, calculates width and height
creates pixmap, renders image in pixmap in a grid */
void tns_load(tns_t*, win_t*, const char*);

/* Clears window, calculates x, y cords, draws
pixmap of image on grid and runs tns_highlight */
void tns_render(tns_t*, win_t*);

/* Draws rectangle around image that is
currently "selected" */
void tns_highlight(tns_t*, win_t*, int, Bool);

/* Moves selection, up, down, left, right when
arrow keys or vim bindings are clicked */
int tns_move_selection(tns_t*, win_t*, tnsdir_t);

/* Calculates what to do when user scrolls mouse wheel
and then calls a function re-renders window */
int tns_scroll(tns_t*, tnsdir_t);

#endif /* THUMBS_H */
