#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

void create_window(void); //Function prototype

int main(int argc, char *argv[]) {
        create_window();
        return 0;
}

void create_window() {
        Display *d;
        int screen;
        Window win;
        XEvent event;

        d = XOpenDisplay(NULL);

        if (d == NULL) {
                fprintf(stderr, "Cannot open display (NULL)\n");
                exit(1);
        }

        screen = DefaultScreen(d);

        win = XCreateSimpleWindow(d, RootWindow(d, screen),
                        10, 10, 600, 400, //position, size
                        1, BlackPixel(d, screen), WhitePixel(d, screen)); //border, background

        XSelectInput(d, win, ExposureMask | KeyPressMask);
        XMapWindow (d, win);

        while(1) {
                XNextEvent(d, &event);
        }
}

//void create_thumbnails() {
        // ffmpeg -i inputvid.mp4 -ss 00:00:10 -frames:v 1 tempimg.jpg

        /*temporary dir to store thumbnails
        way to save thumbnails locally, or load everytime?
        if thumbnails saved locally, way to match image to video
        */

        /*this may be another function, but need to place image
          on the screen (in thumbnail view like sxiv) also list
          the name of the video, allow scrolling through all the
          thumbnails.
          When a video (aka thumbnail) is selected, open up a
          video player of choice (not gonna support playing videos*/
//}
