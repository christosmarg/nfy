/* See LICENSE file for copyright and license details. */
#include <errno.h>
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

static void die(const char *, ...);
static void recvalrm(int);

static char *argv0;
static Display *dpy;
static Window win;

static void
die(const char *fmt, ...)
{
	va_list args;

	fprintf(stderr, "%s: ", argv0);
	
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
	Colormap colormap;
	Visual *vis;
	XEvent ev;
	XRRCrtcInfo *info = NULL;
	XRRScreenResources *screens;
	XSetWindowAttributes attrs;
	XftColor color;
	XftDraw *drw;
	XftFont *font;
	struct sigaction sig;
	pid_t pid;
	int lockfd;
	int scr, scrw, scrh;
	int x, y, w, h, th;
	int i, j, len;

	argv0 = *argv;
	if (argc < 2) {
		fprintf(stderr, "usage: %s str...\n", argv0);
		return 1;
	}

	if ((lockfd = open(lockfile, O_CREAT | O_RDWR, 0600)) == -1)
		die("open:");
	if (!(dpy = XOpenDisplay(NULL)))
		die("XOpenDisplay:");

	if ((pid = fork()) < 0)
		die("fork:");
	if (pid > 0)
		exit(EXIT_SUCCESS);
	umask(0);
	if (setsid() < 0)
		die("setsid:");
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

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
	for (i = 1; i < argc; i++) {
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

	sig.sa_handler = recvalrm;
	sig.sa_flags = SA_RESTART;
	sigemptyset(&sig.sa_mask);
	sigaction(SIGALRM, &sig, 0);
	sigaction(SIGTERM, &sig, 0);
	sigaction(SIGINT, &sig, 0);
	/* XXX: replace with just singal? */

	/* XXX: when being called from another process this one succeeds */
	while (lockf(lockfd, F_LOCK, 0L) == -1)
		;
	if (duration > 0)
		(void)alarm(duration);

	for (;;) {
		XNextEvent(dpy, &ev);

		if (ev.type == Expose) {
			XClearWindow(dpy, win);
			for (i = 1; i < argc; i++)
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

	(void)lockf(lockfd, F_ULOCK, 0);
	(void)close(lockfd);
	(void)unlink(lockfile);

	return 0;
}
