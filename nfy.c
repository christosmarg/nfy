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

struct line {
	char data[MAXLEN+1];
	size_t len;
};

#undef strlcpy	/* compat with openbsd */
size_t strlcpy(char *, const char *, size_t);
static void sighandler(int);

static char *argv0;
static Display *dpy;
static Window win;

size_t
strlcpy(char *dst, const char *src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	if (nleft != 0) {
		while (--nleft != 0)
			if ((*dst++ = *src++) == '\0')
				break;
	}
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';
		while (*src++)
			;
	}
	return (src - osrc - 1);
}

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
	XSetWindowAttributes attrs = {0};
	XftColor color;
	XftDraw *drw;
	XftFont *font;
	XGlyphInfo gi;
	struct flock fl;
	struct sigaction sa;
	struct line *lines, *lp;
	pid_t pid;
	int lockfd;
	int scr, scrw, scrh;
	int x, y, w = 0, h, th;
	int i, len = 0, nlines = 0;
	char buf[MAXLEN+1], ch;

	argv0 = *argv;
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
	argc -= optind;
	argv += optind;

	if (isatty(STDIN_FILENO)) {
		warnx("stdin is empty");
		return (0);
	}

	/* detach from tty */
	if ((pid = fork()) < 0)
		err(1, "fork");
	if (pid > 0)
		_exit(0);
	umask(0);
	if (setsid() < 0)
		err(1, "setsid");

	if ((lockfd = open(lockfile, O_CREAT | O_RDWR, 0600)) < 0)
		err(1, "open");

	/* init x11 */
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
	XftColorAllocName(dpy, vis, colormap, fontcolor, &color);


	/* read stdin into buffer */
	if ((lines = malloc(sizeof(struct line))) == NULL)
		err(1, "malloc");
	while (read(STDIN_FILENO, &ch, 1) > 0) {
		if (ch == '\n' || len == MAXLEN) {
			buf[len] = '\0';
			if ((lines = realloc(lines,
			    (nlines + 1) * sizeof(struct line))) == NULL)
				err(1, "realloc");
			lp = &lines[nlines];
			lp->len = len;
			strlcpy(lp->data, buf, sizeof(lp->data));
			/* determine window width based on largest line */
			XftTextExtentsUtf8(dpy, font, (XftChar8 *)lp->data,
			    lp->len, &gi);
			if (gi.width > w)
				w = gi.width;
			nlines++;
			len = 0;
		} else
			buf[len++] = ch;
	}
	w += padding * 2;
	h = (nlines - 1) * linespace + nlines * th + 2 * padding;

	/* calculate position coordinates */
	switch (pos) {
	case TOP_LEFT:
		x = mx;
		y = my;
		break;
	case TOP_RIGHT: /* FALLTHROUGH */
	default:
		x = scrw - w - mx - borderw * 2;
		y = my;
		break;
	case BOTTOM_LEFT:
		x = mx;
		y = scrh - h - my - borderw * 2;
		break;
	case BOTTOM_RIGHT:
		x = scrw - w - mx - borderw * 2;
		y = scrh - h - my - borderw * 2;
		break;
	}
	
	win = XCreateWindow(dpy, RootWindow(dpy, scr), x, y, w, h, borderw,
	    DefaultDepth(dpy, scr), CopyFromParent, vis,
	    CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attrs);
	drw = XftDrawCreate(dpy, win, vis, colormap);
	XSelectInput(dpy, win, ExposureMask | ButtonPress);
	XMapWindow(dpy, win);

	sa.sa_handler = sighandler;
	sa.sa_flags = SA_RESTART;
	(void)sigfillset(&sa.sa_mask);
	if (sigaction(SIGALRM, &sa, NULL) < 0)
		err(1, "sigaction(SIGALRM)");
	if (sigaction(SIGTERM, &sa, NULL) < 0)
		err(1, "sigaction(SIGTERM)");
	if (sigaction(SIGINT, &sa, NULL) < 0)
		err(1, "sigaction(SIGINT)");

	/* setup lock file */
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
			for (i = 0; i < nlines; i++)
				XftDrawStringUtf8(drw, &color, font,
				    padding,
				    linespace * i + th * (i + 1) + padding,
				    (XftChar8 *)lines[i].data, lines[i].len);
		} else if (ev.type == ButtonPress)
			break;
	}

	free(lines);
	XftDrawDestroy(drw);
	XftColorFree(dpy, vis, colormap, &color);
	XftFontClose(dpy, font);
	XRRFreeCrtcInfo(info);
	XRRFreeScreenResources(screens);
	XCloseDisplay(dpy);
	(void)close(lockfd);

	return (0);
}
