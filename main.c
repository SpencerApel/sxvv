#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "image.h"
#include "options.h"
#include "thumbs.h"
#include "util.h"
#include "window.h"

int check_append(const char*);
void read_dir_rec(const char*);
void run();

img_t img;
tns_t tns;
win_t win;

#define DNAME_CNT 512
#define FNAME_CNT 1024
const char **filenames;
int filecnt, fileidx;
size_t filesize;

#define TITLE_LEN 256
char win_title[TITLE_LEN];

void cleanup()
{
    img_close(&img);
    img_free(&img);
    tns_free(&tns, &win);
    win_close(&win);
}

int load_image()
{
    struct stat fstats;

    img_close(&img);

    if (!stat(filenames[fileidx], &fstats))
        filesize = fstats.st_size;
    else
        filesize = 0;

    return img_load(&img, filenames[fileidx]);
}

int main(int argc, char **argv)
{
    int i;
    const char *filename;
    struct stat fstats;

    parse_options(argc, argv); // figure out what to do with cmd line arguments

    if (!options->filecnt) // if no arguments are given, print usage and exit
    {
        print_usage();
        exit(1);
    }

    if (options->recursive || options->from_stdin)
        filecnt = FNAME_CNT;
    else
        filecnt = options->filecnt;

    filenames = (const char**) s_malloc(filecnt * sizeof(const char*));
    fileidx = 0;

    if (options->from_stdin)
    {
        while ((filename = readline(stdin)))
        {
            if (!*filename || !check_append(filename))
                free((void*) filename);
        }
    }
    else
    {
        for (i = 0; i < options->filecnt; ++i)
        {
            filename = options->filenames[i];
            if (!stat(filename, &fstats) && S_ISDIR(fstats.st_mode))
            {
                if (options->recursive)
                    read_dir_rec(filename);
                else
                    warn("ignoring directory: %s", filename);
            }
            else
            {
                check_append(filename);
            }
        }
    }

    filecnt = fileidx;
    fileidx = 0;

    if (!filecnt)
    {
        fprintf(stderr, "sxvv: no valid image filename given, aborting\n");
        exit(1);
    }

    win_open(&win);
    img_init(&img, &win);

    tns_init(&tns, filecnt);
    win_clear(&win);
    win_draw(&win);

    run();
    cleanup();

    return 0;
}

int check_append(const char *filename)
{
    if (!filename)
        return 0;

    if (img_check(filename))
    {
        if (fileidx == filecnt)
        {
            filecnt *= 2;
            filenames = (const char**) s_realloc(filenames, filecnt * sizeof(const char*));
        }
        filenames[fileidx++] = filename;
        return 1;
    }
    else
    {
        return 0;
    }
}

int fncmp(const void *a, const void *b)
{
    return strcoll(*((char* const*) a), *((char* const*) b));
}

void read_dir_rec(const char *dirname)
{
    char *filename;
    const char **dirnames;
    int dircnt, diridx;
    int fcnt, fstart;
    unsigned char first;
    size_t len;
    DIR *dir;
    struct dirent *dentry;
    struct stat fstats;

    // if not directory, return
    if (!dirname)
        return;

    dircnt = DNAME_CNT;
    diridx = first = 1;
    dirnames = (const char**) s_malloc(dircnt * sizeof(const char*));
    dirnames[0] = dirname;

    fcnt = 0;
    fstart = fileidx;

    while (diridx > 0)
    {
        dirname = dirnames[--diridx];
        if (!(dir = opendir(dirname)))
        {
            warn("could not open directory: %s", dirname);
        }
        else
        {
            while ((dentry = readdir(dir)))
            {
                if (!strcmp(dentry->d_name, ".") || !strcmp(dentry->d_name, ".."))
                    continue;

                len = strlen(dirname) + strlen(dentry->d_name) + 2;
                filename = (char*) s_malloc(len * sizeof(char));
                snprintf(filename, len, "%s/%s", dirname, dentry->d_name);

                if (!stat(filename, &fstats) && S_ISDIR(fstats.st_mode))
                {
                    if (diridx == dircnt)
                    {
                        dircnt *= 2;
                        dirnames = (const char**) s_realloc(dirnames, dircnt * sizeof(const char*));
                    }
                    dirnames[diridx++] = filename;
                }
                else
                {
                    if (check_append(filename))
                        ++fcnt;
                    else
                        free(filename);
                }
            }
            closedir(dir);
        }

        if (!first)
            free((void*) dirname);
        else
            first = 0;
    }

    if (fcnt > 1)
        qsort(filenames + fstart, fcnt, sizeof(char*), fncmp);

    free(dirnames);
}

