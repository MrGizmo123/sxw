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
static XSetWindowAttributes xwa;
static Window root;
static Window win;

static Drw* drw;
static Clr* scheme[SchemeLast];


#define WIDTH 50
#define HEIGHT 50
#define UPDATE_TIME 1 /* in seconds */

static int x_pos;
static int y_pos;
static char* symbol; /* the symbol to display */
static int tag; /* for nth tag it equals 2^n */
static int state; /* first bit gives selected state
                     second bit gives occupied state
                     third bit gives urgent state

                     e.g.
                     state = 0 b 0 1 1 (binary form)
                                 | | |
                                 | | It is currently selected
                                 | |
                                 | It is occupied by some window
                                 |
                                 No urgent message on this tag

                     */

static void redraw(int action)
{
	/* clear screen of previous contents */
	drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

	/* put whatever you want to draw here */

  char selected_str[4];
  sh("dwm-msg get_monitors | jq '.[0].tag_state.selected'", selected_str, 4);
  char occupied_str[4];
  sh("dwm-msg get_monitors | jq '.[0].tag_state.occupied'", occupied_str, 4);
  char urgent_str[4];
  sh("dwm-msg get_monitors | jq '.[0].tag_state.urgent'", urgent_str, 4);

  int selected = atoi(selected_str);
  int occupied = atoi(occupied_str);
  int urgent = atoi(urgent_str);

  int width = TEXTW(symbol);
  int lpad = (WIDTH - width) / 2;

  drw_text(drw, lpad, 0, WIDTH, HEIGHT, 0, symbol, 0);

  if(tag & urgent)
  {
    drw_rect(drw, 0, 0, WIDTH, HEIGHT , 1, 1);
  }
  if(tag & selected)
  {
    drw_rect(drw, 0, HEIGHT - 10, WIDTH, 10, 1, 0);
  }
  if(tag & occupied)
  {
    drw_rect(drw, 5, 5, 10, 10, 1, 0);
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
	if(argc != 5)
		die("not enough arguments, run with x_pos, y_pos, tag and symbol as cmd line arguments");
	
	/* convert string arguments to integers, argv[0] is the command itself */
	x_pos = atoi(argv[1]);
	y_pos = atoi(argv[2]);

  	tag = atoi(argv[3]);
  	symbol = argv[4];

	/* set alarm signal to update the widget contents */
	signal(SIGALRM, sigalrm);

	/* obtain connection to X server */
	dpy = XOpenDisplay(NULL);
	if(dpy == NULL)
		die("Could not open display.");

	/* create window */
	scr = DefaultScreen(dpy);
	root = RootWindow(dpy, scr);

	xwa.background_pixel = BlackPixel(dpy, scr);
	xwa.override_redirect = 1;

	win = XCreateWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, DefaultDepth(dpy, scr), InputOutput, DefaultVisual(dpy, scr), CWBackPixel | CWOverrideRedirect, &xwa);
	//win = XCreateSimpleWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, WhitePixel(dpy, scr), BlackPixel(dpy, scr));

	/* tell X11 that we want to receive Expose events */
	XSelectInput(dpy, win, ExposureMask | ButtonPressMask);

	/* set class hint so dwm does not tile */
	XClassHint* class_hint = XAllocClassHint();
	
	/* make SURE to change the res_name to the name of your widget */
	class_hint->res_name = "template";
	class_hint->res_class = "widget";

	XSetClassHint(dpy, win, class_hint);


	/* put window on the screen */
	XMapWindow(dpy, win);

	/* create drw object to draw stuff */
	drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

	
	const char* fonts[] = {"iosevka:size=17"};
	if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
		die("no fonts could be loaded.");

	/* initialize color schemes from config.h */
	for (int i=0;i<SchemeLast;i++)
		scheme[i] = drw_scm_create(drw, colors[i], 2);

	drw_setscheme(drw, scheme[SchemeYellow]);
	

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
        if(ev.xbutton.button == Button1)
          redraw(1);
				break;
		}
	}

	/* close window and clean up */
	XUnmapWindow(dpy, win);
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);

	return 0;
}
