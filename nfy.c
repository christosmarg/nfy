/* See LICENSE file for copyright and license details. */
#include <err.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrandr.h>

#include "config.h"

static void sighandler(int);

static char *argv0;
static Display *dpy;
static Window win;

static void
sighandler(int sig)
{
	XEvent ev;
	
	ev.type = ButtonPress;
	XSendEvent(dpy, win, 0, 0, &ev);
	XFlush(dpy);
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-v] str...\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	Colormap colormap;
	Visual *vis;
	XEvent ev;
	XRRCrtcInfo *info = NULL;
	XRRScreenResources *screens;
	XSetWindowAttributes attrs;
	XftColor color;
	XftDraw *drw;
	XftFont *font;
	struct flock fl;
	struct sigaction sig;
	pid_t pid;
	int lockfd;
	int scr, scrw, scrh;
	int x, y, w, h, th;
	int i, j, len, argi;
	char ch;

	argv0 = *argv;
	if (argc < 2)
		usage();
	while ((ch = getopt(argc, argv, "v")) != -1) {
		switch (ch) {
		case 'v':
			fprintf(stderr, "%s-"VERSION"\n", argv0);
			exit(1);
		case '?':
		default:
			usage();
		}
	}
	/*argc -= optind;*/
	/*argv += optind;*/
	argi = optind;

	if ((pid = fork()) < 0)
		err(1, "fork");
	if (pid > 0)
		_exit(0);
	umask(0);
	if (setsid() < 0)
		err(1, "setsid");

	if ((lockfd = open(lockfile, O_CREAT | O_RDWR, 0600)) < 0)
		err(1, "open");
	if (!(dpy = XOpenDisplay(NULL)))
		errx(1, "XOpenDisplay");

	scr = DefaultScreen(dpy);
	screens = XRRGetScreenResources(dpy, RootWindow(dpy, scr));
	info = XRRGetCrtcInfo(dpy, screens, screens->crtcs[0]);
	scrw = info->width;
	scrh = info->height;

	vis = DefaultVisual(dpy, scr);
	colormap = DefaultColormap(dpy, scr);
	XftColorAllocName(dpy, vis, colormap, bgcolor, &color);
	attrs.background_pixel = color.pixel;
	XftColorAllocName(dpy, vis, colormap, bordercolor, &color);
	attrs.border_pixel = color.pixel;
	attrs.override_redirect = True;

	font = XftFontOpenName(dpy, scr, fonts);
	th = font->ascent - font->descent;

	w = 0;
	for (i = argi; i < argc; i++) {
		len = strlen(argv[i]);
		j = len;
		while (j--)
			if (**argv == '\n')
				**argv++ = ' ';
		/* TODO: handle maxlen */
		if (len > w)
			w = len;
	}

	w *= th + borderw;
	h = th * (argc - 1) + (linespace * (argc - 2)) + (padding << 1);

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
	XftColorAllocName(dpy, vis, colormap, fontcolor, &color);

	XSelectInput(dpy, win, ExposureMask | ButtonPress);
	XMapWindow(dpy, win);

	sig.sa_handler = sighandler;
	sig.sa_flags = SA_RESTART;
	(void)sigemptyset(&sig.sa_mask);
	if (sigaction(SIGALRM, &sig, NULL) < 0)
		err(1, "sigaction(SIGALRM)");
	if (sigaction(SIGTERM, &sig, NULL) < 0)
		err(1, "sigaction(SIGTERM)");
	if (sigaction(SIGINT, &sig, NULL) < 0)
		err(1, "sigaction(SIGINT)");

	fl.l_len = 0;
	fl.l_start = 0;
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	if (fcntl(lockfd, F_SETLKW, &fl) < 0)
		err(1, "fcntl(F_SETLKW)");

	if (duration > 0)
		(void)alarm(duration);

	for (;;) {
		XNextEvent(dpy, &ev);
		if (ev.type == Expose) {
			XClearWindow(dpy, win);
			for (i = argi; i < argc; i++)
				XftDrawStringUtf8(drw, &color, font,
				    w >> 3, linespace * (i - 1) + th * i + padding,
				    (FcChar8 *)argv[i], strlen(argv[i]));
		} else if (ev.type == ButtonPress)
			break;
	}

	XftDrawDestroy(drw);
	XftColorFree(dpy, vis, colormap, &color);
	XftFontClose(dpy, font);
	XRRFreeCrtcInfo(info);
	XRRFreeScreenResources(screens);
	XCloseDisplay(dpy);
	(void)close(lockfd);

	return 0;
}
