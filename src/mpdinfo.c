#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
static XSetWindowAttributes xwa;

static Drw* drw;
static Clr* scheme[SchemeLast];


//#define WIDTH 300
//#define HEIGHT 60
#define UPDATE_TIME 1 /* in seconds */

static int x_pos;
static int y_pos;

static int WIDTH;
static int HEIGHT;

static int textsize;

static void redraw()
{
    /* clear screen of previous contents */
    drw_rect(drw, 0, 0, WIDTH, HEIGHT, 1, 1);

    /* put whatever you want to draw here */

    char name[32];
    sh("mpc current | awk -F '.' ' { print $1 } '", name, 32);

    char percent_str[4];
    sh("mpc status \%percenttime\% | tr -d ' %'", percent_str, 4);
    //percent_str[strlen(percent_str) - 1] = '\0'; /* remove last % sign */
    int percent = atoi(percent_str);

    int pad = 0;
    int barwidth = percent * (WIDTH - 2*pad) / 100;

    drw_text(drw, 10,  0, WIDTH, HEIGHT*0.5, 0, name, 0);
    drw_rect(drw, 0, HEIGHT*0.5833, barwidth, HEIGHT*0.5, 1, 0);	

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
    if(argc != 7)
	die("usage: mpdinfo [xpos] [ypos] [width] [height] [override-redirect] [text-size]");
	
    /* convert string arguments to integers, argv[0] is the command itself */
    x_pos = atoi(argv[1]);
    y_pos = atoi(argv[2]);

    WIDTH = atoi(argv[3]);
    HEIGHT = atoi(argv[4]);

    int override_redirect = atoi(argv[5]);
	
    textsize = atoi(argv[6]);
	

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
    xwa.override_redirect = override_redirect; /* This will tell the WM to ignore this window */
    xwa.backing_store = WhenMapped;

    win = XCreateWindow(dpy, root, x_pos, y_pos, WIDTH, HEIGHT, 0, DefaultDepth(dpy, scr), InputOutput, DefaultVisual(dpy, scr), CWBackPixel | CWOverrideRedirect, &xwa);


    
    /* tell X11 that we want to receive Expose events */
    XSelectInput(dpy, win, ExposureMask);
    
    /* set class hint so dwm does not tile (needs a rule in the dwm config.h for floating 'widget' class windows)*/
    XClassHint* class_hint = XAllocClassHint();
	
    class_hint->res_name = "mpdinfo";
    class_hint->res_class = "widget";

    XSetClassHint(dpy, win, class_hint);
    XFree(class_hint);

    /* put window on the screen */
    XMapWindow(dpy, win);

    /* create drw object to draw stuff */
    drw = drw_create(dpy, scr, root, WIDTH, HEIGHT);

    /* start updater thread */
    alarm(UPDATE_TIME);

    char buf[64];
    snprintf(buf, 64, "iosevka:size=%d", textsize);
    
    const char* fonts[] = { buf };
    if(!drw_fontset_create(drw, fonts, LENGTH(fonts)))
	die("no fonts could be loaded.");

    /* initialize color schemes from config.h */
    for (int i=0;i<SchemeLast;i++)
	scheme[i] = drw_scm_create(drw, colors[i], 2);

    drw_setscheme(drw, scheme[SchemeAqua]);
	
    /* main loop */
    XEvent ev;
    while(XNextEvent(dpy, &ev) == 0)	{
	switch(ev.type)		{
	case Expose:
	    redraw();
	}
    }

    /* close window and clean up */
    XUnmapWindow(dpy, win);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);

    return 0;
}
