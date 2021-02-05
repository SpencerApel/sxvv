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

typedef struct thumb_s
{
    Pixmap pm;
    int x;
    int y;
    int w;
    int h;
} thumb_t;

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

void tns_init(tns_t*, int);
void tns_free(tns_t*, win_t*);

void tns_load(tns_t*, win_t*, const char*);

void tns_render(tns_t*, win_t*);
void tns_highlight(tns_t*, win_t*, int, Bool);

int tns_move_selection(tns_t*, win_t*, tnsdir_t);
int tns_scroll(tns_t*, tnsdir_t);

#endif /* THUMBS_H */
