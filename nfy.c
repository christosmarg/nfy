#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "config.h"

static Display *dpy;
static Window win;

static void
die(const char *fmt, ...)
{
        va_list args;
        
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);

        if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
                fputc(' ', stderr);
                perror(NULL);
        } else
                fputc('\n', stderr);

        exit(EXIT_FAILURE);
}

static void
recvalrm(int sig)
{
        XEvent ev;
        
        ev.type = ButtonPress;
        XSendEvent(dpy, win, 0, 0, &ev);
        XFlush(dpy);
}

int
main(int argc, char *argv[])
{
        Visual *vis;
        Colormap colormap;
        XEvent ev;
        XSetWindowAttributes attrs;
        XftColor color;
        XftFont *font;
        XftDraw *drw;
        struct sigaction sig;
        int scr, scrw, scrh;
        int x, y, w, h, th;
        int i, j, len;

        if (argc < 2)
                die("usage: %s str...", argv[0]);

        if (!(dpy = XOpenDisplay(NULL)))
                die("cannot open display");
        scr = DefaultScreen(dpy);
        vis = DefaultVisual(dpy, scr);
        colormap = DefaultColormap(dpy, scr);

        XftColorAllocName(dpy, vis, colormap, bgcol, &color);
        attrs.background_pixel = color.pixel;
        XftColorAllocName(dpy, vis, colormap, bordercol, &color);
        attrs.border_pixel = color.pixel;
        attrs.override_redirect = True;

        font = XftFontOpenName(dpy, scr, fonts);
        th = font->ascent - font->descent;

        w = 0;
        for (i = 1; i < argc; i++) {
                len = strlen(argv[i]);
                for (j = 0; j < len; j++)
                        if (argv[i][j] == '\n')
                                argv[i][j] = ' ';
                /* TODO: handle maxlen */
                if (len > w)
                        w = len;
        }

        w = w * (font->descent << 1) + borderw;
        h = (th + (borderw << 2)) * argc;
        scrw = DisplayWidth(dpy, scr);
        scrh = DisplayHeight(dpy, scr);

        switch (pos) {
                case TOP_LEFT:
                        x = mx;
                        y = my;
                        break;
                case TOP_RIGHT:
                default:
                        x = scrw - w - my;
                        y = my;
                        break;
                case BOTTOM_LEFT:
                        x = mx;
                        y = scrh - h - my;
                        break;
                case BOTTOM_RIGHT:
                        x = scrw - w - mx;
                        y = scrh - h - my;
                        break;
        }
        
        win = XCreateWindow(dpy, RootWindow(dpy, scr), x, y, w, h, borderw,
                DefaultDepth(dpy, scr), CopyFromParent, vis,
                CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attrs);

        drw = XftDrawCreate(dpy, win, vis, colormap);
        XftColorAllocName(dpy, vis, colormap, fontcol, &color);

        XSelectInput(dpy, win, ExposureMask | ButtonPress);
        XMapWindow(dpy, win);

        sig.sa_handler = recvalrm;
        sig.sa_flags = SA_RESTART;
        sigemptyset(&sig.sa_mask);
        sigaction(SIGALRM, &sig, 0);
        sigaction(SIGTERM, &sig, 0);
        sigaction(SIGINT, &sig, 0);

        if (duration > 0)
                alarm(duration);

        for (;;) {
                XNextEvent(dpy, &ev);

                if (ev.type == Expose) {
                        XClearWindow(dpy, win);
                        for (i = 1; i < argc; i++)
                                XftDrawStringUtf8(drw, &color, font,
                                        borderw, (th + (borderw << 2)) * i,
                                        (FcChar8 *)argv[i], strlen(argv[i]));
                } else if (ev.type == ButtonPress)
                        break;
        } 

        XftDrawDestroy(drw);
        XftColorFree(dpy, vis, colormap, &color);
        XftFontClose(dpy, font);
        XCloseDisplay(dpy);

        return 0;
}
