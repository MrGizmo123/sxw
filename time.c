#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <unistd.h>

#include "drw.h"
#include "util.h"
#include "config.h"

#define LENGTH(X) (sizeof X / sizeof X[0])
#define TEXTW(X) (drw_fontset_getwidth(drw, (X)))


static Display* dpy;
static int scr;
static Window root;
static Window win;

static Drw* drw;
static Clr* scheme[SchemeLast];

#define WIDTH 200
#define HEIGHT 100
#define UPDATE_TIME 2 /* in seconds */

static int x_pos;
static int y_pos;

static void redraw()
{
	/* get the formatted time from date command */
	char output[16];
	sh("date '+%H:%M'", output);

	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* calculate position offset to put text in center */
	int text_width = TEXTW(output);
	int lpad = (WIDTH - text_width) / 2;

	/* draw the text */
	drw_text(drw, lpad, 0, WIDTH, HEIGHT, 0, output, 0);

	/* put the drawn things onto the window */
	drw_map(drw, win, 0, 0, WIDTH, HEIGHT);

}

int
main(int argc, char** argv)
{

	/* if x and y pos are not provided, then throw error */
	if(argc < 3)
		die("not enough arguments, run with x_pos and y_pos as cmd line arguments");
	
	/* convert string arguments to integers, argv[0] is the command itself */
	x_pos = atoi(argv[1]);
	y_pos = atoi(argv[2]);

	/* obtain connection to X server */
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL)
		die("Could not open display.");

	/* create window */
	scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);

	win = XCreateSimpleWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, WhitePixel(dpy, scr), BlackPixel(dpy, scr));


	/* set masks to recieve expose events */
	XSelectInput(dpy, win, ExposureMask);


	/* set class hint so dwm does not tile */
	XClassHint* class_hint = XAllocClassHint();
	
	class_hint->res_name = "time";
	class_hint->res_class = "widget";

	XSetClassHint(dpy, win, class_hint);


	/* put window on the screen */
	XMapWindow(dpy, win);

	/* create drw object to draw stuff */
	drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	/* create fonts */
	const char* fonts[] = { "iosevka:size=50:style=Bold", "FontAwesome:size=40"};
	if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);

	drw_setscheme(drw, scheme[SchemeRed]);

	/* start updater thread */
	
	/* main loop */
	XEvent ev;
	unsigned long last_draw_time = (unsigned long)time(NULL);
	unsigned long current_time = last_draw_time;
	while(1)
	{
		
		if(XCheckTypedEvent(dpy, Expose, &ev))
			redraw();

		current_time = (unsigned long)time(NULL);
		if((current_time - last_draw_time) > 1)
		{
			redraw();
			last_draw_time = current_time;
		}

		usleep(100000);

	}


	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
