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


#define WIDTH 50
#define HEIGHT 50

#define PLAY_SYMBOL ""
#define PAUSE_SYMBOL ""

static int x_pos;
static int y_pos;

static int state; /* 0 if paused, 1 if playing */

static void redraw(int action)
{
	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* put whatever you want to draw here */

	if(state)
	{
		if(action)
		{
			char buffer[16];
			sh("mpc pause", buffer);
			state = 0;
		}

		int textw = TEXTW(PLAY_SYMBOL);
		int lpad  = (WIDTH - textw) / 2;

		drw_text(drw, lpad, 0, WIDTH, HEIGHT, 0, PLAY_SYMBOL, 0);
	}
	else
	{
		if(action)
		{
			char buffer[16];
			sh("mpc play", buffer);
			state = 1;
		}

		int textw = TEXTW(PAUSE_SYMBOL);
		int lpad  = (WIDTH - textw) / 2;

		drw_text(drw, lpad, 0, WIDTH, HEIGHT, 0, PAUSE_SYMBOL, 0);
	}

	/* you should only be changing this part and the WIDTH, HEIGHT and UPDATE_TIME */

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

	/* tell X11 that we want to receive Expose events */
	XSelectInput(dpy, win, ExposureMask | ButtonPressMask);

	/* set class hint so dwm does not tile */
	XClassHint* class_hint = XAllocClassHint();
	
	class_hint->res_name = "mpdplay";
	class_hint->res_class = "widget";

	XSetClassHint(dpy, win, class_hint);

	/* put window on the screen */
	XMapWindow(dpy, win);

	/* create drw object to draw stuff */
	drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	
	const char* fonts[] = {"Font Awesome 6 Free:size=30"};
	if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);

	drw_setscheme(drw, scheme[SchemeBlue]);
	
	/* main loop */
	XEvent ev;
	while(XNextEvent(dpy, &ev) == 0)
	{
		/* if window is exposed then redraw */
		if (ev.type == Expose)
			redraw(0);
		else if (ev.type == ButtonPress && ev.xbutton.button == Button1)
			redraw(1);

	}


	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
