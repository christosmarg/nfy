/* See LICENSE file for copyright and license details. */
enum { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT }; /* Window positions */
static const char *bgcolor = "#201f1c";		/* Background color */
static const char *bordercolor = "#f28f19";	/* Border color */
static const char *fontcolor = "#e6e5e3";	/* Font color */
/* TODO: add fallbacks */
static const char *fonts = "monospace:size=15"; /* Font */
static const int padding = 15;			/* Padding (pixels) */
static const int borderw = 2;			/* Border width (pixels) */
static const int duration = 3;			/* Notification duration (seconds) */
static const int pos = TOP_RIGHT;		/* Window position */
static const int mx = 10;			/* Margin X (pixels) */
static const int my = 25;			/* Margin Y (pixels) */
static const int maxlen = 250;			/* Max window length (pixels) */
static const int linespace = 10;		/* Line spacing (pixels) */
static const char *lockfile = "/tmp/nfy.lock";
