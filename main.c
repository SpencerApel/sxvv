#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
//#include <X11/keysym.h>

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
    static int in = 0;

    if (!in++)
    {
        img_close(&img);
        img_free(&img);
        tns_free(&tns, &win);
        win_close(&win);
    }
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

    parse_options(argc, argv);

    if (!options->filecnt)
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
            filenames = (const char**) s_realloc(filenames,
                                                 filecnt * sizeof(const char*));
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
                        dirnames = (const char**) s_realloc(dirnames,
                                                            dircnt * sizeof(const char*));
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
#define TO_THUMBS_LOAD 75000;
int timo_cursor;
int timo_redraw;

unsigned char drag;
int mox, moy;

void redraw()
{
    tns_render(&tns, &win);
    timo_redraw = 0;
}

void on_keypress(XKeyEvent *kev)
{
    char key;
    KeySym ksym;
    int changed;

    if (!kev)
        return;

    XLookupString(kev, &key, 1, &ksym, NULL);
    changed = 0;

    switch (ksym)
    {
    /* move selection */
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
    case XK_g:
        if (tns.sel != 0)
        {
            tns.sel = 0;
            changed = tns.dirty = 1;
        }
        break;
    case XK_G:
        if (tns.sel != tns.cnt - 1)
        {
            tns.sel = tns.cnt - 1;
            changed = tns.dirty = 1;
        }
    }
    /* common key mappings */
    switch (ksym)
    {
    case XK_Escape:
        cleanup();
        exit(2);
    case XK_q:
        cleanup();
        exit(0);
    }
    if (changed)
        redraw();
}

void on_buttonpress(XButtonEvent *bev)
{
    int changed;

    if (!bev)
        return;

    changed = 0;
    switch (bev->button)
    {
    case Button4:
        changed = tns_scroll(&tns, TNS_UP);
        break;
    case Button5:
        changed = tns_scroll(&tns, TNS_DOWN);
        break;
    }

    if (changed)
        redraw();
}

void run()
{
    int xfd, timeout;
    fd_set fds;
    struct timeval tt, t0, t1;
    XEvent ev;

    timo_cursor = timo_redraw = 0;
    drag = 0;

    while (1)
    {
        if(tns.cnt < filecnt)
        {
            win_set_cursor(&win, CURSOR_WATCH);
            gettimeofday(&t0, 0);

            while (!XPending(win.env.dpy) && tns.cnt < filecnt)
            {
                tns_load(&tns, &win, filenames[tns.cnt]);
                gettimeofday(&t1, 0);
                if (TV_TO_DOUBLE(t1) - TV_TO_DOUBLE(t0) >= 0.25)
                    break;
            }
            if (tns.cnt == filecnt)
                win_set_cursor(&win, CURSOR_ARROW);
            if (!XPending(win.env.dpy))
            {
                redraw();
                continue;
            }
            else
            {
                timo_redraw = TO_THUMBS_LOAD;
            }
        }
        else if (timo_cursor || timo_redraw)
        {
            gettimeofday(&t0, 0);
            if (timo_cursor && timo_redraw)
                timeout = MIN(timo_cursor, timo_redraw);
            else if (timo_cursor)
                timeout = timo_cursor;
            else
                timeout = timo_redraw;
            tt.tv_sec = timeout / 1000000;
            tt.tv_usec = timeout % 1000000;
            xfd = ConnectionNumber(win.env.dpy);
            FD_ZERO(&fds);
            FD_SET(xfd, &fds);

            if (!XPending(win.env.dpy))
                select(xfd + 1, &fds, 0, 0, &tt);
            gettimeofday(&t1, 0);
            timeout = MIN((TV_TO_DOUBLE(t1) - TV_TO_DOUBLE(t0)) * 1000000, timeout);

            if (timo_cursor)
            {
                timo_cursor = MAX(0, timo_cursor - timeout);
                if (!timo_cursor)
                    win_set_cursor(&win, CURSOR_NONE);
            }
            if (timo_redraw)
            {
                timo_redraw = MAX(0, timo_redraw - timeout);
                if (!timo_redraw)
                    redraw();
            }
            if (!XPending(win.env.dpy) && (timo_cursor || timo_redraw))
                continue;
        }

        if (!XNextEvent(win.env.dpy, &ev))
        {
            switch (ev.type)
            {
            case KeyPress:
                on_keypress(&ev.xkey);
                break;
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
