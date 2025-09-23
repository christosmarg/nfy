/* See LICENSE file for copyright and license details. */
#ifndef _NFY_CONFIG_H_
#define _NFY_CONFIG_H_

#define MAXLEN 82	/* Max length per line (characters) */

enum { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT }; /* Window positions */
static const char *bgcolor = "#201f1c";		/* Background color */
static const char *bordercolor = "#f18f19";	/* Border color */
static const char *fontcolor = "#e6e5e3";	/* Font color */
/* TODO: add fallbacks */
static const char *fonts = "monospace:size=12"; /* Fonts */
static const unsigned int padding = 10;		/* Padding (pixels) */
static const unsigned int borderw = 3;		/* Border width (pixels) */
static const unsigned int duration = 3;		/* Notification duration (seconds) */
static const unsigned int pos = TOP_RIGHT;	/* Window position */
static const unsigned int mx = 10;		/* Margin X (pixels) */
static const unsigned int my = 25;		/* Margin Y (pixels) */
static const unsigned int linespace = 10;	/* Line spacing (pixels) */
static const char *lockfile = "/tmp/nfy.lock";

#endif /* _NFY_CONFIG_H_ */