/* event handling */
void redraw()
{
    // re-render thumbnails
    tns_render(&tns, &win);
}

void on_keypress(XKeyEvent *kev)
{
    char key;
    KeySym ksym;
    int changed;

    // if no keypress event, return
    if (!kev)
        return;

    XLookupString(kev, &key, 1, &ksym, NULL); // translates a key event to a KeySym and a string
    changed = 0;

    switch (ksym)
    {
    /* move selection */
    /*h, left arrow moves selection to the left
      j, down arrow moves selection down
      k, up arrow moves selection up
      l, right arrow moves selection to the right

      then calls on tns_move_selection to actually move the selection
      to the correct position, which returns a value greater than 0
      causing the screen to refresh*/
    case XK_h:
    case XK_Left:
        changed = tns_move_selection(&tns, &win, TNS_LEFT);
        break;
    case XK_j:
    case XK_Down:
        changed = tns_move_selection(&tns, &win, TNS_DOWN);
        break;
    case XK_k:
    case XK_Up:
        changed = tns_move_selection(&tns, &win, TNS_UP);
        break;
    case XK_l:
    case XK_Right:
        changed = tns_move_selection(&tns, &win, TNS_RIGHT);
        break;

    // g, goes to first thumbnail in list
    case XK_g:
        if (tns.sel != 0)
        {
            // set tns.sel to 0 thus selecting the first one
            tns.sel = 0;
            changed = tns.dirty = 1;
        }
        break;
    // G, goes to last thumbnail in list
    case XK_G:
        if (tns.sel != tns.cnt - 1)
        {
            // set tns.sel to the total amount of thumbnails
            // thus selecting the last one
            tns.sel = tns.cnt - 1;
            changed = tns.dirty = 1;
        }
        break;
    // escape and q key quits program
    case XK_Escape:
    case XK_q:
        cleanup();
        exit(0);
    }
    // if user presses button (not esc or q), redraw screen
    if (changed)
        redraw();
}

void on_buttonpress(XButtonEvent *bev)
{
    int changed;

    // if no button event, return
    if (!bev)
        return;

    changed = 0;
    switch (bev->button)
    {
    // scroll up
    case Button4:
        changed = tns_scroll(&tns, TNS_UP);
        break;
    // scroll down
    case Button5:
        changed = tns_scroll(&tns, TNS_DOWN);
        break;
    }
    // if user scrolls up or down, redraw screen
    if (changed)
        redraw();
}

void run()
{
    XEvent ev;

    win_set_cursor(&win, CURSOR_ARROW); // set the shape of the cursor

    while (1)
    {
        // if the number of thumbnails loaded is
        // less than the number of 'files' to load
        if(tns.cnt < filecnt)
        {
            // while tns.cnt is still less than nnumber of 'files' to load
            // and not pending on display, load the next file/image
            while (!XPending(win.env.dpy) && tns.cnt < filecnt)
                tns_load(&tns, &win, filenames[tns.cnt]);

            // if not pending on display, redraw
            if (!XPending(win.env.dpy))
                redraw();
        }

        if (!XNextEvent(win.env.dpy, &ev))
        {
            switch (ev.type)
            {
            // key navigation/selection
            case KeyPress:
                on_keypress(&ev.xkey);
                break;
            // mouse buttons, just scrolling
            case ButtonPress:
                on_buttonpress(&ev.xbutton);
                break;
            // close/exit program from WM
            case ClientMessage:
                if ((Atom) ev.xclient.data.l[0] == wm_delete_win)
                    return;
                break;
            }
        }
    }
}
