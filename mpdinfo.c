#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


#define WIDTH 300
#define HEIGHT 30
#define UPDATE_TIME 2 /* in seconds */

static int x_pos;
static int y_pos;

static void redraw()
{
	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* put whatever you want to draw here */

	char name[32];
	sh("echo $(mpc current)", name);

	char percent_str[4];
	sh("mpc status \%percenttime\%", percent_str);
	percent_str[strlen(percent_str) - 1] = '\0'; /* remove last % sign */
	int percent = atoi(percent_str);


	int pad = 0;
	int barwidth = percent * (WIDTH - 2*pad) / 100;

	drw_rect(drw, 0, 0, barwidth, HEIGHT, 1, 0);
	

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
	XSelectInput(dpy, win, ExposureMask);

	/* put window on the screen */
	XMapWindow(dpy, win);

	/* create drw object to draw stuff */
	drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	
	const char* fonts[] = {"iosevka:size=20"};
	if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);

	drw_setscheme(drw, scheme[SchemeAqua]);
	
	/* main loop */
	XEvent ev;
	unsigned long last_draw_time = (unsigned long)time(NULL);
	unsigned long current_time = last_draw_time;
	while(1)
	{
		/* if window is exposed then redraw */
		if(XCheckTypedEvent(dpy, Expose, &ev))
			redraw();

		/* redraw after 'UPDATE_TIME' seconds */
		current_time = (unsigned long)time(NULL);
		if((current_time - last_draw_time) > UPDATE_TIME)
		{
			redraw();
			last_draw_time = current_time;
		}

		/* sleep for 100000us i.e 100ms so that it does not consume all the cpu usage */
		usleep(100000);

	}


	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
