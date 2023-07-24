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


#define WIDTH 55
#define HEIGHT 240

#define BARHEIGHT 200

#define UPDATE_TIME 1 /* in seconds */

#define SYMBOL_LOW ""
#define SYMBOL_HIGH ""
#define SYMBOL_MUTE ""

static int x_pos;
static int y_pos;

static int vol;
static int mute;

static void redraw(int action)
{
	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* put whatever you want to draw here */


	if(action == 1)
	{
	  char buf[4];
	  sh("pactl set-sink-volume @DEFAULT_SINK@ +5% 1>/dev/null", buf, 4);
	}
	else if(action == 2)
	{
	  char buf[4];
	  sh("pactl set-sink-volume @DEFAULT_SINK@ -5% 1>/dev/null", buf, 4);
	}
	else if(action == 3)
	{
	  char buf[4];
	  sh("pactl set-sink-mute @DEFAULT_SINK@ toggle 1>/dev/null", buf, 4);
	}
	
	char volume[4];
	sh("pactl get-sink-volume @DEFAULT_SINK@ | grep -o '...%' | tr -d ' ' | uniq", volume, 4);
	vol = atoi(volume);

	int height = BARHEIGHT * vol/100;
	int y = BARHEIGHT - height;
	
	drw_rect(drw, 0, y, WIDTH, height, 1, 0);

	
	if(vol > 50)
	{
	  int lpad = (WIDTH - TEXTW(SYMBOL_HIGH)) / 2;
	  drw_text(drw, lpad, BARHEIGHT, WIDTH, HEIGHT - BARHEIGHT, 0, SYMBOL_HIGH, 0);
	}
	else
	{
	  int lpad = (WIDTH - TEXTW(SYMBOL_LOW)) / 2;
	  drw_text(drw, lpad, BARHEIGHT, WIDTH, HEIGHT - BARHEIGHT, 0, SYMBOL_LOW, 0);
	}

	char muted[4];
	sh("pactl get-sink-mute @DEFAULT_SINK@ | cut -d ' ' -f2 | sed 's/yes/1/g; s/no/0/g'", muted, 4);
	mute = atoi(muted);

	if(mute)
	{
	  int lpad = (WIDTH - TEXTW(SYMBOL_MUTE)) / 2;
	  drw_text(drw, lpad, BARHEIGHT, WIDTH, HEIGHT - BARHEIGHT, 0, SYMBOL_MUTE, 0);
	}
	
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
	class_hint->res_name = "volume";
	class_hint->res_class = "widget";

	XSetClassHint(dpy, win, class_hint);


	/* put window on the screen */
	XMapWindow(dpy, win);

	/* create drw object to draw stuff */
	drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	
	const char* fonts[] = {"iosevka:size=21"};
	if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);

	drw_setscheme(drw, scheme[SchemeRed]);
	

	/* start updater thread */
	alarm(UPDATE_TIME);
	redraw(0);

	/* main loop */
	XEvent ev;
	while(XNextEvent(dpy, &ev) == 0)
	{
		switch(ev.type)
		{
			case Expose:
				redraw(0);
				break;
			case ButtonPress:
				if(ev.xbutton.button == Button4)
				        redraw(1); // increment
				else if (ev.xbutton.button == Button5)
				        redraw(2); //decrement
				else if (ev.xbutton.button == Button1)
				        redraw(3); // mute/unmute
				break;
		}
	}

	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
