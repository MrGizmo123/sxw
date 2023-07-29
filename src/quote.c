#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <unistd.h>
#include <signal.h>

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


#define WIDTH 450
#define HEIGHT 210
#define UPDATE_TIME 3600 /* in seconds */

static int x_pos;
static int y_pos;

static Fnt* big;
static Fnt* small;

static void redraw()
{
	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* put whatever you want to draw here */

	char quote[1024];
	sh("shuf --random-source=/dev/urandom -n1 data/quotes.txt | sed 's/-/\\n       -/g' | fold -s -w 37 | tr '\n' '$' ", quote, 1024);

	char* quoteline;
	char* source;
	char separator[2] = "-";

	quoteline = strtok(quote, separator);
	source = strtok(NULL, separator);
	
	drw_setfontset(drw, big);
	drw_paragraph(drw, 20, 20, WIDTH - 20, HEIGHT - 20, quoteline, 17, 7);

	drw_setfontset(drw, small);
	drw_paragraph(drw, 20, 160, WIDTH - 20, HEIGHT - 20, source, 13, 7);
	
	/* you should only be changing this part and the WIDTH, HEIGHT and UPDATE_TIME */

	/* put the drawn things onto the window */
	drw_map(drw, win, 0, 0, WIDTH, HEIGHT);

}

void sigalrm(int signum)
{
	XEvent exppp;

	memset(&exppp, 0, sizeof(exppp));
        exppp.type = Expose;
        exppp.xexpose.window = win;
        XSendEvent(dpy,win,False,ExposureMask,&exppp);
        XFlush(dpy);

	alarm(UPDATE_TIME);
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

	/* set alarm signal to update the widget contents */
	signal(SIGALRM, sigalrm);

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
	
	/* make SURE to change the res_name to the name of your widget */
	class_hint->res_name = "quote";
	class_hint->res_class = "widget";

	XSetClassHint(dpy, win, class_hint);


	/* put window on the screen */
	XMapWindow(dpy, win);

	/* create drw object to draw stuff */
	drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	const char* big_font[] = { "iosevka:size=17:style=Italic" };
	const char* small_font[] = { "iosevka:size=13" };

	
	big = drw_fontset_create(drw, big_font, 1);
	small = drw_fontset_create(drw, small_font, 1);

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);

	drw_setscheme(drw, scheme[SchemeYellow]);
	

	/* start updater thread */
	alarm(UPDATE_TIME);

	/* main loop */
	XEvent ev;
	while(XNextEvent(dpy, &ev) == 0)
	{
		switch(ev.type)
		{
			case Expose:
				redraw();
				break;
		        case ButtonPress:
				if (ev.xbutton.button == Button1)
				  redraw();
				break;
		}
	}

	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
